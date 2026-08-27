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

#include "../../../inc/MarlinConfig.h"
#include "../../../lcd/extui/ui_api.h"
#include "print_options.h"

namespace ADVi3pp::PrintOptions {

  inline namespace internals {
    constexpr uint16_t KEY_CODE_HOTEND_MINUS = 1;
    constexpr uint16_t KEY_CODE_HOTEND_PLUS = 2;
    constexpr uint16_t KEY_CODE_BED_MINUS = 3;
    constexpr uint16_t KEY_CODE_BED_PLUS = 4;
    constexpr uint16_t KEY_CODE_FAN_MINUS = 5;
    constexpr uint16_t KEY_CODE_FAN_PLUS = 6;
    constexpr uint16_t KEY_CODE_FEEDRATE_MINUS = 7;
    constexpr uint16_t KEY_CODE_FEEDRATE_PLUS = 8;
    constexpr uint16_t KEY_CODE_FLOWRATE_MINUS = 9;
    constexpr uint16_t KEY_CODE_FLOWRATE_PLUS = 10;

    void feedrate_minus_command();
    void feedrate_plus_command();
    void flowrate_minus_command();
    void flowrate_plus_command();
    void fan_minus_command();
    void fan_plus_command();
    void hotend_minus_command();
    void hotend_plus_command();
    void bed_minus_command();
    void bed_plus_command();
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW:           Pages::show(Page::PrintOptions); break;
      case KEY_CODE_BACK:           Pages::back(Pages::BACK_OPTIONS::NONE); break;
      case KEY_CODE_HOTEND_MINUS:   hotend_minus_command(); break;
      case KEY_CODE_HOTEND_PLUS:    hotend_plus_command(); break;
      case KEY_CODE_BED_MINUS:      bed_minus_command(); break;
      case KEY_CODE_BED_PLUS:       bed_plus_command(); break;
      case KEY_CODE_FAN_MINUS:      fan_minus_command(); break;
      case KEY_CODE_FAN_PLUS:       fan_plus_command(); break;
      case KEY_CODE_FEEDRATE_MINUS: feedrate_minus_command(); break;
      case KEY_CODE_FEEDRATE_PLUS:  feedrate_plus_command(); break;
      case KEY_CODE_FLOWRATE_MINUS: flowrate_minus_command(); break;
      case KEY_CODE_FLOWRATE_PLUS:  flowrate_plus_command(); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void back_command() {
      Pages::back(Pages::BACK_OPTIONS::NONE);
    }

    //! Handle the -Feedrate command
    void feedrate_minus_command() {
      auto feedrate = ExtUI::getFeedrate_percent();
      if(feedrate <= 50) return;
      ExtUI::setFeedrate_percent(feedrate - 1);
    }

    //! Handle the +Feedrate command
    void feedrate_plus_command() {
      auto feedrate = ExtUI::getFeedrate_percent();
      if(feedrate >= 150) return;
      ExtUI::setFeedrate_percent(feedrate + 1);
    }

    //! Handle the -Flowrate command
    void flowrate_minus_command() {
      auto flowrate = ExtUI::getFlow_percent(ExtUI::E0);
      if(flowrate <= 50) return;
      ExtUI::setFlow_percent(flowrate - 1, ExtUI::E0);
    }

    //! Handle the +Flowrate command
    void flowrate_plus_command() {
      auto flowrate = ExtUI::getFlow_percent(ExtUI::E0);
      if(flowrate >= 150) return;
      ExtUI::setFlow_percent(flowrate + 1, ExtUI::E0);
    }

    //! Handle the -Fan command
    void fan_minus_command() {
      auto speed = ExtUI::getTargetFan_percent(ExtUI::FAN0);
      if(speed <= 0) return;
      speed = speed <= 5 ? 0 : speed - 5;
      ExtUI::setTargetFan_percent(speed, ExtUI::FAN0);
    }

    //! Handle the +Fan command
    void fan_plus_command() {
      auto speed = ExtUI::getTargetFan_percent(ExtUI::FAN0);
      if(speed >= 100) return;
      speed = speed >= 100 - 5 ? 100 : speed + 5;
      ExtUI::setTargetFan_percent(speed, ExtUI::FAN0);
    }

    //! Handle the -Hotend Temperature command
    void hotend_minus_command() {
      auto temperature = ExtUI::getTargetTemp_celsius(ExtUI::E0);
      if(temperature <= 0) return;
      ExtUI::setTargetTemp_celsius(temperature - 1, ExtUI::H0);
    }

    //! Handle the +Hotend Temperature command
    void hotend_plus_command() {
      auto temperature = ExtUI::getTargetTemp_celsius(ExtUI::E0);
      if(temperature >= 300) return;
      ExtUI::setTargetTemp_celsius(temperature + 1, ExtUI::H0);
    }

    //! Handle the -Bed Temperature command
    void bed_minus_command() {
      auto temperature = ExtUI::getTargetTemp_celsius(ExtUI::BED);
      if(temperature <= 0) return;
      ExtUI::setTargetTemp_celsius(temperature - 1, ExtUI::BED);
    }

    //! Handle the +Bed Temperature command
    void bed_plus_command() {
      auto temperature = ExtUI::getTargetTemp_celsius(ExtUI::BED);
      if(temperature >= 180) return;
      ExtUI::setTargetTemp_celsius(temperature + 1, ExtUI::BED);
    }

  }
}
