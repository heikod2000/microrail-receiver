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

  int i = 0;
  // Warteschleife bis Verbindung hergestellt ist
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(++i);
    Serial.print(" ");
  }

  // Verbindungsdaten ausgeben
  Serial.println("");
  Serial.print("- Connected to: ");
  Serial.println(ssid);
  Serial.print("- IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("- MAC address: ");
  Serial.println(WiFi.macAddress());
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
  Serial.print("- Connected to: ");
  Serial.println(ssid);
  Serial.print("- IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("- MAC address: ");
  Serial.println(WiFi.macAddress());
}