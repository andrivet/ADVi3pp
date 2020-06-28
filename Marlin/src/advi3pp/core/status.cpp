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

#include "../parameters.h"
#include "../../lcd/extui/ui_api.h"
#include "status.h"
#include "dgus.h"
#include "enums.h"

namespace ADVi3pp {

Status status;


void Status::reset()
{
    message_.reset();
    centered_.reset();
    has_status_ = false;
}

bool Status::has() const
{
    return has_status_;
}

void Status::set(const FlashChar* message)
{
    message_.set(message).align(Alignment::Left);
    centered_.set(message).align(Alignment::Center);
    has_status_ = true;
}

void Status::set(const char* message)
{
    message_.set(message).align(Alignment::Left);
    centered_.set(message).align(Alignment::Center);
    has_status_ = true;
}

void Status::set(const FlashChar* fmt, va_list& args)
{
    message_.set(fmt, args).align(Alignment::Left);
    centered_.set(fmt, args).align(Alignment::Center);
    has_status_ = true;
}

//! Compute the current progress message (name and percentage)
void Status::compute_progress()
{
    auto done = ExtUI::getProgress_percent();
    if(done == percent_)
        return;

    progress_ = progress_name_;
    if(progress_.length() > 0)
        progress_  << " " << done << "%";
    progress_.align(Alignment::Left);
    percent_ = done;
}

//! Set the name for the progress message. Usually, it is the name of the file printed.
void Status::set_progress_name(const char* name)
{
    progress_name_ = name;
    progress_.reset().align(Alignment::Left);
    percent_ = -1;
    compute_progress();
}

//! Clear the progress message
void Status::reset_progress()
{
    progress_name_.reset();
    progress_.reset().align(Alignment::Left);
    percent_ = -1;
}

void Status::send()
{
    // If one of the messages has changed, send them to the LCD panel
    if(message_.has_changed(true) || centered_.has_changed(true) || progress_.has_changed(true))
    {
        WriteRamDataRequest frame{Variable::Message};
        frame << message_ << centered_ << progress_;
        frame.send(false);
    }
}

}
