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
#include "../../core/core.h"
#include "../core/wait.h"
#include "sensor_z_height.h"

namespace ADVi3pp {

SensorZHeight sensor_z_height;

#ifdef ADVi3PP_PROBE

const double SENSOR_Z_HEIGHT_MULTIPLIERS[] = {0.04, 0.12, 1.0};

//! Handle Sensor Z Height command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool SensorZHeight::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::Multiplier1:     multiplier1_command(); break;
        case KeyValue::Multiplier2:     multiplier2_command(); break;
        case KeyValue::Multiplier3:     multiplier3_command(); break;
        default:                        return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page SensorZHeight::do_prepare_page()
{
    if(!core.ensure_not_printing())
        return Page::None;
    pages.save_forward_page();

    wait.show(F("Homing..."));
    core.inject_commands(F("G28 F6000"));  // homing
    task.set_background_task(BackgroundTask(this, &SensorZHeight::post_home_task), 200);
    return Page::None;
}

//! Reset Sensor Z Height data.
void SensorZHeight::reset()
{
    multiplier_ = Multiplier::M1;
}

//! Check if the printer is homed, and continue the Z Height Tuning process.
void SensorZHeight::post_home_task()
{
    if(!ExtUI::isMachineHomed() || core.is_busy())
        return;

    task.clear_background_task();
    reset();

    ExtUI::setZOffset_mm(0);

    ExtUI::setFeedrate_mm_s(1200);
    ExtUI::setAxisPosition_mm(4, ExtUI::Z);
    ExtUI::setAxisPosition_mm(100, ExtUI::X);
    ExtUI::setAxisPosition_mm(100, ExtUI::Y);
    ExtUI::setAxisPosition_mm(0, ExtUI::Z);
    ExtUI::setSoftEndstopState(false);

    send_data();

    pages.show_page(Page::ZHeightTuning, ShowOptions::None);
}

//! Execute the Back command
void SensorZHeight::do_back_command()
{
    // enable enstops, z-home, XY-homing, compensation
    ExtUI::setSoftEndstopState(true);
    core.inject_commands(F("G28 Z F1200\nG28 X Y F6000\nM420 S1"));
    Parent::do_back_command();
}

//! Handles the Save (Continue) command
void SensorZHeight::do_save_command()
{
    // Current Z position becomes Z offset
    ExtUI::setZOffset_mm(ExtUI::getAxisPosition_mm(ExtUI::Z));
    // enable enstops, raise head, homing
    ExtUI::setSoftEndstopState(true);
    ExtUI::setFeedrate_mm_s(1200);
    ExtUI::setAxisPosition_mm(4, ExtUI::Z);
    core.inject_commands(F("G28 X Y F6000\nM420 S1"));
    Parent::do_save_command();
}

//! Change the multiplier.
void SensorZHeight::multiplier1_command()
{
    multiplier_ = Multiplier::M1;
    send_data();
}

//! Change the multiplier.
void SensorZHeight::multiplier2_command()
{
    multiplier_ = Multiplier::M2;
    send_data();
}

//! Change the multiplier.
void SensorZHeight::multiplier3_command()
{
    multiplier_ = Multiplier::M3;
    send_data();
}

//! Change the position of the nozzle (-Z).
void SensorZHeight::minus()
{
    adjust_height(-get_multiplier_value());
}

//! Change the position of the nozzle (+Z).
void SensorZHeight::plus()
{
    adjust_height(+get_multiplier_value());
}

//! Get the current multiplier value on the LCD panel.
double SensorZHeight::get_multiplier_value() const
{
    if(multiplier_ < Multiplier::M1 || multiplier_ > Multiplier::M3)
    {
        Log::error() << F("Invalid multiplier value: ") << static_cast<uint16_t >(multiplier_) << Log::endl();
        return SENSOR_Z_HEIGHT_MULTIPLIERS[0];
    }

    return SENSOR_Z_HEIGHT_MULTIPLIERS[static_cast<uint16_t>(multiplier_)];
}

//! Adjust the Z height.
//! @param offset Offset for the adjustment.
void SensorZHeight::adjust_height(double offset)
{
    ExtUI::setFeedrate_mm_s(1200);
    ExtUI::setAxisPosition_mm(ExtUI::getAxisPosition_mm(ExtUI::Z) + offset, ExtUI::Z);
    send_data();
}

//! Send the current data (i.e. multiplier) to the LCD panel.
void SensorZHeight::send_data() const
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(static_cast<uint16_t>(multiplier_));
    frame.send();
}

#else

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page SensorZHeight::do_prepare_page()
{
    return Page::NoSensor;
}

#endif

}