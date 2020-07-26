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
#include "statistics.h"
#include "../../core/string.h"
#include "../../core/dgus.h"

namespace ADVi3pp {

Statistics statistics;

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page Statistics::do_prepare_page()
{
    send_stats();
    return Page::Statistics;
}

void Statistics::send_stats()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(ExtUI::getTotalPrints())
          << Uint16(ExtUI::getFinishedPrints())
          << Uint16(static_cast<uint16_t>(freeMemory()));
    frame.send();

    // Minimize the RAM used so send each value separately.
    char buffer[21];
    ADVString<16> value;

    value.set(ExtUI::getTotalPrintTime_str(buffer));
    frame.reset(Variable::LongText0);
    frame << value;
    frame.send();

    value.set(ExtUI::getLongestPrint_str(buffer));
    frame.reset(Variable::LongText1);
    frame << value;
    frame.send();

    value.set(ExtUI::getFilamentUsed_str(buffer));
    frame.reset(Variable::LongText2);
    frame << value;
    frame.send();
}

}
