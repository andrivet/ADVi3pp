/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2020 Sebastien Andrivet [https://github.com/andrivet/]
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#pragma once

/**
 * Wanhao Duplicator i3 Plus pin assignments for ADVi3++ firmware
 * This file is an adaptation of ramps/pins_DUPLICATOR_I3_PLUS.h which itself is an adaptation of my own work.
 */

#define REQUIRE_MEGA2560
#include "env_validate.h"

#define BOARD_INFO_NAME       "ADVi3++"
#define DEFAULT_MACHINE_NAME  BOARD_INFO_NAME

//
// Limit Switches
//
#define X_STOP_PIN              54   // PF0 / ADC0 - A0
#define Y_STOP_PIN              24   // PA2 / AD2

#if MB(ADVI3PP_I3_PLUS_51)
  #if ENABLED(BLTOUCH)
    #define Z_STOP_PIN          25   // PA3 / AD3
    #define Z_MIN_PROBE_PIN     25   // PA3 / AD3
    #define SERVO0_PIN          40   // PG1 / !RD
  #else
    #define Z_STOP_PIN          23   // PA1 / AD1
  #endif
#elif MB(ADVI3PP_I3_PLUS_52C)
  #define Z_STOP_PIN             6   // PH3 / PWM6
  #define Z_MIN_PROBE_PIN        6   // PH3 / PCINT8
  #if ENABLED(BLTOUCH)
    #define SERVO0_PIN          40   // PG1 / !RD
  #endif
#elif MB(ADVI3PP_I3_PLUS_54)
  #define Z_STOP_PIN             6   // PH3 / PCINT8
  #define Z_MIN_PROBE_PIN        6   // PH3 / PCINT8
#else
  #error "Unknown ADVi3++ mainboard"
#endif

//
// Steppers
//
#define X_STEP_PIN              61  // PF7 / A7
#define X_DIR_PIN               62  // PK0 / A8
#define X_ENABLE_PIN            60  // PF6 / A6

#define Y_STEP_PIN              64  // PK2 / A10
#define Y_DIR_PIN               65  // PK3 / A11
#if MB(ADVI3PP_I3_PLUS_51) || MB(ADVI3PP_I3_PLUS_52C)
  #define Y_ENABLE_PIN          63  // PK1 / A9
#elif MB(ADVI3PP_I3_PLUS_54)
  #define Y_ENABLE_PIN           2  // PE4 / INT4
#else
#error "Unknown i3 Plus mainboard"
#endif

#define Z_STEP_PIN              67  // PK5 / A13
#define Z_DIR_PIN               69  // PK7 / A15
#define Z_ENABLE_PIN            66  // PK4 / A12

#define E0_STEP_PIN             58  // PF4 / A4
#define E0_DIR_PIN              59  // PF5 / A5
#define E0_ENABLE_PIN           57  // PF3 / A3

//
// Temperature Sensors
//
#define TEMP_0_PIN              1  // PF1 / A1   Analog
#define TEMP_BED_PIN           14  // PK6 / A14  Analog

//
// Heaters / Fans
//
#define HEATER_0_PIN            4  // PG5 / PWM4
#define HEATER_BED_PIN          3  // PE5 / PWM3

#define FAN_PIN                 5  // PE3 / PWM5

//
// Filament Runout sensor
//
#define FIL_RUNOUT_PIN          36 // PC1 / A9

//
// SD card
//
#define SD_DETECT_PIN          49  // PL0
#define SD_MISO_PIN            50  // PB3
#define SD_MOSI_PIN            51  // PB2
#define SD_SCK_PIN             52  // PB1
#define SD_SS_PIN              53  // PB0 / SS
#define SDSS                   SD_SS_PIN

//
// Misc. Functions
//
#define LED_PIN                13  // PB7 / PWM13




/**
 *  == EXT connector ==
 *
 *         2  4  6  8  10
 *       #---------------#
 *  #2   | °  °  °  °  ° |
 *  #1   | °  °  °  °  ° |
 *       #---------------#
 *         1  3  5  7  9
 *
 * ##################################
 * # Pin | ATMEGA2560 Pin | Arduino #
 * ##################################
 * #  1  | 52 / PG1 (!RD) |    40   #
 * #  2  | 95 / PF2 (A2)  |    56   #
 * #  3  | 54 / PC1 (A9)  |    36   #
 * #  4  | 53 / PC0 (A8)  |    37   #
 * #  5  | 56 / PC3 (A11) |    34   #
 * #  6  | 55 / PC2 (A10) |    35   #
 * #  7  | 58 / PC5 (A13) |    32   #
 * #  8  | 57 / PC4 (A12) |    33   #
 * #  9  |       GND      |     -   #
 * # 10  |       VCC      |     +   #
 * ##################################
 *
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 *
 *  == Z-probe connector ==
 *
 *          1  2  3
 *        #---------#
 *        | °  °  ° |
 *        #---------#
 *
 * ##################################
 * # Pin | ATMEGA2560 Pin | Arduino #
 * ##################################
 * #  1  |    24V or 5V   |     +   #
 * #  2  | 75 / PA3 (AD3) |    25   #
 * #  3  |       GND      |     -   #
 * ##################################
 *
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 *
 *  == Y-endstop ==                            == Z-endstop ==                 == Bed temperature ==
 *
 *        1  2                                       1  2                               1  2
 *      #------#                                   #------#                           #------#
 *      | °  ° |                                   | °  ° |                           | °  ° |
 *      #------#                                   #------#                           #------#
 *
 * ############### Y ################ ############### Z ################ ############## BED ###############
 * # Pin | ATMEGA2560 Pin | Arduino # # Pin | ATMEGA2560 Pin | Arduino # # Pin | ATMEGA2560 Pin | Arduino #
 * ################################## ################################## ##################################
 * #  1  |       GND      |     -   # #  1  |       GND      |     -   # #  1  |       GND      |     -   #
 * #  2  | 76 / PA2 (AD2) |    24   # #  2  | 77 / PA1 (AD1) |    23   # #  2  |83 / PK6 (ADC14)|    14   #
 * ################################## ################################## ##################################
 *
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 *
 *  == SPI connector ==
 *
 *        5  3  1
 *      #---------#
 *      | °  °  ° |
 *      | °  °  ° |
 *      #---------#
 *        6  4  2
 *
 * ##################################
 * # Pin | ATMEGA2560 Pin | Arduino #
 * ##################################
 * #  1  | 22 / PB3 (MISO)|    50   #
 * #  2  |       VCC      |     +   #
 * #  3  | 20 / PB1 (SCK) |    52   #
 * #  4  | 21 / PB2 (MOSI)|    51   #
 * #  5  | 30 / !RESET    |  RESET  #
 * #  6  |       GND      |     -   #
 * ##################################
 *
 * Pictogram by Ludy https://github.com/Ludy87
 * See: https://sebastien.andrivet.com/en/posts/wanhao-duplicator-i3-plus-3d-printer/
 */
