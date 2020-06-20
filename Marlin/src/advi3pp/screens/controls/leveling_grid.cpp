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
#include "../../core/dgus.h"
#include "leveling_grid.h"

namespace ADVi3pp {

LevelingGrid leveling_grid;

#ifdef ADVi3PP_PROBE

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page LevelingGrid::do_prepare_page()
{
    auto z_values = ExtUI::getMeshArray();

    WriteRamDataRequest frame{Variable::Value0};
    for(auto y = 0; y < GRID_MAX_POINTS_Y; y++)
        for(auto x = 0; x < GRID_MAX_POINTS_X; x++)
            frame << Uint16(static_cast<int16_t>(z_values[x][y] * 100));
    frame.send();

    return Page::SensorGrid;
}

//! Handles the Save (Continue) command
void LevelingGrid::do_save_command()
{
    // Save settings (including mesh), Set bed leveling state (enable)
    ExtUI::injectCommands_P(PSTR("M500\nM420 S1"));
    Parent::do_save_command();
}

#else

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page LevelingGrid::do_prepare_page()
{
    return Page::NoSensor;
}

#endif

}
