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
 *Wanhao Duplicator i3 Plus pin assignments
 */

#include "advi3pp_defines.h"

#ifndef __AVR_ATmega2560__
  #error "Oops!  Make sure you have 'Arduino Mega' selected from the 'Tools -> Boards' menu."
#endif

#define BOARD_NAME         "Duplicator i3 Plus"

//
// Limit Switches
//
#ifdef ADVi3PP_BLTOUCH
    #define X_STOP_PIN     54   // PF0 / ADC0 - A0
    #define Y_STOP_PIN     24   // PA2 / AD2
    #define Z_STOP_PIN     25   // PA3 / AD3
    #define SERVO0_PIN     40   // PG1 / !RD
#else
    #define X_STOP_PIN     54   // PF0 / A0
    #define Y_STOP_PIN     24   // PA2 / AD2
    #define Z_STOP_PIN     23   // PA1 / AD1
#endif

//
// Steppers
//
#define X_STEP_PIN         61   // PF7 / A7
#define X_DIR_PIN          62   // PK0 / A8
#define X_ENABLE_PIN       60   // PF6 / A6

#define Y_STEP_PIN         64   // PK2 / A10
#define Y_DIR_PIN          65   // PK3 / A11
#define Y_ENABLE_PIN       63   // PK1 / A9

#define Z_STEP_PIN         67   // PK5 / A13
#define Z_DIR_PIN          69   // PK7 / A15
#define Z_ENABLE_PIN       66   // PK4 / A12

#define E0_STEP_PIN        58   // PF4 / A4
#define E0_DIR_PIN         59   // PF5 / A5
#define E0_ENABLE_PIN      57   // PF3 / A3

//
// Temperature Sensors
//
#define TEMP_0_PIN         1	// PF1 / A1
#define TEMP_BED_PIN       14	// PK6 / A14

//
// Heaters / Fans
//
#define HEATER_0_PIN       4    // PG5 / PWM4
#define HEATER_BED_PIN     3    // PE5 / PWM3
#define FAN_PIN            5    // PE3 / PWM5

// SD Card
#define SDSS               53   // PB0 / SS
#define SD_DETECT_PIN	   49   // PL0 / ICP4

//
// Misc. Functions
//
#define LED_PIN            13   // PB7 / PWM13
