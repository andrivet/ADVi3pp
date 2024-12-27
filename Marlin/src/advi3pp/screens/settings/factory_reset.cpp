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
#include "factory_reset.h"


namespace ADVi3pp::FactoryReset {

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
      Pages::show(Page::FactoryReset);
    }

    //! Handles the Save (Continue) command
    void save_command() {
      ExtUI::resetSettings();
      ExtUI::saveSettings();
      ExtUI::setAllAxisUnhomed();
      ExtUI::setAllAxisPositionUnknown();
      Pages::back_all(Pages::BACK_ALL_OPTIONS::NONE);
      Status::set(GET_TEXT_F(ADVI3PP_MSG_SETTINGS_RESET), Status::STATUS_OPTIONS::RESET);
      Core::display(Page::Setup, Core::DISPLAY_OPTIONS::CLEAR_CURRENT);
    }

  }
}
