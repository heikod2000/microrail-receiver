/**
 * Funktionen zur Konfiguration des Webservers.
 * Die separate Datei dient der Code-Auslagerung.
 *
 * (c) 8/2024 by heiko deserno
 *
 */

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

void configureWebServer(AsyncWebServer& server, JsonDocument config);
