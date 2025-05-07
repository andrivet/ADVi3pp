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
#include "../../core/pool.h"
#include "../common/wait.h"
#include "load_unload.h"

namespace ADVi3pp::LoadUnload {

  inline namespace internals {
    constexpr uint16_t KEY_CODE_LOAD = 1;
    constexpr uint16_t KEY_CODE_UNLOAD = 2;
    constexpr Variable VAR_TEMP = Variable::Value0;

    struct Data {
      float previous_z_ = 0;
      bool load_ = true; // false for unload
    };

    inline Data& pool() { return Pool::get<Data>(Page::LoadUnload); }

    void show_command();
    void back_command();
    void load_command();
    void unload_command();

    void prepare(float length, feedRate_t feedrate);
    void extrude();
  }

  //! Handle Load & Unload actions.
  //! @param key_value    The sub-action to handle
  //! @return             True if the action was handled
  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW:   show_command(); break;
      case KEY_CODE_BACK:   back_command(); break;
      case KEY_CODE_LOAD:   load_command(); break;
      case KEY_CODE_UNLOAD: unload_command(); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void show_command() {
      if(!Core::check_not_busy()) return;
      Pool::reset<Data>(Page::LoadUnload);
      Status::reset();
      Wait::ensure_homed([] () -> void {
        WriteRamRequest{VAR_TEMP}.write_word(ExtUI::getDefaultTemp_celsius(ExtUI::H0));
        pool().previous_z_ = Core::ensure_z_enough_room();
        Pages::show(Page::LoadUnload);
      });
    }

    void back_command() {
      ExtUI::setTargetTemp_celsius(0, ExtUI::E0);
      ExtUI::setAxisPosition_mm(pool().previous_z_, ExtUI::Z, 20);
      Pages::back(Pages::BACK_OPTIONS::FINISH_MOVE);
    }

    //! Prepare Load or Unload step #1: set the target temperature, setup the next step and display a wait message
    //! @param background Background task to detect if it is time for step #2
    void prepare(bool load) {
      ReadRam frame{VAR_TEMP};
      if(!frame.send_receive(1)) return;
      const auto target_temp = static_cast<celsius_t>(frame.read_int());

      pool().load_ = load;
      ExtUI::setTargetTemp_celsius(target_temp, ExtUI::E0, true);
      if(ExtUI::getDefaultTemp_celsius(ExtUI::H0) != target_temp) {
        ExtUI::setDefaultTemp_celsius(target_temp, ExtUI::E0);
        ExtUI::saveSettings();
      }

      ExtUI::setHostKeepaliveState(GcodeSuite::IN_PROCESS);
      auto title = load ? GET_TEXT_F(MSG_FILAMENT_CHANGE_LOAD) : GET_TEXT_F(MSG_FILAMENT_CHANGE_UNLOAD);
      Wait::wait_back(title, GET_TEXT_F(MSG_HEATING), [] () -> void {
        background_task.clear();
        Status::set(GET_TEXT_F(ADVI3PP_MSG_CANCELED), Status::STATUS_OPTIONS::RESET);
        ExtUI::setHostKeepaliveState(GcodeSuite::NOT_BUSY);
        ExtUI::setTargetTemp_celsius(0, ExtUI::E0);
      });

      background_task.set([] () -> CALLBACK_RESULT {
        if(ExtUI::getActualTemp_celsius(ExtUI::E0) < ExtUI::getTargetTemp_celsius(ExtUI::E0)) return CALLBACK_RESULT::CONTINUE;
        extrude();
        return CALLBACK_RESULT::STOP;
      });
    }

    void extrude() {
      Log::info() << F("extrude") << Log::endl();
      auto load = pool().load_;
      ExtUI::setAxisPosition_mm(
        ExtUI::getAxisPosition_mm(ExtUI::E0) + (load ? FILAMENT_CHANGE_SLOW_LOAD_LENGTH : -FILAMENT_CHANGE_SLOW_LOAD_LENGTH),
        ExtUI::E0,
        load ? FILAMENT_CHANGE_SLOW_LOAD_FEEDRATE : FILAMENT_CHANGE_UNLOAD_FEEDRATE
      );

      auto title = load ? GET_TEXT_F(MSG_FILAMENT_CHANGE_LOAD) : GET_TEXT_F(MSG_FILAMENT_CHANGE_UNLOAD);
      Wait::wait_back(title, GET_TEXT_F(ADVI3PP_MSG_IN_PROGRESS), [] () -> void {
        background_task.clear();
        Status::set(GET_TEXT_F(ADVI3PP_MSG_CANCELED), Status::STATUS_OPTIONS::RESET);
        Pages::clear_temporaries();
        ExtUI::setHostKeepaliveState(GcodeSuite::NOT_BUSY);
        ExtUI::stopMove();
      });

      background_task.set([] () -> CALLBACK_RESULT {
        if(ExtUI::isMoving()) return CALLBACK_RESULT::CONTINUE;
        ExtUI::setHostKeepaliveState(GcodeSuite::NOT_BUSY);
        Pages::clear_temporaries();
        Status::set_default();
        return CALLBACK_RESULT::STOP;
      });
    }

    //! Start Load action.
    void load_command() {
      prepare(true);
    }

    //! Start Unload action.
    void unload_command() {
      prepare(false);
    }

  }
}
