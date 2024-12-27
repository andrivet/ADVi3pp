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
#include "statistics.h"
#include "../../core/dgus.h"

#if ENABLED(PRINTCOUNTER)

namespace ADVi3pp::Statistics {

  inline namespace internals {
    void show_command();
    void back_command();
    void send_stats();
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: back_command(); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void show_command() {
      send_stats();
      Pages::show(Page::Statistics);
    }

    void back_command() {
      Pages::back(Pages::BACK_OPTIONS::NONE);
    }

    void send_stats() {
      WriteRamRequest{Variable::Value0}.write_words(
        ExtUI::getTotalPrints(),
        ExtUI::getFinishedPrints(),
        freeMemory()
      );

      // Minimize the RAM used so send each value separately.
      char buffer[21];
      WriteRamRequest{Variable::LongText0}.write_text(ExtUI::getTotalPrintTime_str(buffer), LONG_TEXT_LENGTH);
      WriteRamRequest{Variable::LongText1}.write_text(ExtUI::getLongestPrint_str(buffer), LONG_TEXT_LENGTH);
      WriteRamRequest{Variable::LongText2}.write_text(ExtUI::getFilamentUsed_str(buffer), LONG_TEXT_LENGTH);
    }

  }
}
#endif
