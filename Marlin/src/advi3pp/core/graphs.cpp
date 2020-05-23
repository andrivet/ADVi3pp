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

#include <Arduino.h>
#include "../../lcd/extui/ui_api.h"
#include "graphs.h"
#include "dgus.h"

namespace ADVi3pp {

Graphs graphs;

//! Constructor
//! Initialize the update time
Graphs::Graphs()
{
    next_update_graph_time_ = millis() + 1000L * 10; // Wait 10 sec before starting updating graphs
}

//! Update the graphs (if the update delay has elapsed)
void Graphs::update()
{
    if(!ELAPSED(millis(), next_update_graph_time_))
        return;

    send_data();
    next_update_graph_time_ = millis() + 500;
}

//! Update the graphs on the LCD panel (two channels: the bed and the hotend).
void Graphs::send_data()
{
    WriteCurveDataRequest frame{0b00000011};
    frame << Uint16{ExtUI::getActualTemp_celsius(ExtUI::BED)}
          << Uint16{ExtUI::getActualTemp_celsius(ExtUI::E0)};
    frame.send(false);
}

//! Clear the graphs
void Graphs::clear()
{
    WriteRegisterDataRequest request{Register::TrendlineClear}; // TODO: Fix this (Mini DGUS)
    request << 0x55_u8;
    request.send();
}

}
