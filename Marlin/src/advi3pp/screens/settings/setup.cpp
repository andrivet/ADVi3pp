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

#include "src/inc/MarlinConfig.h"
#include "setup.h"

namespace ADVi3pp::Setup {

  inline namespace internals {
#if ENABLED(BLTOUCH)
  static constexpr Page PAGE = Page::Setup;
#elif ENABLED(ADVi3PP_PROBE)
    static constexpr Page PAGE = Page::SetupNoSensor;
#else
    static constexpr Page PAGE = Page::SetupNoSensor;
#endif

    void show_command();
    void save_command();
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_SAVE: save_command(); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void show_command() {
      Pages::show(PAGE);
    }

    void save_command() {
      Pages::save(Pages::SAVE_OPTIONS::NOTHING, Pages::BACK_OPTIONS::NONE);
    }

  }
}
