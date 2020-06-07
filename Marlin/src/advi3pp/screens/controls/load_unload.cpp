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
#include "load_unload.h"
#include "../core/wait.h"
#include "../../core/core.h"
#include "../../core/settings.h"
#include "../../core/dgus.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "HidingNonVirtualFunction"

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
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(settings.get_last_used_temperature(TemperatureKind::Hotend));
    frame.send();
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
    ReadRamData frame{Variable::Value0, 1};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Target Temperature)") << Log::endl();
        return;
    }

    Uint16 temperature; frame >> temperature;
    ExtUI::setTargetTemp_celsius(temperature.word, ExtUI::E0);
}

//! Start Load action.
void LoadUnload::load_command()
{
    prepare();
    ExtUI::injectCommands_P(PSTR("M701"));
}

//! Start Unload action.
void LoadUnload::unload_command()
{
    prepare();
    ExtUI::injectCommands_P(PSTR("M702"));
}

//! Handle back from the Load or Unload LCD screen: stop the process.
//! @return true to continue Back processing
bool LoadUnload::stop()
{
    Log::log() << F("Load/Unload Stop") << Log::endl();
    // TODO actual implementation
    return true;
}


}
