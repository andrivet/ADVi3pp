/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2021 Sebastien Andrivet [https://github.com/andrivet/]
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
#include "../../core/core.h"
#include "../../core/dgus.h"
#include "runout_settings.h"

namespace ADVi3pp {

RunoutSettings runout_settings;


//! Handle LCD Settings command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool RunoutSettings::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::RunoutEnable:        enable_command(); break;
        case KeyValue::RunoutHigh2Low:      high2low_command(); break;
        case KeyValue::RunoutLow2High:      low2high_command(); break;
        default:                            return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page RunoutSettings::do_prepare_page()
{
    inverted_ = ExtUI::getFilamentRunoutInverted();

    WriteRamRequest{Variable::Value0}.write_words(adv::array<uint16_t, 4>
    {
          static_cast<uint16_t>(ExtUI::getFilamentRunoutEnabled()),
          static_cast<uint16_t>(get_filament_state()),
          static_cast<uint16_t>(ExtUI::getFilamentRunoutDistance_mm() * 10),
          static_cast<uint16_t>(ExtUI::getFilamentRunoutInverted() ? 0 : 1)
    });

    background_task.set(Callback{this, &RunoutSettings::send_data}, 250);

    return Page::Runout;
}

void RunoutSettings::do_back_command()
{
    background_task.clear();
    Parent::do_back_command();
}

void RunoutSettings::do_save_command() {
    ReadRam response{Variable::Value0};
    if(!response.send_receive(4))
    {
        Log::error() << F("Receiving Frame (Runout Settings)") << Log::endl();
        return;
    }

    uint16_t enabled = response.read_word();
    [[maybe_unused]] uint16_t state = response.read_word();
    float distance = response.read_word() / 10.0f;
    uint16_t trigger = response.read_word();

    ExtUI::setFilamentRunoutEnabled(enabled == 1);
    ExtUI::setFilamentRunoutDistance_mm(distance);
    ExtUI::setFilamentRunoutInverted(trigger == 0);

    Parent::do_save_command();
}

//! Handle the Enable/Disable command
void RunoutSettings::enable_command()
{
    ReadRam response{Variable::Value0};
    if(!response.send_receive(1))
    {
        Log::error() << F("Receiving Frame (Runout Settings)") << Log::endl();
        return;
    }

    WriteRamRequest{Variable::Value0}.write_word(!response.read_word());
}

//! Handle the Trigger (High/Low) command
void RunoutSettings::high2low_command()
{
    inverted_ = false;
    WriteRamRequest{Variable::Value3}.write_word(1);
}

//! Handle the Trigger (Low/High) command
void RunoutSettings::low2high_command()
{
    inverted_ = true;
    WriteRamRequest{Variable::Value3}.write_word(0);
}

uint16_t RunoutSettings::get_filament_state() {
	auto pin_state = Core::get_pin_state(FIL_RUNOUT_PIN);
    return inverted_ ?
            (pin_state == Core::PinState::On ? 0: 1) :
            (pin_state == Core::PinState::On ? 1 : 0);
}


//! Send the current data to the LCD panel.
void RunoutSettings::send_data()
{
    WriteRamRequest{Variable::Value1}.write_word(get_filament_state());
}

}
