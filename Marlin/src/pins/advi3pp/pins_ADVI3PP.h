/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2025 Sebastien Andrivet [https://github.com/andrivet/]
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
#define X_STOP_PIN              54   // PF0 / A0 - D54
#define Y_STOP_PIN              24   // PA2 / D24

#if MB(ADVI3PP_I3_PLUS_51)
  #if ENABLED(BLTOUCH)
    #define Z_STOP_PIN          25   // PA3 / D25
    #define Z_MIN_PROBE_PIN     25   // PA3 / D25
    #define SERVO0_PIN          40   // PG1 / D40 - EXT1
  #else
    #define Z_STOP_PIN          23   // PA1 / D23
  #endif
#elif MB(ADVI3PP_I3_PLUS_52C)
  #define Z_STOP_PIN             6   // PH3 / D6
  #define Z_MIN_PROBE_PIN        6   // PH3 / D6
  #if ENABLED(BLTOUCH)
    #define SERVO0_PIN          40   // PG1 / D40 - EXT1
  #endif
#elif MB(ADVI3PP_I3_PLUS_54)
  #define Z_STOP_PIN             6   // PH3 / D6
  #define Z_MIN_PROBE_PIN        6   // PH3 / D6
  #if ENABLED(BLTOUCH)
    // Assummes you are using a sub board from MR.S.J.D Developments
    // https://oshwlab.com/MrGamecase/wanhao-duplicator-i3-mkii-subboard
    #define SERVO0_PIN           7   // PH4 / D7
  #endif
#else
  #error "Unknown ADVi3++ mainboard"
#endif

//
// Steppers
//
#define X_STEP_PIN              61  // PF7 / A7 - D61
#define X_DIR_PIN               62  // PK0 / A8 - D62
#define X_ENABLE_PIN            60  // PF6 / A6 - D60

#define Y_STEP_PIN              64  // PK2 / A10 - D64
#define Y_DIR_PIN               65  // PK3 / A11 - D65
#if MB(ADVI3PP_I3_PLUS_51) || MB(ADVI3PP_I3_PLUS_52C)
  #define Y_ENABLE_PIN          63  // PK1 / A9 - D63
#elif MB(ADVI3PP_I3_PLUS_54)
  #define Y_ENABLE_PIN           2  // PE4 / D2
#else
#error "Unknown i3 Plus mainboard"
#endif

#define Z_STEP_PIN              67  // PK5 / A13 - D67
#define Z_DIR_PIN               69  // PK7 / A15 - D69
#define Z_ENABLE_PIN            66  // PK4 / A12 - D66

#define E0_STEP_PIN             58  // PF4 / A4 - D58
#define E0_DIR_PIN              59  // PF5 / A5 - D59
#define E0_ENABLE_PIN           57  // PF3 / A3 - D57

//
// Temperature Sensors
//
#define TEMP_0_PIN               1  // PF1 / A1 - D55
#define TEMP_BED_PIN            14  // PK6 / A14 - D68

//
// Heaters / Fans
//
#define HEATER_0_PIN             4  // PG5 / D4
#define HEATER_BED_PIN           3  // PE5 / D3

#define FAN0_PIN                 5  // PE3 / D5

//
// Filament Runout sensor
//
#define FIL_RUNOUT_PIN          36 // PC1 / D36 - EXT3

//
// SD card
//
#define SD_DETECT_PIN           49  // PL0 / D49
#define SD_MISO_PIN             50  // PB3 / D50
#define SD_MOSI_PIN             51  // PB2 / D51
#define SD_SCK_PIN              52  // PB1 / SCK - D52
#define SD_SS_PIN               53  // PB0 / SS - D53

//
// Misc. Functions
//
#define LED_PIN                 13  // PB7 / D13

// Power Control
// Only Balco (mainboard v5.2c) has a built-in PSU controller
#if MB(ADVI3PP_I3_PLUS_52C)
#define PS_ON_PIN                8   // PH5 / D8
#else
// Add-on (for e. BIGTREETECH Relay V1.2)
#define PS_ON_PIN               32  // PC5 / D32 - EXT7
#endif

// Power Loss
// Only Mark II (mainboard v5.4) has a built-in Powerloss detection
// POWER_LOSS_PIN is an alias of PL_PIN in Configuration_adv.h
#if MB(ADVI3PP_I3_PLUS_54)
#define PL_PIN                  63  // PK1 / A9 - D63
#elif MB(ADVI3PP_I3_PLUS_52C)
// Add-on (for e. BIGTREETECH Relay V1.2) on existing connector (J3)
#define PL_PIN                  2   // PE4 / D2
#else
// Add-on (for e. BIGTREETECH Relay V1.2)
#define PL_PIN                  34  // PC3 / D34 - EXT5
#endif

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
 * ############################################
 * # Pin | ATMEGA2560 Pin | Arduino # ADVi3++ #
 * ############################################
 * #  1  | 52 / PG1 (!RD) |    40   # SERVO0_PIN #
 * #  2  | 95 / PF2 (A2)  |    56   # POWER_LOSS_PIN #
 * #  3  | 54 / PC1 (A9)  |    36   # FIL_RUNOUT_PIN #
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
