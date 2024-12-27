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
#include "../../core/pool.h"
#include "baby_steps.h"

#if ENABLED(BABYSTEPPING)

namespace ADVi3pp::BabySteps {

  inline namespace internals {
    enum class Multiplier: uint8_t { M1, M2, M3 };
    constexpr uint16_t KEY_CODE_BABY1 = 1;
    constexpr uint16_t KEY_CODE_BABY2 = 2;
    constexpr uint16_t KEY_CODE_BABY3 = 3;
    constexpr uint16_t KEY_CODE_MINUS = 4;
    constexpr uint16_t KEY_CODE_PLUS = 5;

    struct Data {
      Multiplier multiplier_ = Multiplier::M1;
      bool save_ = false;
    };

    inline Data& pool() { return Pool::get<Data>(Page::BabySteps); }

    void show_command();
    void back_command();
    void save_command();
    void minus_command();
    void plus_command();

    void send_multiplier();
    int get_multiplier_value();
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: back_command(); break;
      case KEY_CODE_SAVE: save_command(); break;
      case KEY_CODE_BABY1: pool().multiplier_ = Multiplier::M1; break;
      case KEY_CODE_BABY2: pool().multiplier_ = Multiplier::M2; break;
      case KEY_CODE_BABY3: pool().multiplier_ = Multiplier::M3; break;
      case KEY_CODE_MINUS: minus_command(); break;
      case KEY_CODE_PLUS: plus_command(); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void show_command() {
      Pool::reset<Data>(Page::BabySteps);
      pool().save_ = false;
      send_multiplier();
      background_task.set([] () -> CALLBACK_RESULT {
        WriteRamRequest{Variable::Value1}.write_words(lround(100.0 * ExtUI::getZOffset_mm()));
        return CALLBACK_RESULT::CONTINUE;
      });
      Pages::save_forward_page();
      Pages::show(Page::BabySteps);
    }

    void back_command() {
      if(pool().save_) ExtUI::saveSettings();
      Pages::back(Pages::BACK_OPTIONS::NONE);
    }

    void save_command() {
      Pages::save(Pages::SAVE_OPTIONS::SETTINGS, Pages::BACK_OPTIONS::NONE);
    }

    //! Handle the -Babystep command
    void minus_command() {
      pool(). save_= true;
      ExtUI::smartAdjustAxis_steps(-get_multiplier_value(), ExtUI::Z, true);
    }

    //! Handle the +Babystep command
    void plus_command() {
      pool().save_= true;
      ExtUI::smartAdjustAxis_steps(get_multiplier_value(), ExtUI::Z, true);
    }

    // Use a function to save RAM
    int get_multiplier(size_t index) {
        switch(index) {
          case 0: return 8;
          case 1: return 16;
          case 2: return 32;
          default: Log::error() << F("Invalid baby steps multiplier index") << Log::endl(); break;
        }
        return 1;
    }

    //! Get the value corresponding the the current multiplier.
    //! @return The value of the current multiplier (in steps), or the first one in the case of an invalid multiplier
    int get_multiplier_value() {
      if(pool().multiplier_ < Multiplier::M1 || pool().multiplier_ > Multiplier::M3) {
        Log::error() << F("Invalid multiplier value: ") << static_cast<uint16_t >(pool().multiplier_) << Log::endl();
        return get_multiplier(0);
      }

      return get_multiplier(static_cast<uint16_t>(pool().multiplier_));
    }

    //! Send the current data to the LCD panel.
    void send_multiplier() {
      WriteRamRequest{Variable::Value0}.write_words(pool().multiplier_);
    }

  }
}

#endif
