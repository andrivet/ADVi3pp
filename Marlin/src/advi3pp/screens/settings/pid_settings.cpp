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
#include "../../core/dgus.h"
#include "../../core/core.h"
#include "pid_settings.h"

namespace ADVi3pp::PidSettings {

  inline namespace internals {
    void show_command();
    void back_command();
    void save_command();

    void to_lcd();
    void from_lcd();
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: back_command(); break;
      case KEY_CODE_SAVE: save_command(); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void show_command() {
      if(!Core::check_not_busy()) return;
      to_lcd();
      Pages::show(Page::PidSettings);
    }

    void back_command() {
      ExtUI::loadSettings(); // To restore previous PID
      Pages::back(Pages::BACK_OPTIONS::NONE);
    }

    //! Save the PID settings
    void save_command() {
      from_lcd();
      Pages::save(Pages::SAVE_OPTIONS::SETTINGS | Pages::SAVE_OPTIONS::MESSAGE, Pages::BACK_OPTIONS::NONE);
    }

    //! Send the current data to the LCD panel.
    void to_lcd() {
      const hotend_pid_t &pid = thermalManager.temp_hotend[0].pid;
      WriteRamRequest{Variable::Value0}.write_words(
          pid.p() * 100,
          pid.i() * 100,
          pid.d() * 100
      );
    }

    //! Save the settings from the LCD Panel.
    void from_lcd() {
      ReadRam response{Variable::Value0};
      if(!response.send_receive(3)) return;

      auto p = response.read_uint() / 100.0f;
      auto i = response.read_uint() / 100.0f;
      auto d = response.read_uint() / 100.0f;

      SET_HOTEND_PID(Kp, 0, p);
      SET_HOTEND_PID(Ki, 0, i);
      SET_HOTEND_PID(Kd, 0, d);
      thermalManager.updatePID();
    }

  }
}
