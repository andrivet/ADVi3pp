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
#include "def_acceleration_settings.h"

namespace ADVi3pp::DefAccelerationSettings {

  inline namespace internals {
    void show_command();
    void back_command();
    void save_command();
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: back_command(); break;
      case KEY_CODE_SAVE: save_command(); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void show_command() {
      WriteRamRequest{Variable::Value0}.write_words(
          ExtUI::getPrintingAcceleration_mm_s2(),
          ExtUI::getRetractAcceleration_mm_s2(),
          ExtUI::getTravelAcceleration_mm_s2(),
          ExtUI::getJunctionDeviation_mm() * 100
      );
      Pages::show(Page::DefAccelerationSettings);
    }

    void back_command() {
      Pages::back(Pages::BACK_OPTIONS::NONE);
    }

    //! Save the Acceleration settings
    void save_command() {
      ReadRam response{Variable::Value0};
      if(!response.send_receive(4)) return;

      auto printing = response.read_uint();
      auto retract = response.read_uint();
      auto travel = response.read_uint();
      auto junction = response.read_uint() / 100.0f;

      ExtUI::setPrintingAcceleration_mm_s2(printing);
      ExtUI::setRetractAcceleration_mm_s2(retract);
      ExtUI::setTravelAcceleration_mm_s2(travel);
      ExtUI::setJunctionDeviation_mm(junction);

      Pages::save(Pages::SAVE_OPTIONS::SETTINGS | Pages::SAVE_OPTIONS::MESSAGE, Pages::BACK_OPTIONS::NONE);
    }

  }
}

