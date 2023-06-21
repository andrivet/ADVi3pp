/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2022 Sebastien Andrivet [https://github.com/andrivet/]
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

#include "../parameters.h"
#include "../core/buzzer.h"
#include "../core/dimming.h"
#include "../../lcd/marlinui.h"

using namespace ADVi3pp;


void MarlinUI::buzz(const long, const uint16_t) {
    buzzer.buzz_on_action();
}

void MarlinUI::buzz_m300(const long duration, const uint16_t) {
  if(duration <= 0) return;
  buzzer.buzz_on_action(duration / 10);
}

void MarlinUI::sleep_display(const bool sleep) {
  if(sleep) dimming.sleep_on(); else dimming.sleep_off();
}

void MarlinUI::_set_brightness() {
  dimming.send_brightness_to_lcd();
}

void MarlinUI::pause_show_message(
  const PauseMessage message,
  const PauseMode mode/*=PAUSE_MODE_SAME*/,
  const uint8_t extruder/*=active_extruder*/
) {
  pause_mode = mode;
  ExtUI::pauseModeStatus = message;
  switch (message) {
    case PAUSE_MESSAGE_PARKING:  ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_PAUSE_PRINT_PARKING)); break;
    case PAUSE_MESSAGE_CHANGING: ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_FILAMENT_CHANGE_INIT)); break;
    case PAUSE_MESSAGE_UNLOAD:   ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_FILAMENT_CHANGE_UNLOAD)); break;
    case PAUSE_MESSAGE_WAITING:  ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_ADVANCED_PAUSE_WAITING)); break;
    case PAUSE_MESSAGE_INSERT:   ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_FILAMENT_CHANGE_INSERT)); break;
    case PAUSE_MESSAGE_LOAD:     ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_FILAMENT_CHANGE_LOAD)); break;
    case PAUSE_MESSAGE_PURGE:
      ExtUI::onUserConfirmRequired(GET_TEXT_F(TERN(ADVANCED_PAUSE_CONTINUOUS_PURGE, MSG_FILAMENT_CHANGE_CONT_PURGE, MSG_FILAMENT_CHANGE_PURGE)));
      break;
    case PAUSE_MESSAGE_RESUME:   ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_FILAMENT_CHANGE_RESUME)); break;
    case PAUSE_MESSAGE_HEAT:     ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_FILAMENT_CHANGE_HEAT)); break;
    case PAUSE_MESSAGE_HEATING:  ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_FILAMENT_CHANGE_HEATING)); break;
    case PAUSE_MESSAGE_OPTION:   ExtUI::onUserConfirmRequired(GET_TEXT_F(MSG_FILAMENT_CHANGE_OPTION_HEADER)); break;
    case PAUSE_MESSAGE_STATUS:   ExtUI::onShowStatus(); break;
    default: break;
  }
}
