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
#include "move.h"
#include "../../core/core.h"

namespace ADVi3pp {

namespace {

    const float MOVE_FEEDRATE_XY = MMM_TO_MMS(HOMING_FEEDRATE_XY);
    const float MOVE_FEEDRATE_Z = MMM_TO_MMS(HOMING_FEEDRATE_Z);
    const float MOVE_FEEDRATE_E = 1.0;
    const millis_t MOVE_DELAY = 200;
    const float MOVE_DISTANCE_XY = 2.5;
    const float MOVE_DISTANCE_Z = 0.5;
    const float MOVE_DISTANCE_E = 1;

}

Move move;


//! Execute a move command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool Move::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::MoveXHome:           x_home_command(); break;
        case KeyValue::MoveYHome:           y_home_command(); break;
        case KeyValue::MoveZHome:           z_home_command(); break;
        case KeyValue::MoveAllHome:         all_home_command(); break;
        case KeyValue::DisableMotors:       disable_motors_command(); break;
        default:                            return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page Move::do_prepare_page()
{
    if(!core.ensure_not_printing())
        return Page::None;
    ExtUI::finishAndDisableHeaters(); // To circumvent homing problems
    return Page::Move;
}

//! Move the nozzle. Check that the command is not send too early when multiple move commands are send in a short time
//! (i.e. when the user keep the button presses)
//! @params command Actual command to move the nozzle.
void Move::move(void (*commands)(), millis_t delay)
{
    if(!ELAPSED(millis(), last_move_time_ + delay))
        return;
    commands();
    last_move_time_ = millis();
}

//! Move the nozzle (+X)
void Move::x_plus_command()
{
    move([]{
        ExtUI::setFeedrate_mm_s(MOVE_FEEDRATE_XY);
        ExtUI::setAxisPosition_mm(ExtUI::getAxisPosition_mm(ExtUI::X) + MOVE_DISTANCE_XY, ExtUI::X);
    }, MOVE_DELAY);
}

//! Move the nozzle (-X)
void Move::x_minus_command()
{
    move([]{
        ExtUI::setFeedrate_mm_s(MOVE_FEEDRATE_XY);
        ExtUI::setAxisPosition_mm(ExtUI::getAxisPosition_mm(ExtUI::X) - MOVE_DISTANCE_XY, ExtUI::X);
    }, MOVE_DELAY);
}

//! Move the nozzle (+Y)
void Move::y_plus_command()
{
    move([]{
        ExtUI::setFeedrate_mm_s(MOVE_FEEDRATE_XY);
        ExtUI::setAxisPosition_mm(ExtUI::getAxisPosition_mm(ExtUI::Y) + MOVE_DISTANCE_XY, ExtUI::Y);
    }, MOVE_DELAY);
}

//! Move the nozzle (-Y)
void Move::y_minus_command()
{
    move([]{
        ExtUI::setFeedrate_mm_s(MOVE_FEEDRATE_XY);
        ExtUI::setAxisPosition_mm(ExtUI::getAxisPosition_mm(ExtUI::Y) - MOVE_DISTANCE_XY, ExtUI::Y);
    }, MOVE_DELAY);
}

//! Move the nozzle (+Z)
void Move::z_plus_command()
{
    move([]{
        ExtUI::setFeedrate_mm_s(MOVE_FEEDRATE_Z);
        ExtUI::setAxisPosition_mm(ExtUI::getAxisPosition_mm(ExtUI::Z) + MOVE_DISTANCE_Z, ExtUI::Z);
    }, MOVE_DELAY);
}

//! Move the nozzle (-Z)
void Move::z_minus_command()
{
    move([]{
        ExtUI::setFeedrate_mm_s(MOVE_FEEDRATE_Z);
        ExtUI::setAxisPosition_mm(ExtUI::getAxisPosition_mm(ExtUI::Z) - MOVE_DISTANCE_Z, ExtUI::Z);
    }, MOVE_DELAY);
}

//! Extrude some filament.
void Move::e_plus_command()
{
    move([]{
        ExtUI::setFeedrate_mm_s(MOVE_FEEDRATE_E);
        ExtUI::setAxisPosition_mm(ExtUI::getAxisPosition_mm(ExtUI::E0) + MOVE_DISTANCE_E, ExtUI::E0);
    }, MOVE_DELAY);
}

//! Unextrude some filament.
void Move::e_minus_command()
{
    move([]{
        ExtUI::setFeedrate_mm_s(MOVE_FEEDRATE_E);
        ExtUI::setAxisPosition_mm(ExtUI::getAxisPosition_mm(ExtUI::E0) - MOVE_DISTANCE_E, ExtUI::E0);
    }, MOVE_DELAY);
}

//! Disable the motors.
void Move::disable_motors_command()
{
    core.inject_commands(F("M84"));
    ExtUI::setAllAxisUnhomed();
    ExtUI::setAllAxisPositionUnknown();
}

//! Go to home on the X axis.
void Move::x_home_command()
{
    core.inject_commands(F("G28 X F6000"));
}

//! Go to home on the Y axis.
void Move::y_home_command()
{
    core.inject_commands(F("G28 Y F6000"));
}

//! Go to home on the Z axis.
void Move::z_home_command()
{
    core.inject_commands(F("G28 Z F6000"));
}

//! Go to home on all axis.
void Move::all_home_command()
{
    core.inject_commands(F("G28 F6000"));
}

}
