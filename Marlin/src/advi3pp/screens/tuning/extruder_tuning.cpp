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
#include "../../core/core.h"
#include "../../core/dgus.h"
#include "../../core/status.h"
#include "../../core/pool.h"
#include "../common/wait.h"
#include "extruder_tuning.h"

namespace ADVi3pp::ExtruderTuning {

  inline namespace internals {
    constexpr uint16_t FILAMENT_TO_EXTRUDE = 100; //!< Filament to extrude (10 cm)
    constexpr uint16_t FEEDRATE = 1.0; //< Feedrate to extrude the filament
    constexpr uint16_t REMAINING_FILAMENT = 20; //!< Amount of filament supposes tp remain after extruding (2 cm)
    constexpr uint16_t KEY_CODE_START = 1;
    constexpr uint16_t KEY_CODE_FINISH = 2;
    constexpr uint16_t KEY_CODE_BACK_STEP1 = 3;
    constexpr uint16_t KEY_CODE_BACK_STEP2 = 4;
    constexpr Variable VAR_TEMP = Variable::Value0;
    constexpr Variable VAR_FEEDRATE = Variable::Value1;
    constexpr Variable VAR_DISTANCE = Variable::Value2;
    constexpr Variable VAR_OLD_STEPS = Variable::Value3;
    constexpr Variable VAR_NEW_STEPS = Variable::Value4;

    struct Data {
      float extruded_ = 0.0;
      float previous_z_ = 0.0;
      feedRate_t feedrate_ = FEEDRATE;
    };

    inline Data& pool() { return Pool::get<Data>(Page::ExtruderTuning1); }

    void show_command();
    void back_command();
    void save_command();
    void start_command();
    void finish_command();

    void extrude();
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: back_command(); break;
      case KEY_CODE_BACK_STEP1: Pages::back(Pages::BACK_OPTIONS::FINISH_MOVE); break;
      case KEY_CODE_BACK_STEP2: Pages::back(Pages::BACK_OPTIONS::NONE); break;
      case KEY_CODE_SAVE: save_command(); break;
      case KEY_CODE_START: start_command(); break;
      case KEY_CODE_FINISH: finish_command(); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void show_command() {
      if(!Core::check_not_busy()) return;
      Pool::reset<Data>(Page::ExtruderTuning1);
      Status::reset();
      Pages::save_forward_page();
      WriteRamRequest{VAR_TEMP}.write_words(
          static_cast<uint16_t>(ExtUI::getDefaultTemp_celsius(ExtUI::H0)),
          static_cast<uint16_t>(FEEDRATE * 10.0f)
      );
      pool().previous_z_ = Core::ensure_z_enough_room();
      Pages::show(Page::ExtruderTuning1);
    }

    void back_command() {
      ExtUI::setAxisPosition_mm(pool().previous_z_, ExtUI::Z, 20);
      Pages::back(Pages::BACK_OPTIONS::FINISH_MOVE);
    }

    //! Start extruder tuning.
    void start_command() {
      ReadRam frame{VAR_TEMP};
      if (!frame.send_receive(2)) return;
      auto temperature = static_cast<celsius_t>(frame.read_uint());
      pool().feedrate_ = frame.read_uint() / 10.0f;

      ExtUI::setTargetTemp_celsius(temperature, ExtUI::E0, true);
      if(ExtUI::getDefaultTemp_celsius(ExtUI::H0) != temperature) {
        ExtUI::setDefaultTemp_celsius(temperature, ExtUI::E0);
        ExtUI::saveSettings();
      }

      Wait::wait_back(GET_TEXT_F(ADVI3PP_MSG_EXTRUDER_TUNING), GET_TEXT_F(MSG_HEATING), [] () -> void {
        ExtUI::setHostKeepaliveState(GcodeSuite::NOT_BUSY);
        ExtUI::setTargetTemp_celsius(0, ExtUI::E0);
      });

      background_task.set([] () -> CALLBACK_RESULT {
        if(ExtUI::getActualTemp_celsius(ExtUI::E0) < ExtUI::getTargetTemp_celsius(ExtUI::E0))
          return CALLBACK_RESULT::CONTINUE;
        extrude();
        return CALLBACK_RESULT::STOP;
      });

      ExtUI::setHostKeepaliveState(GcodeSuite::IN_PROCESS);
    }

    void extrude() {
      pool().extruded_ = ExtUI::getAxisPosition_mm(ExtUI::E0);
      ExtUI::setAxisPosition_mm(pool().extruded_ + FILAMENT_TO_EXTRUDE, ExtUI::E0, pool().feedrate_);

      Wait::wait_back(GET_TEXT_F(MSG_EXTRUDE), GET_TEXT_F(ADVI3PP_TITLE_WAIT), [] () -> void {
        ExtUI::setHostKeepaliveState(GcodeSuite::NOT_BUSY);
        background_task.clear();
        ExtUI::setTargetTemp_celsius(0, ExtUI::E0);
        ExtUI::stopMove();
      });

      background_task.set([] () -> CALLBACK_RESULT {
        if(ExtUI::isMoving()) return CALLBACK_RESULT::CONTINUE;

        Status::set(GET_TEXT_F(ADVI3PP_MSG_EXTRUSION_FINISHED), Status::STATUS_OPTIONS::RESET);
        ExtUI::setHostKeepaliveState(GcodeSuite::NOT_BUSY);
        ExtUI::setTargetTemp_celsius(0, ExtUI::E0);

        pool().extruded_ = ExtUI::getAxisPosition_mm(ExtUI::E0) - pool().extruded_;

        // Always set default to 20mm
        WriteRamRequest{VAR_DISTANCE}.write_word(REMAINING_FILAMENT * 10);
        Pages::show(Page::ExtruderTuning2);
        return CALLBACK_RESULT::STOP;
      });
    }

    void finish_command() {
      ReadRam frame{VAR_DISTANCE};
      if(!frame.send_receive(1)) return;
      auto e = frame.read_uint() / 10.0f;
      // Note: e is divided by 10 because the LCD panel gives a value in 0.1 mm unit
      // Formula: new_value = old_value * theoretical_extruded / actual_extruded
      auto previous_value = ExtUI::getAxisSteps_per_mm(ExtUI::E0);
      auto new_value = previous_value * pool().extruded_ / (pool().extruded_ + REMAINING_FILAMENT - e);
      Log::info() << F("New value") << new_value << Log::endl();

      WriteRamRequest{VAR_OLD_STEPS}.write_words(
          static_cast<uint16_t>(previous_value * 10.0f),
          static_cast<uint16_t>(new_value * 10.0f)
      );

      Pages::show(Page::ExtruderTuning3);
      ExtUI::setAxisPosition_mm(pool().previous_z_, ExtUI::Z, 20);
    }

    void save_command() {
      ReadRam frame{VAR_NEW_STEPS};
      if(!frame.send_receive(1)) return;
      auto value = frame.read_uint() / 10.0f;
      ExtUI::setAxisSteps_per_mm(value, ExtUI::E0);
      Pages::save(Pages::SAVE_OPTIONS::SETTINGS | Pages::SAVE_OPTIONS::MESSAGE, Pages::BACK_OPTIONS::FINISH_MOVE);
    }
  }
}
