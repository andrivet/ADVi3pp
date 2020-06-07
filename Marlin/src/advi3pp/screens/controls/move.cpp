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
#include "move.h"
#include "../../core/core.h"

namespace ADVi3pp {

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
void Move::move(const char* command, millis_t delay)
{
    if(!ELAPSED(millis(), last_move_time_ + delay))
        return;
    ExtUI::injectCommands_P(PSTR("G91"));
    ExtUI::injectCommands_P(command);
    ExtUI::injectCommands_P(PSTR("G90"));
    last_move_time_ = millis();
}

//! Move the nozzle (+X)
void Move::x_plus_command()
{
    move(PSTR("G1 X4 F1000"), 150);
}

//! Move the nozzle (-X)
void Move::x_minus_command()
{
    move(PSTR("G1 X-4 F1000"), 150);
}

//! Move the nozzle (+Y)
void Move::y_plus_command()
{
    move(PSTR("G1 Y4 F1000"), 150);
}

//! Move the nozzle (-Y)
void Move::y_minus_command()
{
    move(PSTR("G1 Y-4 F1000"), 150);
}

//! Move the nozzle (+Z)
void Move::z_plus_command()
{
    move(PSTR("G1 Z0.5 F240"), 10);
}

//! Move the nozzle (-Z)
void Move::z_minus_command()
{
    move(PSTR("G1 Z-0.5 F240"), 10);
}

//! Extrude some filament.
void Move::e_plus_command()
{
    if(ExtUI::getActualTemp_celsius(ExtUI::E0) < 180)
        return;

    ExtUI::injectCommands_P(PSTR("G91"));
    ExtUI::injectCommands_P(PSTR("G1 E1 F120"));
    ExtUI::injectCommands_P(PSTR("G90"));
}

//! Unextrude some filament.
void Move::e_minus_command()
{
    if(ExtUI::getActualTemp_celsius(ExtUI::E0) < 180)
        return;

    ExtUI::injectCommands_P(PSTR("G91"));
    ExtUI::injectCommands_P(PSTR("G1 E-1 F120"));
    ExtUI::injectCommands_P(PSTR("G90"));
}

//! Disable the motors.
void Move::disable_motors_command()
{
    ExtUI::injectCommands_P(PSTR("M84"));
    ExtUI::setAllAxisUnhomed();
    ExtUI::setAllAxisPositionUnknown();
}

//! Go to home on the X axis.
void Move::x_home_command()
{
    ExtUI::injectCommands_P(PSTR("G28 X F6000"));
}

//! Go to home on the Y axis.
void Move::y_home_command()
{
    ExtUI::injectCommands_P(PSTR("G28 Y F6000"));
}

//! Go to home on the Z axis.
void Move::z_home_command()
{
    ExtUI::injectCommands_P(PSTR("G28 Z F6000"));
}

//! Go to home on all axis.
void Move::all_home_command()
{
    ExtUI::injectCommands_P(PSTR("G28 F6000"));
}

}
