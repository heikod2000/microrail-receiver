/**
 * Utility-Funktion für WLAN
*/

// Initialisierung Station-Mode (bestehendes WLAN)
void setupWiFiSTA(const char* ssid, const char* password);

// Initialisierung Accesspoint-Mode (eigenes WLAN)
void setupWifiAP(const char* ssid, const char* password);