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
#include "input_shaping.h"

#if HAS_ZV_SHAPING
namespace ADVi3pp::InputShaping {

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
      if(!Core::check_not_busy()) return;
      WriteRamRequest{Variable::Value0}.write_words(
          ExtUI::getShapingFrequency(AxisEnum::X_AXIS),
          ExtUI::getShapingFrequency(AxisEnum::Y_AXIS),
          ExtUI::getShapingDampingRatio(AxisEnum::X_AXIS) * 100,
          ExtUI::getShapingDampingRatio(AxisEnum::Y_AXIS) * 100
      );
      Pages::show(Page::InputShaping);
    }

    void save_command() {
      ReadRam response{Variable::Value0};
      if(!response.send_receive(4)) return;
      auto xFreq = response.read_uint();
      auto yFreq = response.read_uint();
      auto xZeta = response.read_uint() / 100.f;
      auto yZeta = response.read_uint() / 100.f;

      ExtUI::setShapingFrequency(AxisEnum::X_AXIS, xFreq);
      ExtUI::setShapingFrequency(AxisEnum::Y_AXIS, yFreq);
      ExtUI::setShapingDampingRatio(AxisEnum::X_AXIS, xZeta);
      ExtUI::setShapingDampingRatio(AxisEnum::Y_AXIS, yZeta);

      Pages::save(Pages::SAVE_OPTIONS::SETTINGS | Pages::SAVE_OPTIONS::MESSAGE, Pages::BACK_OPTIONS::NONE);
    }

  }
}

#endif
