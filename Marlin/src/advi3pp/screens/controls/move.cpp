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
#include "move.h"
#include "../../core/core.h"

namespace ADVi3pp {

namespace {

    const millis_t MOVE_XY_DELAY = 150;
    const millis_t MOVE_Z_DELAY = 50;
    const millis_t MOVE_E_DELAY = 500;

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
//! @params commands Actual commands to move the nozzle.
void Move::move(const FlashChar* commands, millis_t delay)
{
    if(!ELAPSED(millis(), last_move_time_ + delay))
        return;
    core.inject_commands(commands);
    last_move_time_ = millis();
}

//! Move the nozzle (+X)
void Move::x_plus_command()
{
    move(F("G91\nG1 X5 F1000\nG90"), MOVE_XY_DELAY);
}

//! Move the nozzle (-X)
void Move::x_minus_command()
{
    move(F("G91\nG1 X-5 F1000\nG90"), MOVE_XY_DELAY);
}

//! Move the nozzle (+Y)
void Move::y_plus_command()
{
    move(F("G91\nG1 Y5 F1000\nG90"), MOVE_XY_DELAY);
}

//! Move the nozzle (-Y)
void Move::y_minus_command()
{
    move(F("G91\nG1 Y-5 F1000\nG90"), MOVE_XY_DELAY);
}

//! Move the nozzle (+Z)
void Move::z_plus_command()
{
    move(F("G91\nG1 Z0.5 F240\nG90"), MOVE_Z_DELAY);
}

//! Move the nozzle (-Z)
void Move::z_minus_command()
{
    move(F("G91\nG1 Z-0.5 F240\nG90"), MOVE_Z_DELAY);
}

//! Extrude some filament.
void Move::e_plus_command()
{
    move(F("G91\nG1 E1 F120\nG90"), MOVE_E_DELAY);
}

//! Unextrude some filament.
void Move::e_minus_command()
{
    move(F("G91\nG1 E-1 F120\nG90"), MOVE_E_DELAY);
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
    core.inject_commands(F("G28 Z F1200"));
}

//! Go to home on all axis.
void Move::all_home_command()
{
    core.inject_commands(F("G28 F6000"));
}

}
