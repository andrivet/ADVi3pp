/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin)
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

#pragma once

#include "stack.h"
#include "enums.h"
#include "task.h"

namespace ADVi3pp {


//! Display a page on top of the others; display back and forward pages
struct Pages
{
    void show(Page page);
    Page get_current_page();
    void clear_temporaries();
    bool current_page_ensure_no_move();
    void save_forward_page();
    void show_back_page(unsigned nb_back = 1);
    void show_forward_page();
    void reset();
    void save();
    void back();

private:
    static void save_task();
    static void back_task();
    void show_(Page page);
    static bool is_temporary(Page page);
    static bool ensure_no_move(Page page);

private:
    Stack<Page, 8> back_pages_{};
    Page forward_page_ = Page::None;
    Page current_page_ = Page::Main;
};


inline bool Pages::is_temporary(Page page) {
  return test_one_bit(page, Page::Temporary);
}

inline bool Pages::ensure_no_move(Page page) {
  return test_one_bit(page, Page::EnsureNoMove);
}

inline bool Pages::current_page_ensure_no_move() {
  return ensure_no_move(get_current_page());
}


extern Pages pages;

}
