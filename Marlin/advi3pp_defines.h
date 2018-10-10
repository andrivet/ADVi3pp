/**
 * Marlin 3D Printer Firmware For Wanhao Duplicator i3 Plus (ADVi3++)
 *
 * Copyright (C) 2017 Sebastien Andrivet [https://github.com/andrivet/]
 *
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

#ifndef ADVI3PP_DEFINES_H
#define ADVI3PP_DEFINES_H

// The preferred way to build ADVi3++ is with PlatformIO. With PlatformIO, you can easily build BLTouch and non-BLtouch releases.
// If for whatever reason, you prefer to use Arduino IDE, uncomment the following line to build a BLTouch release.
// #define ADVi3PP_BLTOUCH

#ifdef DEBUG

// To log various aspects of ADVi3++
#define ADVi3PP_LOG

// To log the content of frames, uncomment the following line. Very chatty, but useful to debug tough bugs
#define ADVi3PP_LOG_FRAMES
#define ADVi3PP_LOG_ALL_FRAMES

#endif
#endif

