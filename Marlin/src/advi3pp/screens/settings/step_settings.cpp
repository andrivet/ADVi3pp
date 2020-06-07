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
#include "step_settings.h"
#include "../../core/dgus.h"

namespace ADVi3pp {

StepSettings steps_settings;

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page StepSettings::do_prepare_page()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(ExtUI::getAxisSteps_per_mm(ExtUI::X) * 10)
          << Uint16(ExtUI::getAxisSteps_per_mm(ExtUI::Y) * 10)
          << Uint16(ExtUI::getAxisSteps_per_mm(ExtUI::Z) * 10)
          << Uint16(ExtUI::getAxisSteps_per_mm(ExtUI::E0) * 10);
    frame.send();

    return Page::StepsSettings;
}

//! Save the Steps settings
void StepSettings::do_save_command()
{
    ReadRamData response{Variable::Value0, 4};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Steps Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z, e;
    response >> x >> y >> z >> e;

    ExtUI::setAxisSteps_per_mm(static_cast<float>(x.word) / 10, ExtUI::X);
    ExtUI::setAxisSteps_per_mm(static_cast<float>(y.word) / 10, ExtUI::Y);
    ExtUI::setAxisSteps_per_mm(static_cast<float>(z.word) / 10, ExtUI::Z);
    ExtUI::setAxisSteps_per_mm(static_cast<float>(e.word) / 10, ExtUI::E0);

    Parent::do_save_command();
}


}
