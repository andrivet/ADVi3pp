/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
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

#include "../parameters.h"
#include "../../lcd/extui/ui_api.h"
#include "logging.h"
#include "status.h"
#include "pages.h"
#include "dgus.h"
#include "enums.h"

namespace ADVi3pp {

Status status;


void Status::reset() {
  has_status_ = false;
  set("");
}

void Status::reset_and_clear() {
  pages.clear_temporaries();
  reset();
}

bool Status::has() const {
  return has_status_;
}

void Status::set(const FlashChar* message) {
  ADVString<message_length> text{message};
  send_status(text);
  has_status_ = true;
}

void Status::set(const char* message) {
  ADVString<message_length> text{message};
  send_status(text);
  has_status_ = true;
}

void Status::format(const FlashChar* fmt, va_list& args) {
  ADVString<message_length> text{};
  text.set(fmt, args);
  send_status(text);
  has_status_ = true;
}

void Status::format(const FlashChar* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  format(fmt, args);
  va_end(args);
}

void Status::send() {
  send_progress();
  send_times();
}

void Status::send_progress() {
  auto done = ExtUI::getProgress_percent();
  if(done == percent_)
    return;
  percent_ = done;

  ADVString<progress_text_length> progress{filename_};
  if(progress.length() > 0)
    progress  << " " << done << "%";

  ADVString<progress_percent_length> progress_percent{};
  progress_percent << done << "%";

  WriteRamRequest{Variable::ProgressText}.write_text(progress);
  WriteRamRequest{Variable::ProgressPercent}.write_text(progress_percent);
}

template<size_t N>
void set_duration(ADVString<N>& str, uint32_t seconds) {
  auto h = uint16_t(seconds / (60 * 60));
  auto m = uint16_t((seconds / 60) % 60UL);

  if(h < 100)
    str.format(F("%02hu:%02hu"), h, m);
  else
    str.format(F("%hu:%02hu"), h, m);
}

void Status::send_times() {
	if(!ExtUI::isPrinting()) return;
  auto current_time = millis();
  if(!ELAPSED(current_time, next_update_times_time_))
    return;
  next_update_times_time_ = current_time + 2000; // every 2 seconds

  ADVString<tc_length> tc; // time to complete
  ADVString<et_length> et; // elapsed time

  auto durationSec = ExtUI::getProgress_seconds_elapsed();
  auto progress = ExtUI::getProgress_percent();

  set_duration(et, durationSec);

  auto tcSec = progress <= 0 ? 0 : (durationSec * (100 - progress) / progress);
  if (progress < 5)
    tc.set(F("--:--"));
  else
      set_duration(tc, tcSec);

  WriteRamRequest{Variable::ET}.write_text(et);
  WriteRamRequest{Variable::TC}.write_text(tc);
}

//! Set the name for the progress message. Usually, it is the name of the file printed.
void Status::set_filename(const char* name) {
  filename_ = name;
  percent_ = -1;
  send_progress();
}

//! Clear the progress message
void Status::reset_progress() {
  filename_.reset();
  percent_ = -1;
  send_progress();
}

void Status::send_status(ADVString<message_length>& message) {
  WriteRamRequest{Variable::Message}.write_text(message);
  WriteRamRequest{Variable::CenteredMessage}.write_centered_text(message);
}

}
