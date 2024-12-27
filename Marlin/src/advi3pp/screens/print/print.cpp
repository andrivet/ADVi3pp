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
#include "../../core/status.h"
#include "../common/wait.h"
#include "print.h"


namespace ADVi3pp::Print {
  inline namespace internals {
    constexpr uint16_t KEY_CODE_STOP = 1;
    constexpr uint16_t KEY_CODE_PAUSE = 2;
    constexpr uint16_t KEY_CODE_ADVANCED = 3;

    void show_command();
    void back_command();
    void stop_command();
    void pause_command();
    void advanced_pause_command();
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: back_command(); break;
      case KEY_CODE_STOP: stop_command(); break;
      case KEY_CODE_PAUSE: pause_command(); break;
      case KEY_CODE_ADVANCED: advanced_pause_command(); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void show_command() {
      Pages::clear_temporaries(false);
      Pages::show(Page::Print);
    }

    void back_command() {
      Pages::back(Pages::BACK_OPTIONS::NONE);
    }

    //! Stop printing
    void stop_command() {
      if(!Core::is_printing()) return;
      Wait::wait_back_continue(GET_TEXT_F(ADVI3PP_TITLE_CONFIRM), GET_TEXT_F(ADVI3PP_MSG_ABORT_PRINT), [] (CALLBACK_SOURCE src) -> void {
        if(src == CALLBACK_SOURCE::BACK) { // Cancel abort print
          Status::set(GET_TEXT_F(ADVI3PP_MSG_PRINT_CONTINUE), Status::STATUS_OPTIONS::RESET);
          return;
        }

        Pages::clear_temporaries();
        ExtUI::stopPrint();
        Wait::not_busy();
      });
    }

    //! Pause printing
    void pause_command() {
      if(!Core::is_printing() || Core::is_print_paused()) return;
      Wait::wait(GET_TEXT_F(MSG_PAUSE_PRINT));
      ExtUI::pausePrint();
    }

    //! Advanced Pause for filament change
    void advanced_pause_command() {
      if(!Core::is_printing()) return;
      Wait::wait(GET_TEXT_F(MSG_PAUSING));
      Core::inject_commands(F("M600"));
    }

  }
}
