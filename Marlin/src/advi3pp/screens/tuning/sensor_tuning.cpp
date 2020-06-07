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
#include "sensor_tuning.h"

namespace ADVi3pp {

SensorTuning sensor_tuning;


#ifdef ADVi3PP_PROBE

//! Execute a Sensor Tuning command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool SensorTuning::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::SensorSelfTest:  self_test_command(); break;
        case KeyValue::SensorReset:     reset_command(); break;
        case KeyValue::SensorDeploy:    deploy_command(); break;
        case KeyValue::SensorStow:      stow_command(); break;
        default:                        return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page SensorTuning::do_prepare_page()
{
    if(!print.ensure_not_printing())
        return Page::None;
    return Page::SensorTuning;
}

//! Execute the sensor Self-test command
void SensorTuning::self_test_command()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S120"));
}

//! Execute the sensor Reset command
void SensorTuning::reset_command()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S160"));
}

//! Execute the sensor Deploy command
void SensorTuning::deploy_command()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S10"));
}

//! Execute the sensor Stow command
void SensorTuning::stow_command()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S90"));
}

#else

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page SensorTuning::do_prepare_page()
{
    return Page::NoSensor;
}

#endif

}
