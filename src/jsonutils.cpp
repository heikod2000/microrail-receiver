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
 * Implementierung Json-Utils
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include "types.h"
#include "jsonutils.h"

/*
 * Konvertiert einen JSON-Dokument in eine Config-Struktur.
 *
 * @param json Ein JSON-Dokument
 * @return Config-Struktur
 *
 */
Config json2Config(StaticJsonDocument<350>& jsonCfg){
  Config config;
  config.name = jsonCfg[CFG_NAME].as<const char*>();
  config.wlan_ssid = jsonCfg[CFG_WLAN_SSID].as<const char*>();
  config.wlan_password = jsonCfg[CFG_WLAN_PASSWORD].as<const char*>();
  config.motor_frequency = jsonCfg[CFG_MOTOR_FREQUENCY].as<unsigned int>();
  config.motor_maxspeed = jsonCfg[CFG_MOTOR_MAXSPEED].as<unsigned int>();
  config.motor_inertia = jsonCfg[CFG_MOTOR_INERTIA].as<unsigned int>();
  config.motor_speed_step = jsonCfg[CFG_MOTOR_SPEED_STEP].as<unsigned int>();
  config.motor_reverse = jsonCfg[CFG_MOTOR_REVERSE].as<unsigned int>();
  return config;
}

/**
 * Konvertiert eine Config-Struktur in einen JSON-Dokument.
 * Validierung wird durchgeführt
 */
StaticJsonDocument<350> config2Json(Config& config){
  StaticJsonDocument<350> newConfig;

  newConfig[CFG_MOTOR_REVERSE] = config.motor_reverse;

  if (config.motor_inertia >= 50 && config.motor_inertia <= 500) {
    newConfig[CFG_MOTOR_INERTIA] = config.motor_inertia;
  } else {
    Serial.println("Invalid motor-inertia value. Must be between 50 and 500.");
    newConfig[CFG_MOTOR_INERTIA] = 200; // Default to 200
  }

  if (config.motor_frequency >= 50 && config.motor_frequency <= 20000) {
    newConfig[CFG_MOTOR_FREQUENCY] = config.motor_frequency;
  } else {
    Serial.println("Invalid motor-frequence value. Must be between 50 and 20000.");
    newConfig[CFG_MOTOR_FREQUENCY] = 100;
  }

  if (config.motor_maxspeed >= 20 && config.motor_maxspeed <= 100) {
    newConfig[CFG_MOTOR_MAXSPEED] = config.motor_maxspeed;
  } else {
    Serial.println("Invalid motor-maxspeed value. Must be between 20 and 100.");
    newConfig[CFG_MOTOR_MAXSPEED] = 100;
  }

  if (config.motor_speed_step >= 4 && config.motor_speed_step <= 30) {
    newConfig[CFG_MOTOR_SPEED_STEP] = config.motor_speed_step;
  } else {
    Serial.println("Invalid motor-speedstep value. Must be between 4 and 30.");
    newConfig[CFG_MOTOR_SPEED_STEP] = 10;
  }

  // TODO: Validate wlan_name, name
  newConfig[CFG_WLAN_SSID] = config.wlan_ssid;

  return newConfig;
}
