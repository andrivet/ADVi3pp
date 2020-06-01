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

#include "feedrate_settings.h"
#include "../../core/dgus.h"

namespace ADVi3pp {

FeedrateSettings feedrates_settings;

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page FeedrateSettings::do_prepare_page()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(ExtUI::getAxisMaxFeedrate_mm_s(ExtUI::X))
          << Uint16(ExtUI::getAxisMaxFeedrate_mm_s(ExtUI::Y))
          << Uint16(ExtUI::getAxisMaxFeedrate_mm_s(ExtUI::Z))
          << Uint16(ExtUI::getAxisMaxFeedrate_mm_s(ExtUI::E0))
          << Uint16(ExtUI::getMinFeedrate_mm_s())
          << Uint16(ExtUI::getMinTravelFeedrate_mm_s());
    frame.send();

    return Page::FeedrateSettings;
}

//! Save the Feedrate settings
void FeedrateSettings::do_save_command()
{
    ReadRamData response{Variable::Value0, 6};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Feedrate Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z, e, min, travel;
    response >> x >> y >> z >> e >> min >> travel;

    ExtUI::setAxisMaxFeedrate_mm_s(static_cast<float>(x.word), ExtUI::X);
    ExtUI::setAxisMaxFeedrate_mm_s(static_cast<float>(y.word), ExtUI::Y);
    ExtUI::setAxisMaxFeedrate_mm_s(static_cast<float>(z.word), ExtUI::Z);
    ExtUI::setAxisMaxFeedrate_mm_s(static_cast<float>(e.word), ExtUI::E0);
    ExtUI::setMinFeedrate_mm_s(static_cast<float>(min.word));
    ExtUI::setMinTravelFeedrate_mm_s(static_cast<float>(travel.word));

    Parent::do_save_command();
}

}
