/**
 *  MICRORAIL
 *  Motorensteuerung via WLAN
 *
 *  Heiko Deserno (heiko@deserno-mail.de)
 */
#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LITTLEFS.h>
#include <ArduinoJson.h>
#include <LOLIN_I2C_MOTOR.h>
#include <ticker.h>
#include <RunningMedian.h>
#include "version.h"
#include "wlanutils.h"

#define CFG_NAME "name"
#define CFG_WLAN_SSID "wlan_ssid"
#define CFG_WLAN_PASSWORD "wlan_password"
#define CFG_MOTOR_FREQUENCY "motor_frequency"
#define CFG_MOTOR_MAXSPEED "motor_maxspeed"
#define CFG_MOTOR_SPEED_STEP "motor_speed_step"
#define CFG_IP_ADDRESS "ip_address"
#define CFG_MAC_ADDRESS "mac_address"

// Wenn definiert, dann Verbindung mit bestehendem WLAN (file localconfig.h)
#define LOCAL_DEBUG

#ifdef  LOCAL_DEBUG
#include "localconfig.h"
#endif

#define dir_forward 0
#define dir_backward 1
#define LED_STATUS D6
#define LED_ONBOARD D4

// ----------------------------------------------------------------------------
// Definition of the LED component
// ----------------------------------------------------------------------------

struct Led {
    // state variables
    uint8_t pin;
    bool    on;

    // methods
    void update() {
        digitalWrite(pin, on ? HIGH : LOW);
    }
};

// ----------------------------------------------------------------------------
// Definition of global variables
// ----------------------------------------------------------------------------

Led onboard_led = { LED_BUILTIN, true };  // Die Onboard LED verhält sich anders!
Led status_led = { LED_STATUS, false };

// Create a webserver that listens for HTTP request on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Lolin Motor-Shield (Version 2.0.0, HR8833, AT8870)
LOLIN_I2C_MOTOR motor;

void motionControl();
void checkPower();

// Timer regelt alle x ms die Motorgeschwindigkeit
Ticker motionControlTicker(motionControl, 100, 0, MILLIS);
// Timer fragt alle 60 Sekunden den Akku-Status ab
Ticker powerCheckTicker(checkPower, 60000, 0, MILLIS);

// Speicher zur Berechnung der Durchschnittswerte bei der Akkuprüfung
RunningMedian median1 = RunningMedian(10);

// interne Variablen
int direction = dir_forward;   // Richtung 0: vorwärts, 1: rückwärts
int actual_speed = 0;          // aktuelle Geschwindigkeit 0 - 100
int target_speed = 0;          // Zielgeschwindigkeit
byte batRate = 100;            // Akku-Kapazität
float batVoltage = 4.2;        // Akku-Spannung

const char *configFilename = "/config.json";  // Filename in Filesystem (LittleFS)
String appVersionString = "MicroRail R v" + appVersion;
StaticJsonDocument<350> config;

void listAllFilesInDir(String dir_path) {
	Dir dir = LittleFS.openDir(dir_path);
	while(dir.next()) {
		if (dir.isFile()) {
			// print file names
			Serial.printf("File: %s%s\n", dir_path.c_str(), dir.fileName().c_str());
		}
		if (dir.isDirectory()) {
			// print directory names
			Serial.printf("Dir: %s%s/\n", dir_path.c_str(), dir.fileName().c_str());
			// recursive file listing inside new directory
			listAllFilesInDir(dir_path + dir.fileName() + "/");
		}
	}
}

/**
 * Filesystem initialisieren
 */
void initLittleFS() {
  // Initialize LittleFS
  if(!LittleFS.begin()) {
    Serial.println("Cannot mount LittleFS volume...");
    while (1) {
      onboard_led.on = millis() % 200 < 50;
      onboard_led.update();
    }
  }
  // Dateien für Debug auflisten
  //listAllFilesInDir("/");
  Serial.println("- init LittleFS : OK");
}

/**
 * Konfiguration 'config.json' aus dem FS lesen
 */
void initConfiguration() {
  File configFile = LittleFS.open(configFilename, "r");
  if(!configFile || configFile.isDirectory()){
    Serial.println("Fail to open configfile for reading");
    while (1) {
      onboard_led.on = millis() % 200 < 50;
      onboard_led.update();
    }
  }
  deserializeJson(config, configFile);
  configFile.close();
  Serial.println("- init Configuration : OK");
}

void initWiFi() {
#ifdef LOCAL_DEBUG
  setupWiFiSTA(ssidSTA, passwordSTA);  // WiFi Verbindung mit bestehendem WLAN aufbauen
  config[CFG_IP_ADDRESS] = WiFi.localIP().toString();
#else
  const char* wlan_ssid = config[CFG_WLAN_SSID];
  const char* wlan_password = config[CFG_WLAN_PASSWORD];
  setupWifiAP(wlan_ssid, wlan_password);  // WLAN-Accesspoint starten
  config[CFG_IP_ADDRESS] = WiFi.softAPIP();
#endif
  config[CFG_MAC_ADDRESS] = WiFi.macAddress();
}

// ----------------------------------------------------------------------------
// Web server initialization
// ----------------------------------------------------------------------------

String processor(const String& var) {
  if(var == "SSID") {
    return config[CFG_WLAN_SSID];
  } else if(var == "NAME") {
      return config[CFG_NAME];
  } else if(var == "VERSION") {
    return appVersionString;
  } else if(var == "PASSWORD") {
      return config[CFG_WLAN_PASSWORD];
  } else if(var == "MOTORFREQUENCY") {
      return String(config[CFG_MOTOR_FREQUENCY]);
  } else if(var == "MOTORMAXSPEED") {
      return String(config[CFG_MOTOR_MAXSPEED]);
  } else if(var == "MOTORSPEEDSTEP") {
      return String(config[CFG_MOTOR_SPEED_STEP]);
  }
  return String();
}

void initWebServer() {
  server.serveStatic("/", LittleFS, "/");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html", false, processor);
  });

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/chip.png", "image/png");
  });

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.on("/setup", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/setup.html", "text/html", false, processor);
  });

  server.on("/setup", HTTP_POST, [](AsyncWebServerRequest *request){
    Serial.println("save setup data");
    StaticJsonDocument<300> newConfig;
    newConfig[CFG_NAME] = request->getParam("name", true)->value();
    newConfig[CFG_WLAN_SSID] = request->getParam("wlanssid", true)->value();
    newConfig[CFG_MOTOR_FREQUENCY] = request->getParam("motor-frequency", true)->value();
    newConfig[CFG_MOTOR_MAXSPEED] = request->getParam("motor-maxspeed", true)->value();
    newConfig[CFG_MOTOR_SPEED_STEP] = request->getParam("motor-speedstep", true)->value();
    newConfig[CFG_WLAN_PASSWORD] = request->getParam("password", true)->value();

    String configFile;
    serializeJsonPretty(newConfig, configFile);
    File file = LittleFS.open("/config.json", "w");
    file.print(configFile);
    file.close();

    request->send(LittleFS, "/setupok.html");
  });

  server.begin();
  Serial.println("- HTTP server started : OK");
}

// ----------------------------------------------------------------------------
// WebSocket initialization
// ----------------------------------------------------------------------------

void notifyClients() {
  ws.printfAll("A:%d:%d", direction, actual_speed);
}

void handleCommands(char* command) {
  Serial.printf("Command: [%s]\n", command);

  int motor_speed_step = config[CFG_MOTOR_SPEED_STEP];
  if (strcmp(command, "#STOP") == 0) {
    target_speed = 0;
  } else if (strcmp(command, "#SLOWER") == 0) {
    target_speed -= motor_speed_step;
    if (target_speed < 0) {
      target_speed = 0;
    }
  } else if (strcmp(command, "#FASTER") == 0) {
    target_speed += motor_speed_step;
    if (target_speed > 100) {
      target_speed = 100;
    }
  } else if (strcmp(command, "#CHANGEDIRECTION") == 0) {
    // Richtungswechsel nur bei Halt
    if (actual_speed == 0) {
      if (direction == dir_forward) {
        // vorwärts -> umschalten auf rückwärts
        Serial.println("Richtungswechsel rückwärts");
        direction = dir_backward;
        //motor.changeStatus(MOTOR_CH_BOTH, MOTOR_STATUS_STANDBY);
        //delay(200);
        //motor.changeStatus(MOTOR_CH_BOTH, MOTOR_STATUS_CCW);
      } else {
        // rückwärts -> umschalten auf vorwärts
        Serial.println("Richtungswechsel vorwärts");
        direction = dir_forward;
        //motor.changeStatus(MOTOR_CH_BOTH, MOTOR_STATUS_STANDBY);
        //delay(200);
        //motor.changeStatus(MOTOR_CH_BOTH, MOTOR_STATUS_CW);
      }
      //delay(200);
       notifyClients();
    }
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo * info = (AwsFrameInfo*)arg;
  String msg = "";
  if(info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT){
      data[len] = 0;
      char* cmd = (char*)data;
      handleCommands(cmd);
  }
}

/**
 * Websocket-Eventhandler
 */
void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    client->printf("A:%d:%d", direction, actual_speed);
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
  } else if(type == WS_EVT_DISCONNECT){
    Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
  } else if(type == WS_EVT_ERROR){
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if(type == WS_EVT_DATA){
      handleWebSocketMessage(arg, data, len);
  }
}

/**
 * WebSocket-Server initialisieren
 */
void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
  Serial.println("- init WebSocket: OK");
}

void initMotorShield() {
  while (motor.PRODUCT_ID != PRODUCT_ID_I2C_MOTOR) {
    motor.getInfo();
    onboard_led.on = millis() % 200 < 50;
    onboard_led.update();
  }
  int motor_frequency = config[CFG_MOTOR_FREQUENCY];
  motor.changeFreq(MOTOR_CH_BOTH, motor_frequency);
  motor.changeDuty(MOTOR_CH_BOTH, 0.0);
  motor.changeStatus(MOTOR_CH_BOTH, MOTOR_STATUS_CW); // Vorwärts
  Serial.println("- init Motor-Shield: OK");
}

void debugConfig(JsonDocument& config) {
    Serial.printf("WLAN SSID: [%s], Passwort: [%s]\n",
      config[CFG_WLAN_SSID].as<const char*>(),
      config[CFG_WLAN_PASSWORD].as<const char*>());
    Serial.printf("IP-Address: [%s], MAC-Address: [%s]\n",
      config[CFG_IP_ADDRESS].as<const char*>(), config[CFG_MAC_ADDRESS].as<const char*>());
    Serial.printf("Name: [%s], Motor Frequenz: [%d] Hz, Maxspeed: [%d] %%, SpeedStep: [%d]\n",
      config[CFG_NAME].as<const char*>(),
      config[CFG_MOTOR_FREQUENCY].as<unsigned int>(),
      config[CFG_MOTOR_MAXSPEED].as<unsigned int>(),
      config[CFG_MOTOR_SPEED_STEP].as<unsigned int>());
}


/**
 * @brief timer-gesteuerte Routine zur Anpasusng der Geschwindigkeit.
 *
 */
void motionControl() {
  if (actual_speed == target_speed) {
    // nichts zu tun
    return;
  }
  Serial.println("motionControl");

  int motor_speed_step = config[CFG_MOTOR_SPEED_STEP];
  //float motor_maxspeed = floor(atoi(config[CFG_MOTOR_MAXSPEED]) / 100);

  if (actual_speed < target_speed) {
    // Geschwindigkeit erhöhen
    actual_speed += motor_speed_step;
    if (actual_speed > 100) {
      actual_speed = 100;
    }
  } else if (actual_speed > target_speed) {
    // Geschwindigkeit verringern
    actual_speed -= motor_speed_step;
    if (actual_speed < 0) {
      actual_speed = 0;
    }
  }

  // Motor steuern...
  //motor.changeDuty(MOTOR_CH_BOTH, actual_speed * motor_maxspeed);
  notifyClients();
}

/**
 * @brief
 *
 */
void checkPower() {
  int raw = analogRead(A0);
  median1.add(raw);
  long m = median1.getMedian();
  unsigned long BatValue = m * 100L;
  float BatVoltage1 = BatValue * 4.2 / 1024L;

  batRate = map(BatVoltage1, 240, 420, 0, 100);
  batVoltage = BatVoltage1 / 100.0;
  batVoltage = roundf(batVoltage * 100) / 100;
  Serial.printf("Akku %f V, %d %%\n", batVoltage, batRate);
  ws.printfAll("B:%0.1f:%d", batVoltage, batRate);
}

/**
 * Setup-Routine des Microcontrollers
 * - Filesystem initialisieren und Konfiguration einlesen
 * - WLAN-Verbindung starten
 * - Webserver starten
 * - Motorensteuerung starten
 */
// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------
void setup() {
  pinMode(onboard_led.pin, OUTPUT); // OnBoard-LED
  pinMode(status_led.pin, OUTPUT);  // Status-LED
  pinMode(A0, INPUT);               // Anlaloger Eingang für Akku-Überwachung

  // Onboard-Led einschalten zur Visualisierung des Setup-Prozesses
  onboard_led.on = false; onboard_led.update();

  Serial.begin(115200); delay(500);
  Serial.printf("\n- Init: %s\n", appVersionString.c_str());

  initLittleFS();
  initConfiguration();
  initWiFi();
  initMotorShield();
  initWebSocket();
  initWebServer();

  // Start Timer
  motionControlTicker.start();
  powerCheckTicker.start();
  Serial.println("- Timer started : OK");

  Serial.println("- Setup completed");
  Serial.println("----------------------------------------------");
  debugConfig(config);

  // Onboard-LED ausschalten, Status-LED einschalten
  onboard_led.on = true; onboard_led.update();
  status_led.on = true; status_led.update();
}

// ----------------------------------------------------------------------------
// Main control loop
// ----------------------------------------------------------------------------

void loop() {
  motionControlTicker.update();
  powerCheckTicker.update();
  ws.cleanupClients();
}
