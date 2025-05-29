/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
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
 * Native with a Wanhao i3 Plus like board.
 */

#ifndef BOARD_INFO_NAME
  #define BOARD_INFO_NAME "ADVi3++ Simulator"
#endif

#ifndef DEFAULT_MACHINE_NAME
  #define DEFAULT_MACHINE_NAME "ADVi3++ Simulator 1.0"
#endif

#ifndef MARLIN_EEPROM_SIZE
  #define MARLIN_EEPROM_SIZE    0x1000U  // 4K
#endif

//
// Servos
//
#define SERVO0_PIN              40

//
// Limit Switches
//
#define X_STOP_PIN              54   // PF0 / A0 - D54
#define Y_STOP_PIN              24   // PA2 / D24
#define Z_STOP_PIN              25   // PA3 / D25
#define Z_MIN_PROBE_PIN         25   // PA3 / D25

//
// Steppers
//
#define X_STEP_PIN              61  // PF7 / A7 - D61
#define X_DIR_PIN               62  // PK0 / A8 - D62
#define X_ENABLE_PIN            60  // PF6 / A6 - D60

#define Y_STEP_PIN              64  // PK2 / A10 - D64
#define Y_DIR_PIN               65  // PK3 / A11 - D65
#define Y_ENABLE_PIN            63  // PK1 / A9 - D63

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
#define LED_PIN                 13
#define PS_ON_PIN               32  // PC5 / D32 - EXT7
#define PL_PIN                  34  // PC3 / D34 - EXT5
