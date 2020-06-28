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
#include "pause.h"
#include "../core/wait.h"
#include "../../core/logging.h"

namespace ADVi3pp {

Pause pause;


//! Show Advance Pause message (called from Marlin).
//! @param message Message to display.
void Pause::show_message(PauseMessage message)
{
    switch (message)
    {
        case PAUSE_MESSAGE_PAUSING:     show(GET_TEXT(MSG_PAUSE_PRINT_INIT), false); break;
        case PAUSE_MESSAGE_CHANGING:    show(GET_TEXT(MSG_FILAMENT_CHANGE_INIT), false); break;
        case PAUSE_MESSAGE_UNLOAD:      show(GET_TEXT(MSG_FILAMENT_CHANGE_UNLOAD), false); break;
        case PAUSE_MESSAGE_WAITING:     show(GET_TEXT(MSG_ADVANCED_PAUSE_WAITING), false); break;
        case PAUSE_MESSAGE_INSERT:      show(GET_TEXT(MSG_FILAMENT_CHANGE_INSERT), true); break;
        case PAUSE_MESSAGE_LOAD:        show(GET_TEXT(MSG_FILAMENT_CHANGE_LOAD), false); break;
        case PAUSE_MESSAGE_PURGE:       show(GET_TEXT(MSG_FILAMENT_CHANGE_PURGE), false); break;
        case PAUSE_MESSAGE_RESUME:      show(GET_TEXT(MSG_FILAMENT_CHANGE_RESUME), false); break;
        case PAUSE_MESSAGE_HEAT:        show(GET_TEXT(MSG_FILAMENT_CHANGE_HEAT), false); break;
        case PAUSE_MESSAGE_HEATING:     show(GET_TEXT(MSG_FILAMENT_CHANGE_HEATING), false); break;
        case PAUSE_MESSAGE_OPTION:      options();
        case PAUSE_MESSAGE_STATUS:      break;

        default: Log::log() << F("Unknown PauseMessage: ") << static_cast<uint16_t>(message) << Log::endl(); break;
    }
}

void Pause::show(PGM_P message, bool withContinue)
{
    if(withContinue)
        wait.show(reinterpret_cast<const FlashChar*>(message),
                  WaitCallback{this, &Pause::wait_back},
                  WaitCallback{this, &Pause::wait_continue},
                  ShowOptions::None);
    else
        wait.show(reinterpret_cast<const FlashChar*>(message),
                  WaitCallback{this, &Pause::wait_back},
                  ShowOptions::None);
}

void Pause::options()
{
    pause_menu_response = PAUSE_RESPONSE_RESUME_PRINT; // TODO change that, display new screen
    wait.show(F("Press Continue when the filament comes out of the nozzle..."),
              WaitCallback{this, &Pause::wait_back},
              WaitCallback{this, &Pause::options_continue},
              ShowOptions::None);
}

bool Pause::wait_back()
{
    ExtUI::setUserConfirmed();
    return true;
}

bool Pause::wait_continue()
{
    return true;
}

bool Pause::options_continue()
{
    pause_menu_response = PAUSE_RESPONSE_RESUME_PRINT;
    return true;
}


//! Show "Insert filament" message during Advance Pause
void Pause::insert_filament()
{
    wait.show_continue(F("Insert filament and press continue..."), WaitCallback{this, &Pause::filament_inserted}, ShowOptions::None);
}

//! Action to execute once the filament is inserted (Continue command)
bool Pause::filament_inserted()
{
    ExtUI::setUserConfirmed();
    wait.show(F("Filament inserted.."), ShowOptions::None);
    return false;
}

}
