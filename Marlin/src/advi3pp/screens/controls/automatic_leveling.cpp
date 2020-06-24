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
#include "../../core/status.h"
#include "../core/wait.h"
#include "../../../lcd/extui/ui_api.h"
#include "leveling_grid.h"
#include "automatic_leveling.h"

namespace ADVi3pp {

AutomaticLeveling automatic_leveling;

#ifdef ADVi3PP_PROBE

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page AutomaticLeveling::do_prepare_page()
{
    if(!core.ensure_not_printing())
        return Page::None;
    sensor_interactive_leveling_ = true;
    pages.save_forward_page();
    wait.show(F("Homing..."));

    // homing, raise head, leveling, go back to corner, activate compensation
    core.inject_commands(F("G28 F6000\nG1 Z4 F1200\nG29 E\nG28 X Y F6000\nM420 S1"));

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
            status.set(F("Leveling failed"));

        sensor_interactive_leveling_ = false;
        return;
    }

    status.reset();

    if(sensor_interactive_leveling_)
    {
        sensor_interactive_leveling_ = false;
        leveling_grid.show(ShowOptions::None);
    }
    else
    {
        settings.save();
        ExtUI::setLevelingActive(true);
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
