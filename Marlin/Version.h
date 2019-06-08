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
 * This file is the standard Marlin version identifier file, all fields can be
 * overriden by the ones defined on _Version.h by using the Configuration.h
 * directive USE_AUTOMATIC_VERSIONING.
 */

// @advi3++: ADVi3++ specific defines to ease the compilation of BLTouch firmware
#include "advi3pp_defines.h"

#if ENABLED(USE_AUTOMATIC_VERSIONING)

  #include "_Version.h"

#else

  // @advi3++: Change the name depending of the model of the printer and the model of the sensor
  #if defined(ADVi3PP_MARK2)
  #define ADVi3PP_NAME "ADVi3++MarkII"
  #elif defined(ADVi3PP_HE180021)
    #if defined(ADVi3PP_BLTOUCH3)
      #define ADVi3PP_NAME "ADVi3++HE180021-BLTouch3"
    #elif defined(ADVi3PP_BLTOUCH)
      #define ADVi3PP_NAME "ADVi3++HE180021-BLTouch"
    #else
      #define ADVi3PP_NAME "ADVi3++HE180021"
    #endif
  #else
    #if defined(ADVi3PP_BLTOUCH3)
      #define ADVi3PP_NAME "ADVi3++BLTouch3"
    #elif defined(ADVi3PP_BLTOUCH)
      #define ADVi3PP_NAME "ADVi3++BLTouch"
    #else
      #define ADVi3PP_NAME "ADVi3++"
    #endif
  #endif

  /**
   * Marlin release version identifier
   */
  #define SHORT_BUILD_VERSION "1.1.9"

  /**
   * Verbose version identifier which should contain a reference to the location
   * from where the binary was downloaded or the source code was compiled.
   */
  // @advi3++: Construct build version (version number set by advbumpversion) 
  #define DETAILED_BUILD_VERSION SHORT_BUILD_VERSION " (" ADVi3PP_NAME " 4.0.0)"

  /**
   * The STRING_DISTRIBUTION_DATE represents when the binary file was built,
   * here we define this default string as the date where the latest release
   * version was tagged.
   */
  // @advi3++: Build date (set by advbumpversion) 
  #define STRING_DISTRIBUTION_DATE "2019-06-08"

  /**
   * Required minimum Configuration.h and Configuration_adv.h file versions.
   *
   * You must increment this version number for every significant change such as,
   * but not limited to: ADD, DELETE RENAME OR REPURPOSE any directive/option on
   * the configuration files.
   */
  #define REQUIRED_CONFIGURATION_H_VERSION 010109
  #define REQUIRED_CONFIGURATION_ADV_H_VERSION 010109

  /**
   * The protocol for communication to the host. Protocol indicates communication
   * standards such as the use of ASCII, "echo:" and "error:" line prefixes, etc.
   * (Other behaviors are given by the firmware version and capabilities report.)
   */
  #define PROTOCOL_VERSION "1.0"

  /**
   * Defines a generic printer name to be output to the LCD after booting Marlin.
   */
  // @advi3++: Machine name 
  #define MACHINE_NAME "Wanhao Duplicator i3 Plus"

  /**
   * The SOURCE_CODE_URL is the location where users will find the Marlin Source
   * Code which is installed on the device. In most cases —unless the manufacturer
   * has a distinct Github fork— the Source Code URL should just be the main
   * Marlin repository.
   */
  // @advi3++: ADVi3++ sources 
  #define SOURCE_CODE_URL "https://github.com/andrivet/ADVi3pp"

  /**
   * Default generic printer UUID.
   */
  // @advi3++: ADVi3++ specific UUID 
  #define DEFAULT_MACHINE_UUID "44b2f5d6-e7e4-47bf-be81-c2a6b4fc7975"

  /**
   * The WEBSITE_URL is the location where users can get more information such as
   * documentation about a specific Marlin release.
   */
  // @advi3++: ADVi3++ User Manual
  #define WEBSITE_URL "https://community.advi3pp.com/c/user-manual"

#endif // USE_AUTOMATIC_VERSIONING
