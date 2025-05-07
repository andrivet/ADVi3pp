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
#include "power_loss_settings.h"

#if ENABLED(POWER_LOSS_RECOVERY)
namespace ADVi3pp::PowerLossSettings {

  inline namespace internals {
    constexpr auto ENABLE = Variable::Value0;
    constexpr auto TRIGGER = Variable::Value2;
    constexpr auto SIGNAL = Variable::Value3;
    constexpr auto PULL = Variable::Value4;
    constexpr uint16_t KEY_CODE_ENABLE = 1;
    constexpr uint16_t KEY_CODE_HIGH_TO_LOW = 2;
    constexpr uint16_t KEY_CODE_LOW_TO_HIGH = 3;

    struct Data {
      TriggerWidget<POWER_LOSS_STATE == LOW ? TRANSITION::HIGH_2_LOW : TRANSITION::LOW_2_HIGH, TRIGGER, SIGNAL, PULL, PL_PIN> trigger_widget_;
    };

    inline Data& pool() { return Pool::get<Data>(Page::PowerLossSettings); }

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
      Pool::reset<Data>(Page::PowerLossSettings);
      pool().trigger_widget_.set_inverted(ExtUI::getPowerLossRecoveryInverted());

      WriteRamRequest{ENABLE}.write_words(
          ExtUI::getPowerLossRecoveryEnabled(),
          ExtUI::getPowerLossRecoveryPurge(),
          pool().trigger_widget_.get_trigger_state(),
          pool().trigger_widget_.get_signal_state(),
          static_cast<uint16_t>(pool().trigger_widget_.get_signal_pull())
      );

      background_task.set([] () -> CALLBACK_RESULT {
        pool().trigger_widget_.send_signal_state();
        return CALLBACK_RESULT::CONTINUE;
      });
      Pages::show(Page::PowerLossSettings);
    }

    void save_command() {
      ReadRam response{ENABLE};
      if(!response.send_receive(3)) return;
      auto enabled = response.read_bool();
      auto length = response.read_uint();
      pool().trigger_widget_.set_trigger_state(response.read_enum<TRANSITION>());

      ExtUI::setPowerLossRecoveryEnabled(enabled);
      ExtUI::setPowerLossRecoveryPurge(length);
      ExtUI::setPowerLossRecoveryInverted(pool().trigger_widget_.get_inverted());

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
