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
 * Defnition globaler Typen.
 */

#ifndef types_h
#define types_h

#include <Arduino.h>

// ----------------------------------------------------------------------------
// Definition of Config
// ----------------------------------------------------------------------------

struct Config {
  String name;
  String wlan_ssid;
  String wlan_password;
  int motor_frequency;
  int motor_maxspeed;
  int motor_speed_step;
  int motor_inertia;
  bool motor_reverse;
  String ip_address;
  String mac_address;
};

// ----------------------------------------------------------------------------
// Definition of the LED component
// ----------------------------------------------------------------------------

struct Led {
    // state variables
    uint8_t pin;
    bool    on;

    // methods
    void update() {
        digitalWrite(pin, on ? HIGH : LOW);
    }
};

#endif
