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

/**
 * Utility-Funktion für WLAN
*/

#ifndef wlanutils_h
#define wlanutils_h

// Initialisierung Station-Mode (bestehendes WLAN)
void setupWiFiSTA(const char* ssid, const char* password);

// Initialisierung Accesspoint-Mode (eigenes WLAN)
void setupWifiAP(const char* ssid, const char* password);

#endif
