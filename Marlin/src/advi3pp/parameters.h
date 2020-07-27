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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

// The preferred way to build ADVi3++ is with PlatformIO. With PlatformIO, you can easily build BLTouch and non-BLtouch releases.
// If for whatever reason, you prefer to use Arduino IDE, uncomment the following line to:

// build a Wanhao Duplicator i3 Plus (aka Mark I, mainboard version 5.1):
// #define ADVi3PP_51

// build an Aldi UK Balco 2018 release (mainboard version 5.2C)
// WARNING: For HE180021 models only (2018). For the 2017 models (UK) or 2019 models (Belgium), use ADVi3PP_51
// #define ADVi3PP_52C

// build a  Wanhao Duplicator i3 Plus Mark II (mainboard version 5.4):
// #define ADVi3PP_54

// build a BLTouch release.
// #define BLTOUCH

#ifdef DEBUG

// To log various aspects of ADVi3++
#define ADVi3PP_LOG

// To log the content of frames, uncomment the following line. Very chatty, but useful to debug tough bugs
//#define ADVi3PP_LOG_FRAMES
//#define ADVi3PP_LOG_ALL_FRAMES // Log all the frames, including those that are updating the status

#endif

// Define only when using ADVi3++ Hardware simulator (endstops)
//#define ADVi3PP_HARDWARE_SIMULATOR

// ---------------------------------------------------------------------------
// Do not modify the following definitions
// ---------------------------------------------------------------------------

// This is only to ensure that Jetbrains CLion is parsing code properly inside the IDE (intellisense)
#ifdef __CLION_IDE__
#define DEBUG
#define ADVi3PP_51
//#define BLTOUCH
#endif

#if defined(BLTOUCH) || defined(ADVi3PP_54)
#define ADVi3PP_PROBE 1
#endif
