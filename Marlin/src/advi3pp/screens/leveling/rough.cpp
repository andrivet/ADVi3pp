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
#include "../../core/status.h"
#include "../common/wait.h"
#include "../common/set_temperature.h"
#include "rough.h"

namespace ADVi3pp::RoughLeveling {

  inline namespace internals {
    constexpr xyz_feedrate_t homing_feedrate_mm_m = HOMING_FEEDRATE_MM_M;
    constexpr float FEEDRATE_XY = MMM_TO_MMS(homing_feedrate_mm_m.x);
    constexpr float FEEDRATE_Z = MMM_TO_MMS(homing_feedrate_mm_m.z);
    constexpr uint16_t KEY_CODE_1 = 1;
    constexpr uint16_t KEY_CODE_2 = 2;
    constexpr uint16_t KEY_CODE_3 = 3;
    constexpr uint16_t KEY_CODE_4 = 4;
    constexpr uint16_t KEY_CODE_5 = 5;
    constexpr uint16_t KEY_CODE_A = 6;
    constexpr uint16_t KEY_CODE_B = 7;
    constexpr uint16_t KEY_CODE_C = 8;
    constexpr uint16_t KEY_CODE_D = 9;

    void show_command();
    void back_command();
    void point1_command();
    void point2_command();
    void point3_command();
    void point4_command();
    void point5_command();
    void pointA_command();
    void pointB_command();
    void pointC_command();
    void pointD_command();

    void event(CALLBACK_SOURCE src);
    void move(float x, float y);
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: back_command(); break;
      case KEY_CODE_1: point1_command(); break;
      case KEY_CODE_2: point2_command(); break;
      case KEY_CODE_3: point3_command(); break;
      case KEY_CODE_4: point4_command(); break;
      case KEY_CODE_5: point5_command(); break;
      case KEY_CODE_A: pointA_command(); break;
      case KEY_CODE_B: pointB_command(); break;
      case KEY_CODE_C: pointC_command(); break;
      case KEY_CODE_D: pointD_command(); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void show_command() {
      if(!Core::check_not_busy()) return;

      Status::reset();
      if(ExtUI::getActualTemp_celsius(ExtUI::E0) > 50)
        Wait::wait_back_continue(GET_TEXT_F(ADVI3PP_TITLE_WARNING), GET_TEXT_F(ADVI3PP_MSG_LEVELING_HOT), &event);
      else
        event(CALLBACK_SOURCE::SAVE);
    }

    void back_command() {
      ExtUI::setTargetTemp_celsius(0, ExtUI::BED);
    #if HAS_LEVELING
      ExtUI::setLevelingActive(true); // Enable back compensation
    #endif
      ExtUI::setAxisPosition_mm(Z_AFTER_HOMING, ExtUI::Z, FEEDRATE_Z);
      Pages::back(Pages::BACK_OPTIONS::FINISH_MOVE);
    }

    void event(CALLBACK_SOURCE src) {
      if(src == CALLBACK_SOURCE::BACK) {
        ExtUI::setTargetTemp_celsius(0, ExtUI::BED);
        Status::set(GET_TEXT_F(ADVI3PP_MSG_CANCELED), Status::STATUS_OPTIONS::RESET);
        return;
      }

      Log::info() << F("start") << Log::endl();
      ExtUI::setLevelingActive(false); // We do not want compensation during manual leveling
      Wait::homing([] () -> void {
        Log::info() << F("on_homed") << Log::endl();
        Pages::show(Page::RoughLeveling);
      }, F("G28\nG1 Z4 F1200"));
    }

    void move(float x, float y) {
      ExtUI::setAxisPosition_mm(Z_CLEARANCE_FOR_HOMING, ExtUI::Z, FEEDRATE_Z);

      float positions[2] = {x, y};
      ExtUI::axis_t axis[2] = { ExtUI::X, ExtUI::Y };
      ExtUI::setMultipleAxisPosition_mm(2, positions, axis, FEEDRATE_XY);
      ExtUI::setAxisPosition_mm(0, ExtUI::Z, FEEDRATE_Z);
    }

    //! Handle leveling point #1.
    void point1_command() {
      move(30, 30);
    }

    //! Handle leveling point #2.
    void point2_command() {
      move(30, 170);
    }

    //! Handle leveling point #3.
    void point3_command() {
      move(170, 170);
    }

    //! Handle leveling point #4.
    void point4_command() {
      move(170, 30);
    }

    //! Handle leveling point #5.
    void point5_command() {
      move(100, 100);
    }

    //! Handle leveling point #A.
    void pointA_command() {
      move(100, 30);
    }

    //! Handle leveling point #B.
    void pointB_command() {
      move(30, 100);
    }

    //! Handle leveling point #C.
    void pointC_command() {
      move(100, 170);
    }

    //! Handle leveling point #D.
    void pointD_command() {
      move(170, 100);
    }

  }
}
