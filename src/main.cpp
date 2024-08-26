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
#include "config.h"
#include "wlanutils.h"
#include "webserver.h"
#include "const.h"
#include "helper.h"

// Wenn definiert, dann Verbindung mit bestehendem WLAN (file localconfig.h)
#define LOCAL_DEBUG

#ifdef  LOCAL_DEBUG
#include "localconfig.h"
#endif

#define dir_forward 0
#define dir_backward 1
#define STATUS_LED D6

// Function prototypes
void motionControl();
void checkPower();
void handleCommands(String command);

// Create a webserver that listens for HTTP request on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Lolin Motor-Shield (Version 2.0.0, HR8833, AT8870)
LOLIN_I2C_MOTOR motor;

// Timer regelt alle 100 ms die Motorgeschwindigkeit
Ticker motionControlTicker(motionControl, 500, 0, MILLIS);

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
JsonDocument config;
String appVersionString = "MicroRail R v" + appVersion;

uint32_t wsClientId; // ID des WebSocket-Clients
AsyncWebSocketClient * wsClient;

String buildClientStatus(String source) {
  JsonDocument status;

  status["name"] = config[CFG_NAME];
  status["ssid"] = config[CFG_WLAN_SSID];
  status["version"] = appVersionString;
  status["direction"] = direction;
  status["speed"] = actual_speed;
  status["batRate"] = batRate;
  status["batVoltage"] = (float)((int)(batVoltage*100))/100.0;
  status["source"] = source;

  String result;
  serializeJson(status, result);
  return result;
}

/**
 * Websocket-Eventhandler
 */
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    wsClientId = client->id();
    wsClient = client;
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->text(buildClientStatus("connect"));
  } else if(type == WS_EVT_DISCONNECT){
    Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
  } else if(type == WS_EVT_ERROR){
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if(type == WS_EVT_DATA){
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
      String msg = "";
      if(info->final && info->index == 0 && info->len == len){
        if(info->opcode == WS_TEXT){
          for(size_t i=0; i < info->len; i++) {
            msg += (char) data[i];
          }
          if (msg.startsWith("#")) {
            handleCommands(msg);
          }
        }
      }
  }
}

/*------------------------------------------------------------------------------
SETUP
------------------------------------------------------------------------------*/

/**
 * Setup-Routine des Microcontrollers
 * - Filesystem initialisieren und Konfiguration einlesen
 * - WLAN-Verbindung starten
 * - Webserver starten
 * - Motorensteuerung starten
 */
void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.print("- Init: ");
  Serial.println(appVersionString.c_str());

  // Analoger Eingang für Überwachung der Akku-Spannung
  pinMode(A0, INPUT);
  // Status-LED initialisieren
  pinMode(STATUS_LED, OUTPUT);
  analogWrite(STATUS_LED, 200); // Status-LED hell leuchten

  initFs(); // LittleFS initialisieren
  readConfigurationFromFs(); // Konfiguration aus LittleFS einlesen

  // Start WLAN
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
  configureWebServer(server, config);
  Serial.println("- HTTP server started : OK");

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  Serial.println("- Websocket Server started : OK");

  // Start Timer
  //motionControlTicker.start();
  powerCheckTicker.start();
  Serial.println("- Timer started : OK");

  // Init Motor-Shield
  Serial.print("- Motor-Shield init");
  while (motor.PRODUCT_ID != PRODUCT_ID_I2C_MOTOR) {
    motor.getInfo();
  }
  int motor_frequency = config[CFG_MOTOR_FREQUENCY];
  motor.changeFreq(MOTOR_CH_BOTH, motor_frequency);
  motor.changeDuty(MOTOR_CH_BOTH, 0.0);
  motor.changeStatus(MOTOR_CH_BOTH, MOTOR_STATUS_CW); // Vorwärts
  Serial.println(" : OK");

  // LED einschalten
  analogWrite(STATUS_LED, 50);  // Status-LED dunkel leuchten
  Serial.println("- Setup completed");
  Serial.println("----------------------------------------------");
  debugConfig(config);
}

/*------------------------------------------------------------------------------
LOOP
------------------------------------------------------------------------------*/
void loop() {
  //motionControlTicker.update();
  powerCheckTicker.update();
  ws.cleanupClients();
}

void handleCommands(String command) {
  Serial.printf("Command: [%s]\n", command.c_str());

  int motor_speed_step = config[CFG_MOTOR_SPEED_STEP];
  if (command == "#STOP") {
    target_speed = 0;
  } else if (command == "#SLOWER") {
    target_speed -= motor_speed_step;
    if (target_speed < 0) {
      target_speed = 0;
    }
  } else if (command == "#FASTER") {
    target_speed += motor_speed_step;
    if (target_speed > 100) {
      target_speed = 100;
    }
  } else if (command == "#CHANGEDIRECTION") {
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
       wsClient->text(buildClientStatus("handleCommands")); // Richtungsänderung
    }
  }
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
  wsClient->text(buildClientStatus("motionControl"));
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
  //Serial.printf("Akku %f V, %d %%\n", batVoltage, batRate);

  wsClient->text(buildClientStatus("checkPower"));
}
