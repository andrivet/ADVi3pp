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
#include "acceleration_settings.h"
#include "../../core/dgus.h"

namespace ADVi3pp {

AccelerationSettings accelerations_settings;

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page AccelerationSettings::do_prepare_page()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(static_cast<uint16_t>(ExtUI::getAxisMaxAcceleration_mm_s2(ExtUI::X)))
          << Uint16(static_cast<uint16_t>(ExtUI::getAxisMaxAcceleration_mm_s2(ExtUI::Y)))
          << Uint16(static_cast<uint16_t>(ExtUI::getAxisMaxAcceleration_mm_s2(ExtUI::Z)))
          << Uint16(static_cast<uint16_t>(ExtUI::getAxisMaxAcceleration_mm_s2(ExtUI::E0)))
          << Uint16(static_cast<uint16_t>(ExtUI::getPrintingAcceleration_mm_s2()))
          << Uint16(static_cast<uint16_t>(ExtUI::getRetractAcceleration_mm_s2()))
          << Uint16(static_cast<uint16_t>(ExtUI::getTravelAcceleration_mm_s2()));
    frame.send();

    return Page::AccelerationSettings;
}

//! Save the Acceleration settings
void AccelerationSettings::do_save_command()
{
    ReadRamData response{Variable::Value0, 7};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Acceleration Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z, e, print, retract, travel;
    response >> x >> y >> z >> e >> print >> retract >> travel;

    ExtUI::setAxisMaxAcceleration_mm_s2(static_cast<uint32_t>(x.word), ExtUI::X);
    ExtUI::setAxisMaxAcceleration_mm_s2(static_cast<uint32_t>(y.word), ExtUI::Y);
    ExtUI::setAxisMaxAcceleration_mm_s2(static_cast<uint32_t>(z.word), ExtUI::Z);
    ExtUI::setAxisMaxAcceleration_mm_s2(static_cast<uint32_t>(e.word), ExtUI::E0);
    ExtUI::setPrintingAcceleration_mm_s2(static_cast<float>(print.word));
    ExtUI::setRetractAcceleration_mm_s2(static_cast<float>(retract.word));
    ExtUI::setTravelAcceleration_mm_s2(static_cast<float>(travel.word));

    Parent::do_save_command();
}

}
