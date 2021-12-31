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

#include "../../parameters.h"
#include "../../core/core.h"
#include "../../core/dgus.h"
#include "../../core/dates.h"
#include "../../versions.h"
#include "versions.h"

namespace ADVi3pp {

Versions versions;


//! Get the current DGUS firmware version.
//! @return     The version as a string.
template<size_t L>
ADVString<L>& get_lcd_firmware_version(ADVString<L>& lcd_version)
{
    ReadRegister response{Register::Version};
    if(!response.send_receive(1))
    {
        Log::error() << F("Receiving Frame (Version)") << Log::endl();
        return lcd_version;
    }

    uint8_t version = response.read_byte();

    Log::log() << F("LCD Firmware raw version") << version << Log::endl();

	lcd_version.reset();
    lcd_version << (version / 0x10) << '.' << (version % 0x10);
    return lcd_version;
}


//! Send the different versions to the LCD screen.
void Versions::send_versions() const
{
	// Minimize memory usage (SRAM)
	
	ADVString<16> text;
    core.convert_version(text, advi3_pp_version);
	WriteRamRequest{Variable::ADVi3ppVersion}.write_text(text);

    text.reset();
    text << (YEAR__ - 2000)
         << (MONTH__ < 10 ? "0" : "") << MONTH__
         << (DAY__   < 10 ? "0" : "") << DAY__
         << (HOUR__  < 10 ? "0" : "") << HOUR__
         << (MIN__   < 10 ? "0" : "") << MIN__
         << (SEC__   < 10 ? "0" : "") << SEC__;
    WriteRamRequest{Variable::ADVi3ppBuild}.write_text(text);
	
    get_lcd_firmware_version(text);
    WriteRamRequest{Variable::ADVi3ppDGUSVersion}.write_text(text);
		
    text.set(SHORT_BUILD_VERSION);
    WriteRamRequest{Variable::ADVi3ppMarlinVersion}.write_text(text);
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page Versions::do_prepare_page()
{
    send_versions();
    return Page::Versions;
}

}
