/*
 __     __  _______   _______
|  |   |  ||  ___   \|  _____|
|  |___|  || |   |  || |____
|   ___   || |   |  ||  ____|
|  |   |  || |__ |  || |_____
|__|   |__||_______/ |_______|

microrail.hdecloud.de
© Heiko Deserno, 9/2024

*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

/**
 * @brief Stellt WLAN-Verbindung zum bestehenden Netz her.
 *
 */
void setupWiFiSTA(const char* ssid, const char* password) {
  WiFi.begin(ssid, password);
  Serial.printf("Connecting to [%s] ...\n", ssid);

  // Warteschleife bis Verbindung hergestellt ist
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  // Verbindungsdaten ausgeben
  Serial.printf("Wifi connect to [%s], IP: [%s], MAC: [%s]\n", ssid,
    WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str());
  Serial.printf("- init WiFi-STA: OK\n");
}

/**
 * @brief Öffnet WLAN Access-Point.
 *
 */
void setupWifiAP(const char* ssid, const char* password) {
  // Accesspoint starten
  boolean status = WiFi.softAP(ssid, password);
  if (!status) {
    Serial.println("Wifi SoftAP cannot Connect");
  }

  Serial.printf("Wifi connect with [%s], IP: [%s], MAC: [%s]\n", ssid,
    WiFi.softAPIP().toString().c_str(), WiFi.macAddress().c_str());
  Serial.printf("- init WiFi-AP: OK\n");
}
