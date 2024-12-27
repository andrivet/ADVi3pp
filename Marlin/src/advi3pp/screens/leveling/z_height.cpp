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
#include "../../../lcd/extui/ui_api.h"
#include "../../core/core.h"
#include "../../core/status.h"
#include "../../core/pool.h"
#include "../common/wait.h"
#include "../common/set_temperature.h"
#include "z_height.h"

#if ENABLED(ADVi3PP_PROBE)

namespace ADVi3pp::SensorZHeight {

  inline namespace internals {
    constexpr xyz_feedrate_t homing_feedrate_mm_m = HOMING_FEEDRATE_MM_M;
    constexpr float FEEDRATE_XY = MMM_TO_MMS(homing_feedrate_mm_m.x);
    constexpr float FEEDRATE_Z = MMM_TO_MMS(homing_feedrate_mm_m.z);
    constexpr uint16_t KEY_CODE_PLUS = 1;
    constexpr uint16_t KEY_CODE_MINUS = 2;
    constexpr uint16_t KEY_CODE_MULTIPLIER_1 = 3;
    constexpr uint16_t KEY_CODE_MULTIPLIER_2 = 4;
    constexpr uint16_t KEY_CODE_MULTIPLIER_3 = 5;
    constexpr Variable VAR_MULTIPLIER = Variable::Value0;

    enum class Multiplier: uint8_t { M1, M2, M3 };

    struct Data {
      float old_offset_ = 0;
      Multiplier multiplier_ = Multiplier::M1;
    };

    inline Data& pool() { return Pool::get<Data>(Page::ZHeightTuning); }

    void show_command();
    void back_command();
    void save_command();
    void minus_command();
    void plus_command();
    void multiplier1_command();
    void multiplier2_command();
    void multiplier3_command();

    float get_multiplier_value();
    void adjust_height(float offset);
    void send_data();
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: back_command(); break;
      case KEY_CODE_SAVE: save_command(); break;
      case KEY_CODE_PLUS: minus_command(); break;
      case KEY_CODE_MINUS: plus_command(); break;
      case KEY_CODE_MULTIPLIER_1: multiplier1_command(); break;
      case KEY_CODE_MULTIPLIER_2: multiplier2_command(); break;
      case KEY_CODE_MULTIPLIER_3: multiplier3_command(); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    // Use a function to save RAM
    float get_z_height_multiplier(size_t index) {
      switch(index) {
        case 0: return 0.02;
        case 1: return 0.10;
        case 2: return 1.0;
        default: Log::error() << F("Invalid z height multiplier index") << Log::endl(); break;
      }
      return 1.0;
    }

    void show_command() {
      if(!Core::check_not_busy()) return;

      SetTemperature::extruder([] (CALLBACK_SOURCE src) -> void {
        if(src == CALLBACK_SOURCE::BACK) {
          Pages::back(Pages::BACK_OPTIONS::NONE);
          return;
        }

        Pool::reset<Data>(Page::ZHeightTuning);
        Status::reset();

        pool().old_offset_ = ExtUI::getZOffset_mm();
        ExtUI::setZOffset_mm(0); // Before homing otherwise, Marlin is lost
        ExtUI::setAbsoluteZAxisPosition_mm(ExtUI::getAxisPosition_mm(ExtUI::Z) + pool().old_offset_);

        Wait::homing([] () -> void {
          float positions[2] = {X_CENTER, Y_CENTER};
          ExtUI::axis_t axis[2] = { ExtUI::X, ExtUI::Y };
          ExtUI::setMultipleAxisPosition_mm(2, positions, axis, FEEDRATE_XY);
          ExtUI::setAxisPosition_mm(0, ExtUI::Z, FEEDRATE_Z);
          ExtUI::setSoftEndstopState(false);

          send_data();
          Pages::show(Page::ZHeightTuning);
        });
      }, SetTemperature::OPTIONS::INIT_ZERO);
    }

    //! Handles the Save (Continue) command
    void save_command() {
      ExtUI::setTargetTemp_celsius(0, ExtUI::H0);
      Wait::wait();
      // Current Z position becomes Z offset
      ExtUI::setSoftEndstopState(true);
      ExtUI::setZOffset_mm(ExtUI::getAxisPosition_mm(ExtUI::Z));
      ExtUI::setAbsoluteZAxisPosition_mm(0);
      ExtUI::setAxisPosition_mm(Z_AFTER_HOMING, ExtUI::Z, FEEDRATE_Z);
      Pages::save(Pages::SAVE_OPTIONS::SETTINGS, Pages::BACK_OPTIONS::FINISH_MOVE);
    }

    void back_command() {
      ExtUI::setTargetTemp_celsius(0, ExtUI::H0);
      ExtUI::setSoftEndstopState(true);
      ExtUI::setZOffset_mm(pool().old_offset_);
      ExtUI::setAbsoluteZAxisPosition_mm(ExtUI::getAxisPosition_mm(ExtUI::Z) - pool().old_offset_);
      ExtUI::setAxisPosition_mm(Z_AFTER_HOMING, ExtUI::Z, FEEDRATE_Z);
      Pages::back(Pages::BACK_OPTIONS::FINISH_MOVE);
    }

    //! Change the position of the nozzle (-Z).
    void minus_command() {
      adjust_height(-get_multiplier_value());
    }

    //! Change the position of the nozzle (+Z).
    void plus_command() {
      adjust_height(+get_multiplier_value());
    }

    //! Change the multiplier.
    void multiplier1_command() {
      pool().multiplier_ = Multiplier::M1;
      send_data();
    }

    //! Change the multiplier.
    void multiplier2_command() {
      pool().multiplier_ = Multiplier::M2;
      send_data();
    }

    //! Change the multiplier.
    void multiplier3_command() {
      pool().multiplier_ = Multiplier::M3;
      send_data();
    }

    //! Get the current multiplier value on the LCD panel.
    float get_multiplier_value() {
      if(pool().multiplier_ < Multiplier::M1 || pool().multiplier_ > Multiplier::M3) {
        Log::error() << F("Invalid multiplier value: ") << static_cast<uint16_t >(pool().multiplier_) << Log::endl();
        return get_z_height_multiplier(0);
      }

      return get_z_height_multiplier(static_cast<uint16_t>(pool().multiplier_));
    }

    //! Adjust the Z height.
    //! @param offset Offset for the adjustment.
    void adjust_height(float offset) {
      ExtUI::setAxisPosition_mm(ExtUI::getAxisPosition_mm(ExtUI::Z) + offset, ExtUI::Z, FEEDRATE_Z);
      send_data();
    }

    //! Send the current data (i.e. multiplier) to the LCD panel.
    void send_data() {
      WriteRamRequest{VAR_MULTIPLIER}.write_words(pool().multiplier_, ExtUI::getAxisPosition_mm(ExtUI::Z) * 100);
    }

  }
}

#endif
