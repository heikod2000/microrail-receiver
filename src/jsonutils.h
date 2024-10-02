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

#ifndef jsonutils_h
#define jsonutils_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include "types.h"

#define CFG_NAME "name"
#define CFG_WLAN_SSID "wlan_ssid"
#define CFG_WLAN_PASSWORD "wlan_password"
#define CFG_MOTOR_FREQUENCY "motor_frequency"
#define CFG_MOTOR_MAXSPEED "motor_maxspeed"
#define CFG_MOTOR_SPEED_STEP "motor_speed_step"
#define CFG_MOTOR_INERTIA "motor_inertia"
#define CFG_MOTOR_REVERSE "motor_reverse"

Config json2Config(StaticJsonDocument<350>& json);

StaticJsonDocument<350> config2Json(Config& config);

#endif
