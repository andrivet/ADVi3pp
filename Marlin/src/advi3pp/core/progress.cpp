/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2025 Sebastien Andrivet [https://github.com/andrivet/]
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

#include "../../inc/MarlinConfig.h"
#include "../../lcd/extui/ui_api.h"
#include "dgus.h"
#include "task.h"
#include "progress.h"

namespace ADVi3pp::Progress {

  inline namespace internals {
    constexpr size_t TIME_LENGTH = 8; //!< Size of ET or TC to be displayed on the LCD Panel
    constexpr uint8_t WAIT_DELAY = 1; //!< Delay in minutes before displaying TC
    enum struct STATE: uint8_t { IDLE, HEATING, WAITING, DISPLAYING }; //< Small state machine
    
    uint8_t percent_ = 0;
    uint8_t progress_at_wait_ = 0;
    uint32_t elapsed_at_wait_ = 0;
    STATE state_ = STATE::IDLE;
    Elapse elapse_{2000}; // 2 seconds

    void send_progress(bool force);
    void send_elapsed_time();
    void send_time_to_complete();
  }

  void set(uint8_t percent) {
    WriteRamRequest{Variable::ProgressPercent}.write_word(percent);
    // The progress bar is split into two parts because of a limitation of the DWIN panel
    // so compute the progress of each part.
    WriteRamRequest{Variable::ProgressLow}.write_words(
      percent <= 50 ? percent / 5 : 10,
      percent > 50 ? (percent - 50) / 5 : 0
    );
  }

  void reset() {
    WriteRamRequest{Variable::ProgressPercent}.write_word(0);
    WriteRamRequest{Variable::ProgressLow}.write_words(0, 0);
    start();
  }

  void start() {
    state_ = STATE::HEATING;
    WriteRamRequest{Variable::ET}.write_text(F("00:00"), TIME_LENGTH);
    WriteRamRequest{Variable::TC}.write_text(F("--:--"), TIME_LENGTH);
  }

  void finish() {
    state_ = STATE::IDLE;
  }

  void set_animation(bool animate) {
    WriteRamRequest{Variable::WaitAnimation}.write_word(animate);
  }

  void send() {
    send_progress(false);
    if(elapse_.is_elapsed()) {
      send_elapsed_time();
      send_time_to_complete();
    }
  }

  inline namespace internals {

    void send_progress(bool force) {
      auto done = ExtUI::getProgress_percent();
      if(done == percent_ && !force) return;
      percent_ = done;
      Progress::set(percent_);
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

    void send_elapsed_time() {
      ADVString<TIME_LENGTH> et; // elapsed time
      set_duration(et, ExtUI::getProgress_seconds_elapsed());
      WriteRamRequest{Variable::ET}.write_text(et.get(), TIME_LENGTH);
    }

    void send_time_to_complete() {
      ADVString<TIME_LENGTH> tc{F("--:--")}; // time to complete

      auto elapsed = ExtUI::getProgress_seconds_elapsed();
      auto progress = ExtUI::getProgress_percent();

      if(state_ == STATE::HEATING) {
        auto actual = ExtUI::getActualTemp_celsius(ExtUI::H0);
        auto target =  ExtUI::getTargetTemp_celsius(ExtUI::H0);
        if(actual < target) return;
        state_ = STATE::WAITING;
        elapsed_at_wait_ = elapsed;
        progress_at_wait_ = progress;
      }
      else if(state_ == STATE::WAITING) {
        if(elapsed - elapsed_at_wait_ < WAIT_DELAY * 60) return;
        state_ = STATE::DISPLAYING;
      }
      else { // state_ == STATE::DISPLAYING
        auto adjusted_progress = progress <= progress_at_wait_ ? 0 : progress - progress_at_wait_;
        auto adjusted_elapsed = elapsed <= elapsed_at_wait_ ? 0 : elapsed - elapsed_at_wait_;
        auto tc_sec = adjusted_progress <= 0 ? 0 : (adjusted_elapsed * (100 - adjusted_progress) / adjusted_progress);
        set_duration(tc, tc_sec);
      }

      WriteRamRequest{Variable::TC}.write_text(tc.get(), TIME_LENGTH);
    }
  }
}

