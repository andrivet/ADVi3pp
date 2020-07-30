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

#pragma once

#include <stdint.h>
#include "../../core/millis_t.h"
#include "flash_char.h"
#include "string.h"

namespace ADVi3pp {

const size_t message_length = 48; //!< Size of messages to be displayed on the LCD Panel
const size_t filename_length = 44; //!< Size of the progress name (i.e. filename) to be displayed on the LCD Panel
const size_t progress_text_length = 48; //!< Size of the progress message (filename and percent) to be displayed on the LCD Panel
const size_t progress_percent_length = 8; //!< Size of the progress percent text to be displayed on the LCD Panel
const size_t tc_length = 8; //!< Size of the time to complete message to be displayed on the LCD Panel
const size_t et_length = 8; //!< Size of the elaplsed time message to be displayed on the LCD Panel

struct Status
{
    void reset();
    void set(const FlashChar* message);
    void set(const char* message);
    void set(const FlashChar* fmt, va_list& args);
    bool has() const;

    void send();
    void set_filename(const char* name);
    void reset_progress();

private:
    void send_progress();
    void send_times();
    void send_status(ADVString<message_length>& message);

private:
    bool has_status_ = false;
    ADVString<filename_length> filename_;
    int percent_ = -1;
    millis_t next_update_times_time_ = 0;
};

extern Status status;

}
