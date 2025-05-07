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
#include "../../core/dgus.h"
#include "../../core/pool.h"
#include "beeper_settings.h"

namespace ADVi3pp::BeeperSettings {

  inline namespace internals {
    constexpr uint16_t KEY_CODE_ON_ACTION = 1;
    constexpr uint16_t KEY_CODE_ON_PRESS = 2;

    struct Data {
      bool buzz_on_action_ = true;
      bool buzz_on_press_ = false;
    };

    inline Data& pool() { return Pool::get<Data>(Page::BeeperSettings); }

    void show_command();
    void back_command();
    void save_command();
    void action_command();
    void press_command();

    void send_values(bool on_action, bool on_press, uint8_t duration);
    bool get_values(bool &on_action, bool &on_press, uint8_t &duration);
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: back_command(); break;
      case KEY_CODE_SAVE: save_command(); break;
      case KEY_CODE_ON_ACTION: action_command(); break;
      case KEY_CODE_ON_PRESS: press_command(); break;
      default: return false;
    }
    return true;
  }

  void handle_duration_command(uint16_t duration) {
    // Always give a feedback (sound) even if beeps are disabled
    Buzzer::buzz(duration);
  }

  inline namespace internals {
    void show_command() {
      Pool::reset<Data>(Page::BeeperSettings);
      pool().buzz_on_action_ = Buzzer::is_option_enabled(Buzzer::OPTIONS::ON_ACTION);
      pool().buzz_on_press_ = Buzzer::is_option_enabled(Buzzer::OPTIONS::ON_TOUCH);
      send_values(pool().buzz_on_action_, pool().buzz_on_press_, ui.tone_duration);
      Pages::show(Page::BeeperSettings);
    }

    void back_command() {
      Pages::back(Pages::BACK_OPTIONS::NONE);
    }

    void save_command() {
      bool on_action, on_press; uint8_t duration;
      if(get_values(on_action, on_press, duration))
        ui.set_tone(duration, (on_press ? 1 : 0) | (on_action ? 2 : 0));
      Pages::save(Pages::SAVE_OPTIONS::SETTINGS | Pages::SAVE_OPTIONS::MESSAGE, Pages::BACK_OPTIONS::NONE);
    }

    void send_values(bool on_action, bool on_press, uint8_t duration) {
      WriteRamRequest{Variable::Value0}.write_words(on_action, on_press);
      WriteRamRequest{Variable::BeepDuration}.write_words(duration);
    }

    bool get_values(bool &on_action, bool &on_press, uint8_t &duration) {
      ReadRam frame{Variable::Value0};
      if(!frame.send_receive(2)) return false;

      on_action = frame.read_bool();
      on_press = frame.read_bool();

      ReadRam frame2{Variable::BeepDuration};
      if(!frame2.send_receive(1)) return false;

      duration = frame2.read_uint();
      Log::info() << duration << Log::endl();
      return true;
    }

    void action_command() {
      pool().buzz_on_action_ = !pool().buzz_on_action_;
      WriteRamRequest{Variable::Value0}.write_words(pool().buzz_on_action_);
    }

    void press_command() {
      pool().buzz_on_press_ = !pool().buzz_on_press_;
      WriteRamRequest{Variable::Value1}.write_words(pool().buzz_on_press_);
    }

  }
}
