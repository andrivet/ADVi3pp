/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin)
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

#include "../parameters.h"
#include "dgus.h"
#include "pages.h"

namespace ADVi3pp {

Pages pages;

Log& operator<<(Log& log, Page page)
{
    log << static_cast<uint16_t>(page);
    return log;
}

//! Show the given page on the LCD screen
//! @param [in] page The page to be displayed on the LCD screen
void Pages::show(Page page)
{
    auto current = get_current_page();
    if(!test_one_bit(current, Page::Temporary) && current != Page::Main)
    {
        Log::log() << F("Save back page ") << current << Log::endl();
        back_pages_.push(current);
    }

   show_(page);
}

void Pages::show_(Page page)
{
    Log::log() << F("Show page ") << page << Log::endl();

    WriteRegisterDataRequest frame{Register::PictureID};
    frame << Uint16{static_cast<uint16_t>(get_cleared_bits(page, Page::Temporary))};
    frame.send(true);

    current_page_ = page;
}

//! Retrieve the current page on the LCD screen
Page Pages::get_current_page()
{
    // Boot page switches automatically (animation) to the Main page
	if(current_page_ == Page::None || current_page_ == Page::Boot)
        current_page_ = Page::Main;
    return current_page_;
}

//! Set page to display after the completion of an operation.
void Pages::save_forward_page()
{
    auto current = get_current_page();
    Log::log() << F("Save forward page ") << current << Log::endl();
    forward_page_ = current;
}

//! Show the "Back" page on the LCD display.
void Pages::show_back_page()
{
    forward_page_ = Page::None;

    if(back_pages_.is_empty())
    {
        Log::log() << F("No back page, show Main") << Log::endl();
        show_(Page::Main);
        return;
    }

    auto back = back_pages_.pop();
    Log::log() << F("Pop back page ") << back << Log::endl();
    show_(back);
}

//! Show the "Next" page on the LCD display.
void Pages::show_forward_page()
{
    if(forward_page_ == Page::None)
    {
        show_back_page();
        return;
    }

    while(!back_pages_.is_empty())
    {
        Page back_page = back_pages_.pop();
        Log::log() << F("Pop back page ") << back_page << Log::endl();
        if(back_page == forward_page_)
        {
            show_(forward_page_);
            forward_page_ = Page::None;
            return;
        }
    }

    Log::error() << F("Back pages do not contain page ") << forward_page_ << Log::endl();
    forward_page_ = Page::Main;
    show_(forward_page_);
}

void Pages::reset()
{
    back_pages_.empty();
}

}
