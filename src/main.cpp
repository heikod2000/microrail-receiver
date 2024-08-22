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

// Events für Daten an Browser senden
#define EVENT_START             "event.start"
#define EVENT_BATTERY_VOLTAGE   "event.batvoltage"
#define EVENT_BATTERY_RATE      "event.batrate"
#define EVENT_SPEED             "event.speed"
#define EVENT_DIRECTION         "event.direction"

// Function prototypes
void motionControl();
void checkPower();
void handleCommands(String command);
void sendEventDirection();
void sendEventSpeed();

// Create a webserver that listens for HTTP request on port 80
AsyncWebServer server(80);
AsyncEventSource events("/events");

// Lolin Motor-Shield (Version 2.0.0, HR8833, AT8870)
LOLIN_I2C_MOTOR motor;

// Timer regelt alle 100 ms die Motorgeschwindigkeit
Ticker motionControlTicker(motionControl, 100, 0, MILLIS);

// Timer fragt alle 30 Sekunden den Akku-Status ab
Ticker powerCheckTicker(checkPower, 30000, 0, MILLIS);

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
String appVersionString = "MicroRail R v" + appVersion + " by hde";


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

  server.on("/cmd", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "ok");
    handleCommands(request->getParam("command")->value());
  });

  events.onConnect([](AsyncEventSourceClient * client) {
    if (client->lastId()) {
      Serial.printf("Client reconnected! Letzte message ID: %u\n", client->lastId());
    }
    client->send(buildConnectMessage().c_str(), EVENT_START, millis());
    sendEventSpeed();
    sendEventDirection();
  });
  server.addHandler(&events);
  Serial.println("- HTTP server started : OK");

  // Start Timer
  motionControlTicker.start();
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
  motionControlTicker.update();
  powerCheckTicker.update();
  delay(10);
}

void sendEventDirection() {
  char directionChar[2];
  itoa(direction, directionChar, 10);
  events.send(directionChar, EVENT_DIRECTION, millis());
}

void sendEventSpeed() {
  char speedChar[4];
  itoa(actual_speed, speedChar, 10);
  events.send(speedChar, EVENT_SPEED, millis());
}

void handleCommands(String command) {
  Serial.print("Command: [");
  Serial.print(command);
  Serial.println("]");

  int motor_speed_step = config[CFG_MOTOR_SPEED_STEP];
  if (command == "STOP") {
    target_speed = 0;
  } else if (command == "SLOWER") {
    target_speed -= motor_speed_step;
    if (target_speed < 0) {
      target_speed = 0;
    }
  } else if (command == "FASTER") {
    target_speed += motor_speed_step;
    if (target_speed > 100) {
      target_speed = 100;
    }
  } else if (command == "CHANGEDIRECTION") {
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
      // Send direction
      sendEventDirection();
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

  int motor_speed_step = config[CFG_MOTOR_SPEED_STEP];
  float motor_maxspeed = floor(atoi(config[CFG_MOTOR_MAXSPEED]) / 100);

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
  motor.changeDuty(MOTOR_CH_BOTH, actual_speed * motor_maxspeed);
  sendEventSpeed();
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

  char batVoltageChar[6];
  dtostrf(batVoltage, 4, 2, batVoltageChar);
  events.send(batVoltageChar, EVENT_BATTERY_VOLTAGE, millis());

  char batRateChar[4];
  itoa((int)batRate, batRateChar, 10);
  events.send(batRateChar, EVENT_BATTERY_RATE, millis());
}
