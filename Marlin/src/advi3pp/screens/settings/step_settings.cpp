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
#include "step_settings.h"
#include "../../core/dgus.h"

namespace ADVi3pp {

StepSettings steps_settings;

static const unsigned SCALE = 10;

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page StepSettings::do_prepare_page()
{
    WriteRamRequest{Variable::Value0}.write_words(
        ExtUI::getAxisSteps_per_mm(ExtUI::X) * SCALE,
        ExtUI::getAxisSteps_per_mm(ExtUI::Y) * SCALE,
        ExtUI::getAxisSteps_per_mm(ExtUI::Z) * SCALE,
        ExtUI::getAxisSteps_per_mm(ExtUI::E0) * SCALE
    );
    return Page::StepsSettings;
}

//! Save the Steps settings
void StepSettings::do_save_command()
{
    ReadRam response{Variable::Value0};
    if(!response.send_receive(4))
    {
        Log::error() << F("Receiving Frame (Steps Settings)") << Log::endl();
        return;
    }

    uint16_t x = response.read_word();
    uint16_t y = response.read_word();
    uint16_t z = response.read_word();
    uint16_t e = response.read_word();

    ExtUI::setAxisSteps_per_mm(static_cast<float>(x) / SCALE, ExtUI::X);
    ExtUI::setAxisSteps_per_mm(static_cast<float>(y) / SCALE, ExtUI::Y);
    ExtUI::setAxisSteps_per_mm(static_cast<float>(z) / SCALE, ExtUI::Z);
    ExtUI::setAxisSteps_per_mm(static_cast<float>(e) / SCALE, ExtUI::E0);

    Parent::do_save_command();
}


}
