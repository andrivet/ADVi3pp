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
#include "feedrate_settings.h"
#include "../../core/dgus.h"

namespace ADVi3pp {

FeedrateSettings feedrates_settings;

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page FeedrateSettings::do_prepare_page()
{
    WriteRamRequest{Variable::Value0}.write_words(adv::array<uint16_t, 6>
    {
        static_cast<uint16_t>(ExtUI::getAxisMaxFeedrate_mm_s(ExtUI::X)),
        static_cast<uint16_t>(ExtUI::getAxisMaxFeedrate_mm_s(ExtUI::Y)),
        static_cast<uint16_t>(ExtUI::getAxisMaxFeedrate_mm_s(ExtUI::Z)),
        static_cast<uint16_t>(ExtUI::getAxisMaxFeedrate_mm_s(ExtUI::E0)),
        static_cast<uint16_t>(ExtUI::getMinFeedrate_mm_s()),
        static_cast<uint16_t>(ExtUI::getMinTravelFeedrate_mm_s())
    });
    return Page::FeedrateSettings;
}

//! Save the Feedrate settings
void FeedrateSettings::do_save_command()
{
    ReadRam response{Variable::Value0};
    if(!response.send_receive(6))
    {
        Log::error() << F("Receiving Frame (Feedrate Settings)") << Log::endl();
        return;
    }

    uint16_t x = response.read_word();
    uint16_t y = response.read_word();
    uint16_t z = response.read_word();
    uint16_t e = response.read_word();
    uint16_t min = response.read_word();
    uint16_t travel = response.read_word();

    ExtUI::setAxisMaxFeedrate_mm_s(static_cast<float>(x), ExtUI::X);
    ExtUI::setAxisMaxFeedrate_mm_s(static_cast<float>(y), ExtUI::Y);
    ExtUI::setAxisMaxFeedrate_mm_s(static_cast<float>(z), ExtUI::Z);
    ExtUI::setAxisMaxFeedrate_mm_s(static_cast<float>(e), ExtUI::E0);
    ExtUI::setMinFeedrate_mm_s(static_cast<float>(min));
    ExtUI::setMinTravelFeedrate_mm_s(static_cast<float>(travel));

    Parent::do_save_command();
}

}
