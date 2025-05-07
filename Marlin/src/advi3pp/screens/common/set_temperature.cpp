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

#include "src/inc/MarlinConfig.h"
#include "src/lcd/extui/ui_api.h"
#include "../../core/dgus.h"
#include "../../core/status.h"
#include "../common/wait.h"
#include "set_temperature.h"

namespace ADVi3pp::SetTemperature {

  inline namespace internals {
    constexpr uint8_t KEY_CODE_NONE = 1;
    constexpr uint8_t KEY_CODE_DEFAULT = 2;
    constexpr Variable VAR_TEMPERATURE = Variable::Value0;
    constexpr Variable VAR_MODE = Variable::Value1;

    void (*callback_)(CALLBACK_SOURCE); // Do not use the pool as this may be called from another screen
    ExtUI::heater_t heater_;
    celsius_t temp_;

    void back_command();
    void save_command();
    void none_command();
    void default_command();

    void send_temperature(celsius_t temp);
    void send_mode(bool none);
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_BACK: back_command(); break;
      case KEY_CODE_SAVE: save_command(); break;
      case KEY_CODE_NONE: none_command(); break;
      case KEY_CODE_DEFAULT: default_command(); break;
      default: return false;
    }
    return true;
  }

  void extruder(void (*cb)(CALLBACK_SOURCE), OPTIONS options) {
    send_temperature(ExtUI::getDefaultTemp_celsius(ExtUI::H0));
    send_mode(options == OPTIONS::INIT_ZERO);
    callback_ = cb;
    heater_ = ExtUI::H0;
    Pages::clear_temporaries(false);
    Pages::show(Page::ExtruderTemperature);
  }

  void bed(void (*cb)(CALLBACK_SOURCE), OPTIONS options) {
    send_temperature(ExtUI::getDefaultTemp_celsius(ExtUI::BED));
    send_mode(options == OPTIONS::INIT_ZERO);
    callback_ = cb;
    heater_ = ExtUI::BED;
    Pages::clear_temporaries(false);
    Pages::show(Page::BedTemperature);
  }

  inline namespace internals {

    void back_command() {
      Log::info() << F("SetTemperature::back_command") << Log::endl();
      if(callback_) callback_(CALLBACK_SOURCE::BACK); else Pages::back(Pages::BACK_OPTIONS::NONE);
    }

    void save_command() {
      Log::info() << F("ExtruderTemperature::save_command") << Log::endl();

      ReadRam frame{VAR_TEMPERATURE};
      if(!frame.send_receive(2)) return;
      const auto temp = static_cast<celsius_t>(frame.read_int());
      const auto def = frame.read_bool();

      if(def) {
        temp_ = temp;
        ExtUI::setTargetTemp_celsius(temp, heater_, true);
        if(temp != ExtUI::getDefaultTemp_celsius(heater_)) {
          ExtUI::setDefaultTemp_celsius(temp, heater_);
          ExtUI::saveSettings();
        }

        Wait::wait_back(GET_TEXT_F(ADVI3PP_TITLE_WAIT), GET_TEXT_F(MSG_HEATING), [] () -> void {
          ExtUI::setTargetTemp_celsius(0, heater_, true);
        });

        background_task.set([] () -> CALLBACK_RESULT {
          if(ExtUI::getActualTemp_celsius(heater_) < temp_) return CALLBACK_RESULT::CONTINUE;
          Pages::clear_temporaries();
          Pages::clear_current();
          callback_(CALLBACK_SOURCE::SAVE);
          return CALLBACK_RESULT::STOP;
        });
      }
      else {
        Pages::clear_current();
        callback_(CALLBACK_SOURCE::SAVE);
      }
    }

    void none_command() {
      send_mode(true);
    }

    void default_command() {
      send_mode(false);
    }

    void send_temperature(celsius_t temp) {
      WriteRamRequest{VAR_TEMPERATURE}.write_word(temp);
    }

    void send_mode(bool none) {
      WriteRamRequest{VAR_MODE}.write_word(none ? 0 : 1);
    }

  }
}

