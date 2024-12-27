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

#include "../../core/core.h"
#include "powerloss_recovery.h"

namespace ADVi3pp::PowerLossRecovery {
  inline namespace internals {
    void back_command();
    void resume_command();
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: Pages::show(Page::PowerLossRecovery); break;
      case KEY_CODE_BACK: back_command(); break;
      case KEY_CODE_SAVE: resume_command(); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void back_command() {
      Pages::back(Pages::BACK_OPTIONS::NONE);
      Core::inject_commands(F("M1000C"));
    }

    void resume_command() {
      Pages::back(Pages::BACK_OPTIONS::NONE);
      Core::inject_commands(F("M1000"));
    }

  }
}