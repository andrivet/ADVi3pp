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
#include "step_settings.h"
#include "../../core/dgus.h"

namespace ADVi3pp::StepSettings {

  inline namespace internals {
    constexpr unsigned SCALE = 10;

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
          ExtUI::getAxisSteps_per_mm(ExtUI::X) * SCALE,
          ExtUI::getAxisSteps_per_mm(ExtUI::Y) * SCALE,
          ExtUI::getAxisSteps_per_mm(ExtUI::Z) * SCALE,
          ExtUI::getAxisSteps_per_mm(ExtUI::E0) * SCALE
      );
      Pages::show(Page::StepsSettings);
    }

    //! Save the Steps settings
    void save_command() {
      ReadRam response{Variable::Value0};
      if(!response.send_receive(4)) return;
      auto x = response.read_uint() * 1.;
      auto y = response.read_uint() * 1.;
      auto z = response.read_uint() * 1.;
      auto e = response.read_uint() * 1.;

      ExtUI::setAxisSteps_per_mm(x / SCALE, ExtUI::X);
      ExtUI::setAxisSteps_per_mm(y / SCALE, ExtUI::Y);
      ExtUI::setAxisSteps_per_mm(z / SCALE, ExtUI::Z);
      ExtUI::setAxisSteps_per_mm(e / SCALE, ExtUI::E0);

      Pages::save(Pages::SAVE_OPTIONS::SETTINGS | Pages::SAVE_OPTIONS::MESSAGE, Pages::BACK_OPTIONS::NONE);
    }

  }
}
