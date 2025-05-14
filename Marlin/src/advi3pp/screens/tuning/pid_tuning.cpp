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

#include "../../../inc/MarlinConfig.h"
#include "../../core/dgus.h"
#include "../../core/core.h"
#include "../../core/status.h"
#include "../../core/progress.h"
#include "../../core/pool.h"
#include "../print/temperatures.h"
#include "pid_tuning.h"

namespace ADVi3pp::PidTuning {
  enum class State: uint8_t {
    None,
    Processing,
    FromLCDMenu = 0x80
  };
}
ENABLE_BITMASK_OPERATOR(ADVi3pp::PidTuning::State);

namespace ADVi3pp::PidTuning {
  inline namespace internals {

    constexpr uint16_t KEY_CODE_STEP2 = 1;
    constexpr uint16_t KEY_CODE_BED = 2;
    constexpr uint16_t KEY_CODE_EXTRUDER = 3;
    constexpr Variable VAR_TEMP = Variable::Value0;
    constexpr Variable VAR_HEATER = Variable::Value1;

    // Can't be stored in the Pool because of is_running that is called any time.
    State state_ = State::None;
    bool bed_ = false;

    void show_command();
    void step2_command();
    void heater_command(bool bed);

    void set_message(ExtUI::pidresult_t result);
  }

  RUNNING is_running() {
    return !test_one_bit(state_, State::Processing) ? RUNNING::NO : bed_ ? RUNNING::BED : RUNNING::EXTRUDER;
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: Pages::back(Pages::BACK_OPTIONS::NONE); break;
      case KEY_CODE_STEP2: step2_command(); break;
      case KEY_CODE_EXTRUDER: heater_command(false); break;
      case KEY_CODE_BED: heater_command(true); break;
      default: return false;
    }
    return true;
  }

  void on_start(bool bed) {
    Status::set(GET_TEXT_F(ADVI3PP_MSG_PID_TUNING_START), Status::STATUS_OPTIONS::RESET);
    state_ |= State::Processing;
    bed_ = bed;
  }

  void on_progress(int cycle, int nb) {
    Log::info() << F("ExtUI::on_progress") << cycle << nb << Log::endl();
    Progress::set(cycle * 100 / nb);
  }

  //! PID automatic tuning is finished.
  void on_finished(ExtUI::pidresult_t result) {
    if((state_ & ~State::FromLCDMenu) != State::Processing) return;
    state_ = State::None;
    set_message(result);
    ExtUI::setTargetFan_percent(0, ExtUI::FAN0);
    Progress::set_animation(false);
    Progress::reset();
    if(result != ExtUI::PID_DONE) return;
    Core::display(Page::PidSettings, Core::DISPLAY_OPTIONS::NONE, bed_);
  }

  inline namespace internals {

    void show_command() {
      if(!Core::check_not_busy()) return;
      Status::reset();
      Pages::save_forward_page();
      WriteRamRequest{VAR_TEMP}.write_words(static_cast<uint16_t>(ExtUI::getDefaultTemp_celsius(ExtUI::H0)), 0);
      Pages::show(Page::PidTuning);
    }

    adv::tuple<bool, celsius_t> save_temperature() {
      ReadRam frame{VAR_TEMP};
      if(!frame.send_receive(2)) return adv::make_tuple(false, 0);
      auto temperature = static_cast<celsius_t>(frame.read_uint());
      bool bed = frame.read_bool();
      ExtUI::setDefaultTemp_celsius(temperature, bed ? ExtUI::BED : ExtUI::H0);
      return adv::make_tuple(bed, temperature);
    }

    void heater_command(bool bed) {
      save_temperature();
      auto temperature = static_cast<uint16_t>(ExtUI::getDefaultTemp_celsius(bed ? ExtUI::BED : ExtUI::H0));
      WriteRamRequest{VAR_TEMP}.write_words(temperature, bed);
    }

    //! Show step #2 of PID tuning
    void step2_command() {
      state_ |= State::FromLCDMenu;
      if(!bed_)
        ExtUI::setTargetFan_percent(100, ExtUI::FAN0); // Turn on fan (only for extruder PID)

      auto values= save_temperature();
      auto bed = adv::get<0>(values);
      auto temperature = adv::get<1>(values);

      Progress::reset();
      Progress::set_animation(true);

      Temperatures::display([] () -> void {
        Log::info() << F("Cancel PID tuning") << Log::endl();
        Status::set(GET_TEXT_F(ADVI3PP_MSG_PID_TUNING_CANCEL), Status::STATUS_OPTIONS::RESET);
        ExtUI::cancelWaitForHeatup();
        ExtUI::setTargetFan_percent(0, ExtUI::FAN0);
        state_ = State::None;
      });
      // startPIDTune will enter a loop and thus will call idle from idle
      if(bed)
        ExtUI::startBedPIDTune(temperature);
      else
        ExtUI::startPIDTune(temperature, ExtUI::E0);
    }

    void set_message(ExtUI::pidresult_t result) {
      switch(result) {
        case ExtUI::PID_BAD_HEATER_ID:        break; // Never happens
        case ExtUI::PID_TEMP_TOO_HIGH:        Status::set(GET_TEXT_F(ADVI3PP_MSG_TEMP_TOO_HIGH), Status::STATUS_OPTIONS::RESET); break;
        case ExtUI::PID_TUNING_TIMEOUT:       Status::set(GET_TEXT_F(ADVI3PP_MSG_TIMEOUT), Status::STATUS_OPTIONS::RESET); break;
        case ExtUI::PID_DONE:                 Status::set(GET_TEXT_F(ADVI3PP_MSG_PID_TUNING_SUCCESS), Status::STATUS_OPTIONS::RESET); break;
        default: Log::error() << F("Unknown result_t ") << static_cast<uint16_t>(result) << Log::endl(); break;
      }
    }

  }
}
