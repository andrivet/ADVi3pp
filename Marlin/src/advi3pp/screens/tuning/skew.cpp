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
#include "skew.h"

#if ENABLED(SKEW_CORRECTION)
namespace ADVi3pp::Skew {

  inline namespace internals {
    constexpr unsigned DEFAULT_AC = 1415;
    constexpr unsigned DEFAULT_BD = 1415;
    constexpr unsigned DEFAULT_AD = 1000;
    constexpr uint16_t KEY_CODE_STEP2 = 1;
    constexpr uint16_t KEY_CODE_STEP3 = 2;
    constexpr uint16_t KEY_CODE_BACK_STEP1 = 3;
    constexpr uint16_t KEY_CODE_BACK_STEP2 = 4;
    constexpr Variable VAR_XY_AC = Variable::Value0;
    constexpr Variable VAR_XZ_AC = Variable::Value3;
    constexpr Variable VAR_YZ_AC = Variable::Value6;

    void show_command();
    void save_command();
    void set_default_values();
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: Pages::back(Pages::BACK_OPTIONS::NONE); break;
      case KEY_CODE_SAVE: save_command(); break;
      case KEY_CODE_STEP2: Pages::show(Page::Skew2); break;
      case KEY_CODE_STEP3: Pages::show(Page::Skew3); break;
      case KEY_CODE_BACK_STEP1:
      case KEY_CODE_BACK_STEP2: Pages::back(Pages::BACK_OPTIONS::NONE); break;
      default: return false;
    }
    return true;
  }


  inline namespace internals {

    void show_command() {
      if(!Core::check_not_busy()) return;
      Status::reset();
      Pages::save_forward_page();
      set_default_values();
      Pages::show(Page::Skew1);
    }

    void set_default_values() {
      WriteRamRequest{VAR_XY_AC}.write_words(
        DEFAULT_AC, DEFAULT_BD, DEFAULT_AD,
        DEFAULT_AC, DEFAULT_BD, DEFAULT_AD,
        DEFAULT_AC, DEFAULT_BD, DEFAULT_AD
      );
    }

    float get_factor(Variable var) {
      ReadRam response{var};
      if(!response.send_receive(3)) return 0;
      auto ac = response.read_uint() / 10.0f;
      auto bd = response.read_uint() / 10.0f;
      auto ad = response.read_uint() / 10.0f;
      Log::info() << F("Values") << ac << bd << ad << Log::endl();

      return _SKEW_FACTOR(ac, bd, ad);
    }

    //! Save the settings
    void save_command() {
      float xy = get_factor(VAR_XY_AC);
      float xz = get_factor(VAR_XZ_AC);
      float yz = get_factor(VAR_YZ_AC);
      if(abs(xy) < 0.000001) xy = 0;
      if(abs(xz) < 0.000001) xz = 0;
      if(abs(yz) < 0.000001) yz = 0;

      if(WITHIN(xy, SKEW_FACTOR_MIN, SKEW_FACTOR_MAX) &&
         WITHIN(xz, SKEW_FACTOR_MIN, SKEW_FACTOR_MAX) &&
         WITHIN(yz, SKEW_FACTOR_MIN, SKEW_FACTOR_MAX)
      ) {
        Log::info() << F("Skew factors") << xy << xz << yz << Log::endl();
        ExtUI::setSkewFactors(xy, xz, yz);
      }

      Pages::save(Pages::SAVE_OPTIONS::SETTINGS | Pages::SAVE_OPTIONS::MESSAGE, Pages::BACK_OPTIONS::NONE);
    }

  }
}

#endif
