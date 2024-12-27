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
#include "max_acceleration_settings.h"
#include "../../core/dgus.h"

namespace ADVi3pp::MaxAccelerationSettings {

  inline namespace internals {
    void show_command();
    void save_command();
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: Pages::back(Pages::BACK_OPTIONS::NONE); break;
      case KEY_CODE_SAVE: save_command(); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void show_command() {
      WriteRamRequest{Variable::Value0}.write_words(
          ExtUI::getAxisMaxAcceleration_mm_s2(ExtUI::X),
          ExtUI::getAxisMaxAcceleration_mm_s2(ExtUI::Y),
          ExtUI::getAxisMaxAcceleration_mm_s2(ExtUI::Z),
          ExtUI::getAxisMaxAcceleration_mm_s2(ExtUI::E0)
      );
      Pages::show(Page::MaxAccelerationSettings);
    }

    //! Save the Acceleration settings
    void save_command() {
      ReadRam response{Variable::Value0};
      if(!response.send_receive(4)) return;

      auto x = response.read_word<float>();
      auto y = response.read_word<float>();
      auto z = response.read_word<float>();
      auto e = response.read_word<float>();

      ExtUI::setAxisMaxAcceleration_mm_s2(x, ExtUI::X);
      ExtUI::setAxisMaxAcceleration_mm_s2(y, ExtUI::Y);
      ExtUI::setAxisMaxAcceleration_mm_s2(z, ExtUI::Z);
      ExtUI::setAxisMaxAcceleration_mm_s2(e, ExtUI::E0);

      Pages::save(Pages::SAVE_OPTIONS::SETTINGS | Pages::SAVE_OPTIONS::MESSAGE, Pages::BACK_OPTIONS::NONE);
    }

  }
}

