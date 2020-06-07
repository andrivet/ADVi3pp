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
#include "eeprom_mismatch.h"

namespace ADVi3pp {

EepromMismatch eeprom_mismatch;

//! Check if the EEPROM values are compatible or not. If not, display a message.
//! @return False if the values are compatible
bool EepromMismatch::check()
{
    if(does_mismatch())
    {
        show(ShowOptions::None);
        return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page EepromMismatch::do_prepare_page()
{
    return Page::EEPROMMismatch;
}

//! Handles the Save (Continue) command
void EepromMismatch::do_save_command()
{
    settings.save();
    pages.show_page(Page::Main);
}

//! Check the EEPROM value mismatch flag.
bool EepromMismatch::does_mismatch() const
{
    return mismatch_;
}

//! Set the EEPROM value mismatch flag.
void EepromMismatch::set_mismatch()
{
    mismatch_ = true;
}

//! Reset the EEPROM value mismatch flag.
void EepromMismatch::reset_mismatch()
{
    mismatch_ = false;
}

}
