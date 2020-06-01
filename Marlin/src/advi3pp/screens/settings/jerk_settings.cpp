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

#include "jerk_settings.h"
#include "../../core/dgus.h"

namespace ADVi3pp {

JerkSettings jerks_settings;

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page JerkSettings::do_prepare_page()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(ExtUI::getAxisMaxJerk_mm_s(ExtUI::X) * 10)
          << Uint16(ExtUI::getAxisMaxJerk_mm_s(ExtUI::Y) * 10)
          << Uint16(ExtUI::getAxisMaxJerk_mm_s(ExtUI::Z) * 10)
          << Uint16(ExtUI::getAxisMaxJerk_mm_s(ExtUI::E0) * 10);
    frame.send();

    return Page::JerkSettings;
}

//! Save the Jerk settings
void JerkSettings::do_save_command()
{
    ReadRamData response{Variable::Value0, 4};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Acceleration Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z, e;
    response >> x >> y >> z >> e;

    ExtUI::setAxisMaxJerk_mm_s(x.word / 10.0, ExtUI::X);
    ExtUI::setAxisMaxJerk_mm_s(y.word / 10.0, ExtUI::Y);
    ExtUI::setAxisMaxJerk_mm_s(z.word / 10.0, ExtUI::Z);
    ExtUI::setAxisMaxJerk_mm_s(e.word / 10.0, ExtUI::E0);

    Parent::do_save_command();
}

}
