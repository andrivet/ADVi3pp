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
//! @param message Message to dislay.
void Pause::show_message(PauseMessage message)
{
    if(message == last_advanced_pause_message_)
        return;
    last_advanced_pause_message_ = message;

    // TODO handle all the messages
    switch (message)
    {
        case PAUSE_MESSAGE_PAUSING:                    wait.show(F("Pausing...")); break;
        case PAUSE_MESSAGE_UNLOAD:                     wait.set_message(F("Unloading filament...")); break;
        case PAUSE_MESSAGE_INSERT:                     insert_filament(); break;
        case PAUSE_MESSAGE_LOAD:                       wait.set_message(F("Loading...")); break;
        case PAUSE_MESSAGE_PURGE:                      wait.set_message(F("Extruding some filament...")); break;
        //case PAUSE_MESSAGE_CLICK_TO_HEAT_NOZZLE:       wait.set_message(F("Press continue to heat")); break;
        case PAUSE_MESSAGE_RESUME:                     wait.set_message(F("Resuming print...")); break;
        case PAUSE_MESSAGE_STATUS:                     break;
        //case PAUSE_MESSAGE_WAIT_FOR_NOZZLES_TO_HEAT:   wait.set_message(F("Waiting for heat...")); break;
        //case PAUSE_MESSAGE_OPTION:                     pause_menu_response = ADVANCED_PAUSE_RESPONSE_RESUME_PRINT; break;
        default: Log::log() << F("Unknown AdvancedPauseMessage: ") << static_cast<uint16_t>(message) << Log::endl(); break;
    }
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
