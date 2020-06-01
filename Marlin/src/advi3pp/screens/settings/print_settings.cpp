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

#include "print_settings.h"
#include "../../core/dgus.h"

namespace ADVi3pp {

//! List of multipliers in Print Settings
const double BABYSTEPS_MULTIPLIERS[] = {0.04, 0.08, 0.12};

PrintSettings print_settings;


//! Handle Print Settings command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool PrintSettings::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::Baby1:       multiplier_ = Multiplier::M1; break;
        case KeyValue::Baby2:       multiplier_ = Multiplier::M2; break;
        case KeyValue::Baby3:       multiplier_ = Multiplier::M3; break;
        default:                    return false;
    }

    send_data();
    return true;
}

//! Get the value corresponding the the current multiplier.
//! @return The value of the current multiplier, or the first one in the case of an invalid multiplier
double PrintSettings::get_multiplier_value() const
{
    if(multiplier_ < Multiplier::M1 || multiplier_ > Multiplier::M3)
    {
        Log::error() << F("Invalid multiplier value: ") << static_cast<uint16_t >(multiplier_) << Log::endl();
        return BABYSTEPS_MULTIPLIERS[0];
    }

    return BABYSTEPS_MULTIPLIERS[static_cast<uint16_t>(multiplier_)];
}

//! Send the current data to the LCD panel.
void PrintSettings::send_data() const
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(static_cast<uint16_t>(multiplier_));
    frame.send();
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page PrintSettings::do_prepare_page()
{
    send_data();
    return Page::PrintSettings;
}

//! Handle the -Feedrate command
void PrintSettings::feedrate_minus_command()
{
    auto feedrate = ExtUI::getFeedrate_percent();
    if(feedrate <= 50)
        return;

    ExtUI::setFeedrate_percent(feedrate - 1);
}

//! Handle the +Feedrate command
void PrintSettings::feedrate_plus_command()
{
    auto feedrate = ExtUI::getFeedrate_percent();
    if(feedrate >= 150)
        return;

    ExtUI::setFeedrate_percent(feedrate + 1);
}

//! Handle the -Fan command
void PrintSettings::fan_minus_command()
{
    auto speed = ExtUI::getTargetFan_percent(ExtUI::FAN0);
    if(speed <= 0)
        return;

    speed = speed <= 5 ? 0 : speed - 5;
    ExtUI::setTargetFan_percent(speed, ExtUI::FAN0);
}

//! Handle the +Fan command
void PrintSettings::fan_plus_command()
{
    auto speed = ExtUI::getTargetFan_percent(ExtUI::FAN0);
    if(speed >= 100)
        return;

    speed = speed >= 100 - 5 ? 100 : speed + 5;
    ExtUI::setTargetFan_percent(speed, ExtUI::FAN0);
}

//! Handle the -Hotend Temperature command
void PrintSettings::hotend_minus_command()
{
    auto temperature = ExtUI::getTargetTemp_celsius(ExtUI::E0);
    if(temperature <= 0)
        return;

    ExtUI::setTargetTemp_celsius(temperature - 1, ExtUI::E0);
}

//! Handle the +Hotend Temperature command
void PrintSettings::hotend_plus_command()
{
    auto temperature = ExtUI::getTargetTemp_celsius(ExtUI::E0);
    if(temperature >= 300)
        return;

    ExtUI::setTargetTemp_celsius(temperature + 1, ExtUI::E0);
}

//! Handle the -Bed Temperature command
void PrintSettings::bed_minus_command()
{
    auto temperature = ExtUI::getTargetTemp_celsius(ExtUI::BED);
    if(temperature <= 0)
        return;

    ExtUI::setTargetTemp_celsius(temperature - 1, ExtUI::BED);
}

//! Handle the +Bed Temperature command
void PrintSettings::bed_plus_command()
{
    auto temperature = ExtUI::getTargetTemp_celsius(ExtUI::BED);
    if(temperature >= 180)
        return;

    ExtUI::setTargetTemp_celsius(temperature + 1, ExtUI::BED);
}

//! Handle the -Babystep command
void PrintSettings::baby_minus_command()
{
    auto distance = -ExtUI::mmToWholeSteps(get_multiplier_value(), ExtUI::Z);
    ExtUI::smartAdjustAxis_steps(distance, ExtUI::Z, true);
}

//! Handle the +Babystep command
void PrintSettings::baby_plus_command()
{
    auto distance = ExtUI::mmToWholeSteps(get_multiplier_value(), ExtUI::Z);
    ExtUI::smartAdjustAxis_steps(distance, ExtUI::Z, true);
}

}
