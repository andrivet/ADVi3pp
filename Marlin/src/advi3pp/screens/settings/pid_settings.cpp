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
#include "../../lib/ADVstd/endian.h"
#include "../../core/dgus.h"
#include "../../core/core.h"
#include "pid_settings.h"

namespace ADVi3pp::PidSettings {

  inline namespace internals {
    constexpr uint16_t KEY_CODE_BED = 1;
    constexpr uint16_t KEY_CODE_EXTRUDER = 2;
    constexpr uint16_t KEY_CODE_PLUS_Kp = 3;
    constexpr uint16_t KEY_CODE_MINUS_Kp = 4;
    constexpr uint16_t KEY_CODE_PLUS_Ki = 5;
    constexpr uint16_t KEY_CODE_MINUS_Ki = 6;
    constexpr uint16_t KEY_CODE_PLUS_Kd = 7;
    constexpr uint16_t KEY_CODE_MINUS_Kd = 8;
    constexpr Variable VAR_Kp = Variable::Value0;
    constexpr Variable VAR_Ki = Variable::Value2;
    constexpr Variable VAR_Kd = Variable::Value4;
    constexpr Variable VAR_HEATER = Variable::Value6;

    void show_command(bool bed);
    void back_command();
    void save_command();
    void heater_command(bool bed);
    void k_command(int increment, Variable var);

    void to_lcd(bool bed);
    void from_lcd();
  }

  bool handle_command(uint16_t key_code, uint16_t arg) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(arg); break;
      case KEY_CODE_BACK: back_command(); break;
      case KEY_CODE_SAVE: save_command(); break;
      case KEY_CODE_EXTRUDER: heater_command(false); break;
      case KEY_CODE_BED: heater_command(true); break;
      case KEY_CODE_PLUS_Kp: k_command(+1, VAR_Kp); break;
      case KEY_CODE_PLUS_Ki: k_command(+1, VAR_Ki); break;
      case KEY_CODE_PLUS_Kd: k_command(+1, VAR_Kd); break;
      case KEY_CODE_MINUS_Kp: k_command(-1, VAR_Kp); break;
      case KEY_CODE_MINUS_Ki: k_command(-1, VAR_Ki); break;
      case KEY_CODE_MINUS_Kd: k_command(-1, VAR_Kd); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void show_command(bool bed) {
      if(!Core::check_not_busy()) return;
      to_lcd(bed);
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

    void heater_command(bool bed) {
      from_lcd();
      to_lcd(bed);
    }

    void k_command(int increment, Variable var) {
      ReadRam response{var};
      if(!response.send_receive(4)) return;
      auto high = response.read_uint();
      auto low = response.read_uint();
      auto value = adv::dword_from_words(high, low) + increment;
      WriteRamRequest{var}.write_words(adv::high_word(value), adv::low_word(value));
    }

    //! Send the current data to the LCD panel.
    void to_lcd(bool bed) {
      const auto &pid = bed ? thermalManager.temp_bed.pid : thermalManager.temp_hotend[0].pid;
      auto p = static_cast<uint32_t>(pid.p() * 100);
      auto i = static_cast<uint32_t>(pid.i() * 100);
      auto d = static_cast<uint32_t>(pid.d() * 100);
      WriteRamRequest{VAR_Kp}.write_words(
          adv::high_word(p),
          adv::low_word(p),
          adv::high_word(i),
          adv::low_word(i),
          adv::high_word(d),
          adv::low_word(d),
          bed
      );
    }

    //! Save the settings from the LCD Panel.
    void from_lcd() {
      ReadRam response{VAR_Kp};
      if(!response.send_receive(7)) return;

      auto p_high = response.read_uint();
      auto p_low = response.read_uint();
      auto i_high = response.read_uint();
      auto i_low = response.read_uint();
      auto d_high = response.read_uint();
      auto d_low = response.read_uint();
      auto bed = response.read_bool();

      auto p = static_cast<float>(adv::dword_from_words(p_high, p_low)) / 100.f;
      auto i = static_cast<float>(adv::dword_from_words(i_high, i_low)) / 100.f;
      auto d = static_cast<float>(adv::dword_from_words(d_high, d_low)) / 100.f;

      if(bed) {
        thermalManager.temp_bed.pid.set_Kp(p);
        thermalManager.temp_bed.pid.set_Ki(i);
        thermalManager.temp_bed.pid.set_Kd(d);
      }
      else {
        SET_HOTEND_PID(Kp, 0, p);
        SET_HOTEND_PID(Ki, 0, i);
        SET_HOTEND_PID(Kd, 0, d);
        thermalManager.updatePID();
      }
    }

  }
}
