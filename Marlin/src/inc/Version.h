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

// @advi3++: Change the name depending of the model of the printer and the model of the sensor
#if defined(ADVi3PP_54)
    #define ADVi3PP_NAME "ADVi3++MarkII"
#elif defined(ADVi3PP_52C)
    #if defined(BLTOUCH)
      #define ADVi3PP_NAME "ADVi3++HE180021-BLTouch"
    #else
      #define ADVi3PP_NAME "ADVi3++HE180021"
    #endif
#else
    #if defined(BLTOUCH)
        #define ADVi3PP_NAME "ADVi3++BLTouch"
    #else
        #define ADVi3PP_NAME "ADVi3++"
    #endif
#endif

/**
 * Release version. Leave the Marlin version or apply a custom scheme.
 */
#ifndef SHORT_BUILD_VERSION
  #define SHORT_BUILD_VERSION "2.1.2.2"
#endif

/**
 * Verbose version identifier containing a unique identifier, such as the
 * vendor name, download location, GitHub account, etc.
 */
#ifndef DETAILED_BUILD_VERSION
  #define DETAILED_BUILD_VERSION SHORT_BUILD_VERSION " (" ADVi3PP_NAME " 5.8.0)"
#endif

/**
 * The STRING_DISTRIBUTION_DATE represents when the binary file was built,
 * here we define this default string as the date where the latest release
 * version was tagged.
 */
#ifndef STRING_DISTRIBUTION_DATE
  #define STRING_DISTRIBUTION_DATE "2024-03-17"
#endif

/**
 * Minimum Configuration.h and Configuration_adv.h file versions.
 * Set based on the release version number. Used to catch an attempt to use
 * older configurations. Override these if using a custom versioning scheme
 * to alert users to major changes.
 */

#define MARLIN_HEX_VERSION 02010202
#ifndef REQUIRED_CONFIGURATION_H_VERSION
  #define REQUIRED_CONFIGURATION_H_VERSION MARLIN_HEX_VERSION
#endif
#ifndef REQUIRED_CONFIGURATION_ADV_H_VERSION
  #define REQUIRED_CONFIGURATION_ADV_H_VERSION MARLIN_HEX_VERSION
#endif

/**
 * The protocol for communication to the host. Protocol indicates communication
 * standards such as the use of ASCII, "echo:" and "error:" line prefixes, etc.
 * (Other behaviors are given by the firmware version and capabilities report.)
 */
#ifndef PROTOCOL_VERSION
  #define PROTOCOL_VERSION "1.0"
#endif

/**
 * Define a generic printer name to be output to the LCD after booting Marlin.
 */
#ifndef MACHINE_NAME
  #define MACHINE_NAME "3D Printer"
#endif

/**
 * Website where users can find Marlin source code for the binary installed on the
 * device. Override this if you provide public source code download. (GPLv3 requires
 * providing the source code to your customers.)
 */
#ifndef SOURCE_CODE_URL
  #define SOURCE_CODE_URL "https://github.com/andrivet/ADVi3pp/" // @advi3++
#endif

/**
 * Default generic printer UUID.
 */
#ifndef DEFAULT_MACHINE_UUID
  #define DEFAULT_MACHINE_UUID "44b2f5d6-e7e4-47bf-be81-c2a6b4fc7975" // @advi3++
#endif

  /**
   * The WEBSITE_URL is the location where users can get more information such as
   * documentation about a specific Marlin release. Displayed in the Info Menu.
   */
#ifndef WEBSITE_URL
  #define WEBSITE_URL "https://community.advi3pp.com/c/user-manual" // @advi3++
#endif

/**
 * Set the vendor info the serial USB interface, if changable
 * Currently only supported by DUE platform
 */
#ifndef USB_DEVICE_VENDOR_ID
  #define USB_DEVICE_VENDOR_ID            0x03EB /* ATMEL VID */
#endif
#ifndef USB_DEVICE_PRODUCT_ID
  #define USB_DEVICE_PRODUCT_ID           0x2424 /* MSC / CDC */
#endif
//! USB Device string definitions (Optional)
#ifndef USB_DEVICE_MANUFACTURE_NAME
  #define USB_DEVICE_MANUFACTURE_NAME     WEBSITE_URL
#endif
#ifdef CUSTOM_MACHINE_NAME
  #define USB_DEVICE_PRODUCT_NAME         CUSTOM_MACHINE_NAME
#else
  #define USB_DEVICE_PRODUCT_NAME         MACHINE_NAME
#endif
#define USB_DEVICE_SERIAL_NAME            "123985739853"
