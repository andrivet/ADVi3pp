/**
 * Marlin 3D Printer Firmware
 * Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * Bi3Plus pin assignments
 */

#ifndef __AVR_ATmega2560__
  #error "Oops!  Make sure you have 'Arduino Mega' selected from the 'Tools -> Boards' menu."
#endif

#define BOARD_NAME         "Duplicator i3 Plus"

#define ARDUINO_SERIAL 1

//
// Limit Switches
//
#define X_STOP_PIN         54
#define Y_STOP_PIN         24
#define Z_STOP_PIN         23

//
// Steppers
//
#define X_STEP_PIN         61
#define X_DIR_PIN          62
#define X_ENABLE_PIN       60

#define Y_STEP_PIN         64
#define Y_DIR_PIN          65
#define Y_ENABLE_PIN       63

#define Z_STEP_PIN         67
#define Z_DIR_PIN          69
#define Z_ENABLE_PIN       66
#define Z_MIN_PROBE_PIN    25

#define E0_STEP_PIN        58
#define E0_DIR_PIN         59
#define E0_ENABLE_PIN      57

//
// Temperature Sensors
//
#define TEMP_0_PIN         1
#define TEMP_BED_PIN       14

//
// Heaters / Fans
//
#define HEATER_0_PIN       4
#define HEATER_BED_PIN     3

#define FAN_PIN            5

//
// Misc. Functions
//
#define SDSS               53
#define LED_PIN            13
