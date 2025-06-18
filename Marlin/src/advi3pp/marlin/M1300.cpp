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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "../../inc/MarlinConfig.h"
#include "../../gcode/gcode.h"
#include "../core/pages.h"
#include "../core/core.h"

/**
 * M1300 ADVi3++ Pages management
 *
 * B            Show the previous (back) page.
 * A            Simulate pressing Back on all pages and display the Main page.
 * T            Show the Print or Temperature page (if not printing)
 * P<page>      Show the page with the number given. Be careful, can produce unexpected results (it is there mainly for debugging).
 *
 *
 * M1301 ADVi3++ Default Hotend Temperature
 * M1302 ADVi3++ Default Bed Temperature
 *
 *  S<target>   The default temperature in current units
 */

void GcodeSuite::M1300() {
  if(parser.seen('B')) ADVi3pp::Pages::back(ADVi3pp::Pages::BACK_OPTIONS::NONE);
  if(parser.seen('A')) ADVi3pp::Pages::back_all(ADVi3pp::Pages::BACK_ALL_OPTIONS::SEND_BACK | ADVi3pp::Pages::BACK_ALL_OPTIONS::SHOW_MAIN);
  if(parser.seen('T')) ADVi3pp::Core::show_temps();

  if(parser.seen('P')) {
    uint16_t page = parser.ushortval('P');
    ADVi3pp::Pages::show(static_cast<ADVi3pp::Page>(page));
  }
}

void GcodeSuite::M1301() {
  if(parser.seenval('S')) {
    auto temp = parser.value_celsius();
    thermalManager.setDefaultHotend(temp, 0);
  }
}

void GcodeSuite::M1302() {
  if(parser.seenval('S')) {
    auto temp = parser.value_celsius();
    thermalManager.setDefaultBed(temp);
  }
}

void GcodeSuite::M1301_report(const bool forReplay) {
  TERN_(MARLIN_SMALL_BUILD, return);
  report_heading(forReplay, F("Default Hotend temperature"));
  report_echo_start(forReplay);
  SERIAL_ECHOPGM("  M1301 S", thermalManager.getDefaultHotend(0));
  SERIAL_EOL();
}

void GcodeSuite::M1302_report(const bool forReplay) {
  TERN_(MARLIN_SMALL_BUILD, return);
  report_heading(forReplay, F("Default Bed temperature"));
  report_echo_start(forReplay);
  SERIAL_ECHOPGM("  M1302 S", thermalManager.getDefaultBed());
  SERIAL_EOL();
}
