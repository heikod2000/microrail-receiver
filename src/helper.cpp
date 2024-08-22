/**
 * Hilsfunktionen für Debug-Zwecke
 *
 * (c) 8/2024 heiko deserno
 */

#include <ArduinoJson.h>
#include <LITTLEFS.h>
#include "const.h"

extern const char *configFilename;
extern JsonDocument config;
extern String appVersionString;

/**
 * Konsolenausgabe der Konfiguration
 */
void debugConfig(JsonDocument config) {
    const char* wlan_ssid = config[CFG_WLAN_SSID];
    const char* wlan_password = config[CFG_WLAN_PASSWORD];
    const char* name = config[CFG_NAME];
    const char* ip_address = config[CFG_IP_ADDRESS];
    const char* mac_address = config[CFG_MAC_ADDRESS];
    int motor_frequency = config[CFG_MOTOR_FREQUENCY];
    int motor_maxspeed = config[CFG_MOTOR_MAXSPEED];
    int motor_speed_step = config[CFG_MOTOR_SPEED_STEP];

    Serial.print("WLAN SSID: [");
    Serial.print(wlan_ssid);
    Serial.print("] Passwort: [");
    Serial.print(wlan_password);
    Serial.println("]");
    Serial.print("Name: [");
    Serial.print(name);
    Serial.println("]");
    Serial.printf("IP-Address: [%s], MAC-Address: [%s]\n", ip_address, mac_address);
    Serial.printf("Motor Frequenz: [%d] Hz, Maxspeed: [%d] %%, SpeedStep: [%d]\r\n", motor_frequency,
      motor_maxspeed, motor_speed_step);
}

/**
 * listet rekursiv alle Dateien des Filesystems zu Debug-Zwecken auf.
 */
void listAllFilesInDir(String dir_path) {
	Dir dir = LittleFS.openDir(dir_path);
	while(dir.next()) {
		if (dir.isFile()) {
			// print file names
			Serial.print("File: ");
			Serial.println(dir_path + dir.fileName());
		}
		if (dir.isDirectory()) {
			// print directory names
			Serial.print("Dir: ");
			Serial.println(dir_path + dir.fileName() + "/");
			// recursive file listing inside new directory
			listAllFilesInDir(dir_path + dir.fileName() + "/");
		}
	}
}

/**
 * Initialisierung des Filesystems und Auflistung aller Dateien
 */
void initFs() {
  // Initialize LittleFS
  if(!LittleFS.begin()) {
    Serial.println("# LittleFS initialization failed!");
    exit(1);
  }
  Serial.println("- LittleFS init : OK");
  // Dateien für Debug auflisten
  //listAllFilesInDir("/");
}


/**
 * Liest die Empfänger-Konfiguration aus dem Filesystem.
 */
void readConfigurationFromFs() {

  File configFile = LittleFS.open(configFilename, "r");
  if(!configFile || configFile.isDirectory()){
    Serial.println("# Failed to open configfile for reading");
    exit(1);
  }
  DeserializationError error = deserializeJson(config, configFile);
  if (error) {
    Serial.println(F("# Failed to read configfile"));
    exit(1);
  }
  configFile.close();
  Serial.println("- Read Configuration : OK");
}

/**
 * erstellt eine Welcomenachricht für die Konsole mit wichtigen
 * Konfigurationsdaten.
 */
String buildConnectMessage() {
  JsonDocument doc;


  doc["name"] = config[CFG_NAME];
  doc["ssid"] = config[CFG_WLAN_SSID];
  doc["macaddress"] = config[CFG_MAC_ADDRESS];
  doc["version"] = appVersionString.c_str();

  String response;
  serializeJson(doc, response);
  Serial.print("Connect-Message: ");
  Serial.println(response);
  return response;
}
