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

#include "../parameters.h"
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
    adv::array<uint16_t, 2> data = {
        static_cast<uint16_t>(ExtUI::getActualTemp_celsius(ExtUI::BED)),
        static_cast<uint16_t>(ExtUI::getActualTemp_celsius(ExtUI::E0))
    };

    WriteCurveRequest{0b00000011}.write_words(data);
}

//! Clear the graphs
void Graphs::clear()
{
    WriteRegisterRequest{Register::TrendlineClear}.write_byte(0x55);
}

}
