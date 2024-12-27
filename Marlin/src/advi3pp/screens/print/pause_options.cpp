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

#include "../../../lcd/extui/ui_api.h"
#include "../../core/status.h"
#include "../common/wait.h"
#include "pause_options.h"

namespace ADVi3pp::PauseOptions {

  inline namespace internals {
    constexpr uint16_t KEY_CODE_EXTRUDE = 1;
    constexpr uint16_t KEY_CODE_RESUME = 2;
    constexpr uint16_t KEY_CODE_STOP = 3;

    void show_command();
    void extrude_command();
    void resume_command();
    void stop_command();
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_EXTRUDE: extrude_command(); break;
      case KEY_CODE_RESUME: resume_command(); break;
      case KEY_CODE_STOP: stop_command(); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void show_command() {
      ExtUI::setPauseMenuResponse(PAUSE_RESPONSE_WAIT_FOR);
      Pages::clear_temporaries(false);
      Pages::show(Page::PauseOptions);
    }

    void extrude_command() {
      Pages::clear_current();
      ExtUI::setPauseMenuResponse(PAUSE_RESPONSE_EXTRUDE_MORE);
    }

    void resume_command() {
      Pages::clear_current();
      Wait::wait(GET_TEXT_F(MSG_PLEASE_WAIT));
      ExtUI::setPauseMenuResponse(PAUSE_RESPONSE_RESUME_PRINT);
    }

    void stop_command() {
      Log::info() << F("PauseOptions::stop_command") << Log::endl();
      Pages::back_all(Pages::BACK_ALL_OPTIONS::SHOW_MAIN);
      ExtUI::setPauseMenuResponse(PAUSE_RESPONSE_RESUME_ABORT);
    }

  }
}
