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
#include "logging.h"
#include "status.h"
#include "dgus.h"
#include "enums.h"

namespace ADVi3pp {

Status status;


void Status::reset()
{
    has_status_ = false;
    set("");
}

bool Status::has() const
{
    return has_status_;
}

void Status::set(const FlashChar* message)
{
    ADVString<message_length> text{message};
    Log::log() << F("Status::set(FlashChar) ") << text.get() << Log::endl();
    send_status(text);
    has_status_ = true;
}

void Status::set(const char* message)
{
    ADVString<message_length> text{message};
    Log::log() << F("Status::set(const char*) ") << text.get() << Log::endl();
    send_status(text);
    has_status_ = true;
}

void Status::set(const FlashChar* fmt, va_list& args)
{
    ADVString<message_length> text{};
    text.set(fmt, args);
    Log::log() << F("Status::set(fmt)" ) << text.get() << Log::endl();
    send_status(text);
    has_status_ = true;
}

void Status::send()
{
    send_progress();
    send_times();
}

void Status::send_progress()
{
    auto done = ExtUI::getProgress_percent();
    if(done == percent_)
        return;
	percent_ = done;

    ADVString<progress_text_length> progress{filename_};
    if(progress.length() > 0)
        progress  << " " << done << "%";

    ADVString<progress_percent_length> progress_percent{};
    progress_percent << done << "%";

    Log::log() << F("Progress: " ) << progress.get() << Log::endl();
    Log::log() << F("Percent: " ) << progress_percent.get() << Log::endl();

    WriteRamDataRequest frame{Variable::ProgressText};
    frame << progress << progress_percent;
    frame.send();
}

void Status::send_times()
{
	if(!ExtUI::isPrinting())
		return;
    auto current_time = millis();
    if(!ELAPSED(current_time, next_update_times_time_))
        return;
    next_update_times_time_ = current_time + 2000; // every 2 seconds

    ADVString<tc_length> tc; // time to complete
    ADVString<et_length> et; // elapsed time

    auto durationSec = ExtUI::getProgress_seconds_elapsed();
	auto progress = ExtUI::getProgress_percent();

    et.set(duration_t{durationSec}, Duration::digital);

    auto tcSec = progress <= 0 ? 0 : (durationSec * (100 - progress) / progress);
    if (progress < 5)
        tc.set(F("00:00"));
    else
        tc.set(duration_t{tcSec}, Duration::digital);

    WriteRamDataRequest frame{Variable::ET};
    frame << et << tc;
    frame.send(false);
}

//! Set the name for the progress message. Usually, it is the name of the file printed.
void Status::set_filename(const char* name)
{
    Log::log() << F("Set Progress Filename: " ) << name << Log::endl();

    filename_ = name;
    percent_ = -1;
    send_progress();
}

//! Clear the progress message
void Status::reset_progress()
{
    Log::log() << F("Reset Progress" ) << Log::endl();

    filename_.reset();
    percent_ = -1;
    send_progress();
}

void Status::send_status(ADVString<message_length>& message)
{
    WriteRamDataRequest frame{Variable::Message};
    frame << message;
    frame.center(message);
    frame.send(false);
}

}
