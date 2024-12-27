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
#include "feedrate_settings.h"

namespace ADVi3pp::FeedrateSettings {

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
          ExtUI::getAxisMaxFeedrate_mm_s(ExtUI::X),
          ExtUI::getAxisMaxFeedrate_mm_s(ExtUI::Y),
          ExtUI::getAxisMaxFeedrate_mm_s(ExtUI::Z),
          ExtUI::getAxisMaxFeedrate_mm_s(ExtUI::E0),
          ExtUI::getMinFeedrate_mm_s(),
          ExtUI::getMinTravelFeedrate_mm_s()
      );
      Pages::show(Page::FeedrateSettings);
    }

    //! Save the Feedrate settings
    void save_command() {
      ReadRam response{Variable::Value0};
      if(!response.send_receive(6)) return;

      auto x = response.read_word<float>();
      auto y = response.read_word<float>();
      auto z = response.read_word<float>();
      auto e = response.read_word<float>();
      auto min = response.read_word<float>();
      auto travel = response.read_word<float>();

      ExtUI::setAxisMaxFeedrate_mm_s(x, ExtUI::X);
      ExtUI::setAxisMaxFeedrate_mm_s(y, ExtUI::Y);
      ExtUI::setAxisMaxFeedrate_mm_s(z, ExtUI::Z);
      ExtUI::setAxisMaxFeedrate_mm_s(e, ExtUI::E0);
      ExtUI::setMinFeedrate_mm_s(min);
      ExtUI::setMinTravelFeedrate_mm_s(travel);

      Pages::save(Pages::SAVE_OPTIONS::SETTINGS | Pages::SAVE_OPTIONS::MESSAGE, Pages::BACK_OPTIONS::NONE);
    }

  }
}
