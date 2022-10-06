/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2022 Sebastien Andrivet [https://github.com/andrivet/]
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

#include "../../parameters.h"
#include "print_settings.h"
#include "../../core/dgus.h"

namespace ADVi3pp {

PrintSettings print_settings;


//! Handle Print Settings command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool PrintSettings::do_dispatch(KeyValue key_value) {
  if(Parent::do_dispatch(key_value))
    return true;
  return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page PrintSettings::do_prepare_page() {
  return Page::PrintSettings;
}

//! Handle the -Feedrate command
void PrintSettings::feedrate_minus_command() {
  auto feedrate = ExtUI::getFeedrate_percent();
  if(feedrate <= 50)
    return;

  ExtUI::setFeedrate_percent(feedrate - 1);
}

//! Handle the +Feedrate command
void PrintSettings::feedrate_plus_command() {
  auto feedrate = ExtUI::getFeedrate_percent();
  if(feedrate >= 150)
    return;

  ExtUI::setFeedrate_percent(feedrate + 1);
}

//! Handle the -Flowrate command
void PrintSettings::flowrate_minus_command() {
  auto flowrate = ExtUI::getFlow_percent(ExtUI::E0);
  if(flowrate <= 50)
    return;

  ExtUI::setFlow_percent(flowrate - 1, ExtUI::E0);
}

//! Handle the +Flowrate command
void PrintSettings::flowrate_plus_command() {
  auto flowrate = ExtUI::getFlow_percent(ExtUI::E0);
  if(flowrate >= 150)
    return;

  ExtUI::setFlow_percent(flowrate + 1, ExtUI::E0);
}


//! Handle the -Fan command
void PrintSettings::fan_minus_command() {
  auto speed = ExtUI::getTargetFan_percent(ExtUI::FAN0);
  if(speed <= 0)
    return;

  speed = speed <= 5 ? 0 : speed - 5;
  ExtUI::setTargetFan_percent(speed, ExtUI::FAN0);
}

//! Handle the +Fan command
void PrintSettings::fan_plus_command() {
  auto speed = ExtUI::getTargetFan_percent(ExtUI::FAN0);
  if(speed >= 100)
    return;

  speed = speed >= 100 - 5 ? 100 : speed + 5;
  ExtUI::setTargetFan_percent(speed, ExtUI::FAN0);
}

//! Handle the -Hotend Temperature command
void PrintSettings::hotend_minus_command() {
  auto temperature = ExtUI::getTargetTemp_celsius(ExtUI::E0);
  if(temperature <= 0)
    return;

  ExtUI::setTargetTemp_celsius(temperature - 1, ExtUI::E0);
}

//! Handle the +Hotend Temperature command
void PrintSettings::hotend_plus_command() {
  auto temperature = ExtUI::getTargetTemp_celsius(ExtUI::E0);
  if(temperature >= 300)
    return;

  ExtUI::setTargetTemp_celsius(temperature + 1, ExtUI::E0);
}

//! Handle the -Bed Temperature command
void PrintSettings::bed_minus_command() {
  auto temperature = ExtUI::getTargetTemp_celsius(ExtUI::BED);
  if(temperature <= 0)
    return;

  ExtUI::setTargetTemp_celsius(temperature - 1, ExtUI::BED);
}

//! Handle the +Bed Temperature command
void PrintSettings::bed_plus_command() {
  auto temperature = ExtUI::getTargetTemp_celsius(ExtUI::BED);
  if(temperature >= 180)
    return;

  ExtUI::setTargetTemp_celsius(temperature + 1, ExtUI::BED);
}

}
