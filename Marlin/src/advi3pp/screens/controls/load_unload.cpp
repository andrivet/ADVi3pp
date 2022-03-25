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

#include "../../parameters.h"
#include "load_unload.h"
#include "../core/wait.h"
#include "../../core/core.h"
#include "../../core/settings.h"
#include "../../core/dgus.h"

namespace ADVi3pp {

LoadUnload load_unload;


//! Handle Load & Unload actions.
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool LoadUnload::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::Load:    load_command(); break;
        case KeyValue::Unload:  unload_command(); break;
        default:                return false;
    }

    return true;
}

void LoadUnload::send_data()
{
    WriteRamRequest{Variable::Value0}.write_word(settings.get_last_used_temperature(TemperatureKind::Hotend));
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page LoadUnload::do_prepare_page()
{
    if(!core.ensure_not_printing())
        return Page::None;
    send_data();
    return Page::LoadUnload;
}

//! Prepare Load or Unload step #1: set the target temperature, setup the next step and display a wait message
//! @param background Background task to detect if it is time for step #2
void LoadUnload::prepare()
{
    ReadRam frame{Variable::Value0};
    if(!frame.send_receive(1))
    {
        Log::error() << F("Receiving Frame (Target Temperature)") << Log::endl();
        return;
    }

    ExtUI::setTargetTemp_celsius(frame.read_word(), ExtUI::E0);
}

//! Start Load action.
void LoadUnload::load_command()
{
    prepare();
    core.inject_commands(F("M701\nM104 S0"));
}

//! Start Unload action.
void LoadUnload::unload_command()
{
    prepare();
    core.inject_commands(F("M702\nM104 S0"));
}


}
