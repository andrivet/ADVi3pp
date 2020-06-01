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

#include "automatic_leveling.h"

namespace ADVi3pp {

AutomaticLeveling automatic_leveling;

#ifdef ADVi3PP_PROBE

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page AutomaticLeveling::do_prepare_page()
{
    if(!print.ensure_not_printing())
        return Page::None;
    sensor_interactive_leveling_ = true;
    pages.save_forward_page();
    wait.show(F("Homing..."));
    enqueue_and_echo_commands_P(PSTR("G28 F6000"));             // homing
    enqueue_and_echo_commands_P(PSTR("G1 Z4 F1200"));           // raise head
    enqueue_and_echo_commands_P(PSTR("G29 E"));                 // leveling
    enqueue_and_echo_commands_P(PSTR("G28 X Y F6000"));         // go back to corner. Assumes RESTORE_LEVELING_AFTER_G28

    return Page::None;
}

//! Called by Marlin when G29 (automatic bed leveling) is finished.
//! @param success Boolean indicating if the leveling was successful or not.
void AutomaticLeveling::g29_leveling_finished(bool success)
{
    if(!success)
    {
        if(sensor_interactive_leveling_)
            wait.show(F("Leveling failed"), WaitCallback{this, &AutomaticLeveling::g29_leveling_failed});
        else
            advi3pp.set_status(F("Leveling failed"));

        sensor_interactive_leveling_ = false;
        return;
    }

    advi3pp.reset_status();

    if(sensor_interactive_leveling_)
    {
        sensor_interactive_leveling_ = false;
        leveling_grid.show(ShowOptions::None);
    }
    else
    {
        settings.save();
        // From gcode_M420
        set_bed_leveling_enabled(true);
        // Error if leveling failed to enable or reenable
        if(!planner.leveling_active)
        {
            SERIAL_ERROR_START();
            SERIAL_ERRORLNPGM(MSG_ERR_M420_FAILED);
        }
    }
}

//! Show the back page when G29 (automatic bed leveling) failed.
bool AutomaticLeveling::g29_leveling_failed()
{
    pages.show_back_page();
    return true;
}

#else

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page AutomaticLeveling::do_prepare_page()
{
    return Page::NoSensor;
}

#endif

}
