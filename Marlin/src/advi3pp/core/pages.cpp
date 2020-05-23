/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin)
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

#include "dgus.h"
#include "pages.h"

namespace ADVi3pp {

Pages pages;


//! Show the given page on the LCD screen
//! @param [in] page The page to be displayed on the LCD screen
void Pages::show_page(Page page, ShowOptions options)
{
    Log::log() << F("Show page ") << static_cast<uint8_t>(page) << Log::endl();

    if(test_one_bit(options, ShowOptions::SaveBack))
    {
        auto current = get_current_page();
        Log::log() << F("Save back page ") << static_cast<uint8_t>(current) << Log::endl();
        back_pages_.push(current);
    }

    WriteRegisterDataRequest frame{Register::PictureID};
    frame << 00_u8 << page;
    frame.send(true);

    current_page_ = page;
}

//! Retrieve the current page on the LCD screen
Page Pages::get_current_page()
{
    // Boot page switches automatically (animation) to the Main page
    return current_page_ == Page::Boot ? Page::Main : current_page_;
}

//! Set page to display after the completion of an operation.
void Pages::save_forward_page()
{
    auto current = get_current_page();
    Log::log() << F("Save forward page ") << static_cast<uint8_t>(current) << Log::endl();
    forward_page_ = current;
}

//! Show the "Back" page on the LCD display.
void Pages::show_back_page()
{
    forward_page_ = Page::None;

    if(back_pages_.is_empty())
    {
        Log::log() << F("No back page, show Main") << Log::endl();
        show_page(Page::Main, ShowOptions::None);
        return;
    }

    auto back = back_pages_.pop();
    Log::log() << F("Pop back page ") << static_cast<uint8_t>(back) << Log::endl();
    show_page(back, ShowOptions::None);
}

//! Show the "Next" page on the LCD display.
void Pages::show_forward_page()
{
    if(forward_page_ == Page::None)
    {
        show_back_page();
        return;
    }

    if(!back_pages_.contains(forward_page_))
    {
        Log::error() << F("Back pages do not contain forward page ") << static_cast<uint8_t>(forward_page_) << Log::endl();
        return;
    }

    while(!back_pages_.is_empty())
    {
        Page back_page = back_pages_.pop();
        Log::log() << F("Pop back page ") << static_cast<uint8_t>(back_page) << Log::endl();
        if(back_page == forward_page_)
        {
            Log::log() << F("Show forward page ") << static_cast<uint8_t>(forward_page_) << Log::endl();
            show_page(forward_page_, ShowOptions::None);
            forward_page_ = Page::None;
            return;
        }
    }
}

}
