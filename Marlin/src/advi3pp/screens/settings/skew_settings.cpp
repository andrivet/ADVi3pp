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
#include "acceleration_settings.h"
#include "../../core/dgus.h"

namespace ADVi3pp {

AccelerationSettings accelerations_settings;

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page AccelerationSettings::do_prepare_page()
{
    WriteRamRequest{Variable::Value0}.write_words(
        ExtUI::getAxisMaxAcceleration_mm_s2(ExtUI::X),
        ExtUI::getAxisMaxAcceleration_mm_s2(ExtUI::Y),
        ExtUI::getAxisMaxAcceleration_mm_s2(ExtUI::Z),
        ExtUI::getAxisMaxAcceleration_mm_s2(ExtUI::E0),
        ExtUI::getPrintingAcceleration_mm_s2(),
        ExtUI::getRetractAcceleration_mm_s2(),
        ExtUI::getTravelAcceleration_mm_s2(),
        ExtUI::getJunctionDeviation_mm() * 1000
    );
    return Page::AccelerationSettings;
}

//! Save the Acceleration settings
void AccelerationSettings::do_save_command()
{
    ReadRam response{Variable::Value0};
    if(!response.send_receive(8))
    {
        Log::error() << F("Receiving Frame (Acceleration Settings)") << Log::endl();
        return;
    }

    uint16_t x = response.read_word();
    uint16_t y = response.read_word();
    uint16_t z = response.read_word();
    uint16_t e = response.read_word();
    uint16_t print = response.read_word();
    uint16_t retract = response.read_word();
    uint16_t travel = response.read_word();
    uint16_t deviation = response.read_word();

    ExtUI::setAxisMaxAcceleration_mm_s2(static_cast<float>(x), ExtUI::X);
    ExtUI::setAxisMaxAcceleration_mm_s2(static_cast<float>(y), ExtUI::Y);
    ExtUI::setAxisMaxAcceleration_mm_s2(static_cast<float>(z), ExtUI::Z);
    ExtUI::setAxisMaxAcceleration_mm_s2(static_cast<float>(e), ExtUI::E0);
    ExtUI::setPrintingAcceleration_mm_s2(static_cast<float>(print));
    ExtUI::setRetractAcceleration_mm_s2(static_cast<float>(retract));
    ExtUI::setTravelAcceleration_mm_s2(static_cast<float>(travel));
    ExtUI::setJunctionDeviation_mm(static_cast<float>(deviation / 1000.0));

    Parent::do_save_command();
}

}
