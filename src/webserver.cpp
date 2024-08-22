/**
 * Funktionen zur Konfiguration des Webservers.
 * Die separate Datei dient der Code-Auslagerung.
 *
 * (c) 8/2024 by heiko deserno
 *
 */

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <LITTLEFS.h>
#include "const.h"

void configureWebServer(AsyncWebServer& server, JsonDocument config) {

  //DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  //DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT");
  //DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

  // Serve static files from the filesystem (LittleFS)
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
  server.serveStatic("/setup", LittleFS, "/").setDefaultFile("setup.html");

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/chip.png", "image/png");
  });

  /**
   * API-Request liefert die aktuelle Konfiguration als JSON.
   */
  server.on("/config", HTTP_GET, [config](AsyncWebServerRequest * request) {
    String response;
    serializeJson(config, response);
    request->send(200, "application/json", response);
  });

  /**
   * Handle setup POST request to save new configuration data.
   * Konfigurationsdaten werden als Json serialisiert und in die Datei /config.json des
   * Filesystems geschrieben.
   */
  server.on("/setup", HTTP_POST, [](AsyncWebServerRequest * request) {
    Serial.println("save setup data");
    JsonDocument newConfig;
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
}
