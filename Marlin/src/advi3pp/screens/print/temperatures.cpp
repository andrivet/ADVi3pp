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

#include "temperatures.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "HidingNonVirtualFunction"

namespace ADVi3pp {

Temperatures temperatures;


//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page Temperatures::do_prepare_page()
{
    return Page::Temperature;
}

//! Show the temperature page and record and action to be executed when the back button is pressed
//! @param back Action to be executed when the back button is pressed
void Temperatures::show(const WaitCallback& back)
{
    back_ = back;
    Parent::show(ShowOptions::SaveBack);
}

//! Show the temperature page
//! @param options  Options when displaying the page (i.e. save the current page or not)
void Temperatures::show(ShowOptions options)
{
    back_ = nullptr;
    Parent::show(options);
}

//! Execute the Back command
void Temperatures::do_back_command()
{
    if(back_)
    {
        back_();
        back_ = nullptr;
    }

    Parent::do_back_command();
}

}

#pragma clang diagnostic pop