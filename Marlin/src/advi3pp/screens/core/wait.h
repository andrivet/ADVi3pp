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

#include "../../core/task.h"
#include "../../core/string.h"
#include "../../core/dgus.h"
#include "../core/screen.h"

namespace ADVi3pp {

//! Display wait page and messages
struct Wait: Screen<Wait>
{
    void show(const FlashChar* message, ShowOptions options = ShowOptions::SaveBack);
    void show(const FlashChar* message, const WaitCallback& back, ShowOptions options = ShowOptions::SaveBack);
    void show(const FlashChar* message, const WaitCallback& back, const WaitCallback& cont, ShowOptions options = ShowOptions::SaveBack);
    void show_continue(const FlashChar* message, const WaitCallback& cont, ShowOptions options = ShowOptions::SaveBack);
    void show_continue(const FlashChar* message, ShowOptions options = ShowOptions::SaveBack);
    template<size_t L> void show_continue(const ADVString<L>& message, ShowOptions options = ShowOptions::SaveBack);
    void show_back(const FlashChar* message, ShowOptions options = ShowOptions::SaveBack);
    void set_message(const FlashChar* message);
    template<size_t L> void set_message(const ADVString<L>& message);

private:
    Page do_prepare_page();
    void do_save_command();
    void do_back_command();
    bool on_continue();
    bool on_back();

    WaitCallback back_;
    WaitCallback continue_;

    friend Parent;
};

extern Wait wait;

// --------------------------------------------------------------------

template<size_t L> void Wait::set_message(const ADVString<L>& message)
{
    WriteRamDataRequest frame{Variable::LongText0};
    frame << message;
    frame.send();
}

// --------------------------------------------------------------------
//! Show a simple wait page without a message
//! @param message  The message to display
//! @param options  Options when displaying the page (i.e. save the current page or not)
template<size_t L>
void Wait::show_continue(const ADVString<L>& message, ShowOptions options)
{
    set_message(message);
    back_ = nullptr;
    continue_ = WaitCallback{this, &Wait::on_continue};
    //advi3pp.buzz(); TODO
    pages.show_page(Page::WaitContinue, options);
}

}

