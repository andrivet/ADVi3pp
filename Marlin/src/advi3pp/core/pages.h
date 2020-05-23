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

#pragma once

#include "stack.h"
#include "enums.h"

namespace ADVi3pp {

//! Options when pushing a page onto the stack
enum class ShowOptions: uint8_t
{
    None     = 0x00, //!< No option
    SaveBack = 0x01  //!< Save the current page onto the stack of back pages
};
ENABLE_BITMASK_OPERATOR(ShowOptions);


//! Display a page on top of the others; display back and forward pages
struct Pages
{
    void show_page(Page page, ShowOptions options = ShowOptions::SaveBack);
    Page get_current_page();
    void save_forward_page();
    void show_back_page();
    void show_forward_page();

private:
    Stack<Page, 8> back_pages_{};
    Page forward_page_ = Page::None;
    Page current_page_ = Page::Main;
};

extern Pages pages;

}
