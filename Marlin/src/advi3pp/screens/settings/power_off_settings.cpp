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
#include "../../core/trigger_widget.h"
#include "../../core/pool.h"
#include "power_off_settings.h"

#if ENABLED(PSU_CONTROL)
namespace ADVi3pp::PowerOffSettings {

  inline namespace internals {
    constexpr auto ENABLE = Variable::Value0;
    constexpr auto TRIGGER = Variable::Value3;
    constexpr uint16_t KEY_CODE_ENABLE = 1;
    constexpr uint16_t KEY_CODE_HIGH_TO_LOW = 2;
    constexpr uint16_t KEY_CODE_LOW_TO_HIGH = 3;

    struct Data {
      TriggerWidget<PSU_ACTIVE_STATE == LOW ? TRANSITION::LOW_2_HIGH : TRANSITION::HIGH_2_LOW, TRIGGER> trigger_widget_;
    };

    inline Data& pool() { return Pool::get<Data>(Page::PowerOffSettings); }

    void show_command();
    void save_command();
    void enable_command();
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: Pages::back(Pages::BACK_OPTIONS::NONE); break;
      case KEY_CODE_SAVE: save_command(); break;
      case KEY_CODE_ENABLE: enable_command(); break;
      case KEY_CODE_HIGH_TO_LOW: pool().trigger_widget_.high2low_command(); break;
      case KEY_CODE_LOW_TO_HIGH: pool().trigger_widget_.low2high_command(); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void show_command() {
      Pool::reset<Data>(Page::PowerOffSettings);
      pool().trigger_widget_.set_inverted(ExtUI::getPsuControlInverted());

      WriteRamRequest{ENABLE}.write_words(
          ExtUI::getPsuControlEnabled(),
          ExtUI::getPsuControlTimeout() / 60,
          ExtUI::getPsuControlTemperature(),
          pool().trigger_widget_.get_trigger_state()
      );
      Pages::show(Page::PowerOffSettings);
    }

    void save_command() {
      ReadRam response{ENABLE};
      if(!response.send_receive(4)) return;
      auto enabled = response.read_bool();
      auto timeout = response.read_uint();
      auto temperature = static_cast<celsius_t>(response.read_int());
      pool().trigger_widget_.set_trigger_state(response.read_enum<TRANSITION>());

      ExtUI::setPsuControlEnabled(enabled == 1);
      ExtUI::setPsuControlTimeout(timeout * 60);
      ExtUI::setPsuControlTemperature(temperature);
      ExtUI::setPsuControlInverted(pool().trigger_widget_.get_inverted());

      Pages::save(Pages::SAVE_OPTIONS::SETTINGS | Pages::SAVE_OPTIONS::MESSAGE, Pages::BACK_OPTIONS::NONE);
    }

    void send_enable_state(bool enabled) {
      WriteRamRequest{ENABLE}.write_word(enabled);
    }

    //! Handle the Enable/Disable command
    void enable_command() {
      ReadRam response{ENABLE};
      if(!response.send_receive(1)) return;
      send_enable_state(!response.read_bool());
    }

  }
}


#endif
