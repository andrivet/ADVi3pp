/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2020 Sebastien Andrivet [https://github.com/andrivet/]
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

#include "../../parameters.h"
#include "../../core/logging.h"
#include "pause.h"
#include "../core/wait.h"

namespace ADVi3pp {

Pause pause;


//! Show Advance Pause message (called from Marlin).
//! @param message Message to display.
void Pause::show_message(PauseMessage message)
{
    switch (message)
    {
        case PAUSE_MESSAGE_PAUSING:     show(GET_TEXT(MSG_PAUSE_PRINT_INIT)); break;
        case PAUSE_MESSAGE_CHANGING:    show(GET_TEXT(MSG_FILAMENT_CHANGE_INIT)); break;
        case PAUSE_MESSAGE_UNLOAD:      show(GET_TEXT(MSG_FILAMENT_CHANGE_UNLOAD)); break;
        case PAUSE_MESSAGE_WAITING:     show(GET_TEXT(MSG_ADVANCED_PAUSE_WAITING)); break;
        case PAUSE_MESSAGE_INSERT:      show(GET_TEXT(MSG_FILAMENT_CHANGE_INSERT)); break;
        case PAUSE_MESSAGE_LOAD:        show(GET_TEXT(MSG_FILAMENT_CHANGE_LOAD)); break;
        case PAUSE_MESSAGE_PURGE:       show(GET_TEXT(MSG_FILAMENT_CHANGE_PURGE)); break;
        case PAUSE_MESSAGE_RESUME:      show(GET_TEXT(MSG_FILAMENT_CHANGE_RESUME)); break;
        case PAUSE_MESSAGE_HEAT:        show(GET_TEXT(MSG_FILAMENT_CHANGE_HEAT)); break;
        case PAUSE_MESSAGE_HEATING:     show(GET_TEXT(MSG_FILAMENT_CHANGE_HEATING)); break;
        case PAUSE_MESSAGE_OPTION:      options(); break;
        case PAUSE_MESSAGE_STATUS:      pages.show_back_page(); break;

        default: Log::log() << F("Unknown PauseMessage: ") << static_cast<uint16_t>(message) << Log::endl(); break;
    }
}

void Pause::show(PGM_P message)
{
    wait.show(reinterpret_cast<const FlashChar*>(message), ShowOptions::None);
}

void Pause::options()
{
    pause_menu_response = PAUSE_RESPONSE_WAIT_FOR;
    wait.show(F("Press on Continue to purge more filament"),
              WaitCallback{[]{ wait.show(F("Please wait..."), ShowOptions::None); pause_menu_response = PAUSE_RESPONSE_RESUME_PRINT; return false; }},
              WaitCallback{[]{ pause_menu_response = PAUSE_RESPONSE_EXTRUDE_MORE; return false; }},
              ShowOptions::None);
}

}
