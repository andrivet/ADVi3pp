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
#include "xtwist.h"

namespace ADVi3pp::XTwist {

  inline namespace internals {
    constexpr xyz_feedrate_t homing_feedrate_mm_m = HOMING_FEEDRATE_MM_M;
    constexpr float FEEDRATE_X = MMM_TO_MMS(homing_feedrate_mm_m.x);
    constexpr float FEEDRATE_Y = MMM_TO_MMS(homing_feedrate_mm_m.y);
    constexpr float FEEDRATE_Z = MMM_TO_MMS(homing_feedrate_mm_m.z);
    constexpr int MARGIN = 10;
    constexpr uint16_t KEY_CODE_PLUS = 1;
    constexpr uint16_t KEY_CODE_MINUS = 2;
    constexpr uint16_t KEY_CODE_POINT_L = 3;
    constexpr uint16_t KEY_CODE_POINT_M = 4;
    constexpr uint16_t KEY_CODE_POINT_R = 5;
    constexpr uint16_t KEY_CODE_MULTIPLIER_1 = 6;
    constexpr uint16_t KEY_CODE_MULTIPLIER_2 = 7;
    constexpr uint16_t KEY_CODE_MULTIPLIER_3 = 8;

    enum class Point: uint8_t {L, M, R};
    enum class Multiplier: uint8_t { M1, M2, M3 };

    struct Data {
      Multiplier multiplier_ = Multiplier::M1;
      Point point_ = Point::L;
      bool enabled_ = false;
      adv::array<float, ExtUI::xTwistPoints> z_offsets_{};
    };

    inline Data& pool() { return Pool::get<Data>(Page::XTwist); }

    void show_command();
    void back_command();
    void save_command();
    void minus_command();
    void plus_command();
    void multiplier1_command();
    void multiplier2_command();
    void multiplier3_command();
    void point_L_command();
    void point_M_command();
    void point_R_command();

    void move_x(Point x);
    float get_multiplier_value();
    void adjust_height(float offset);
    void send_data();
    float get_x_mm(Point x);
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: back_command(); break;
      case KEY_CODE_SAVE: save_command(); break;
      case KEY_CODE_PLUS: plus_command(); break;
      case KEY_CODE_MINUS: minus_command(); break;
      case KEY_CODE_POINT_L: point_L_command(); break;
      case KEY_CODE_POINT_M: point_M_command(); break;
      case KEY_CODE_POINT_R: point_R_command(); break;
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
      Log::info() << F("Leveling active") << ExtUI::getLevelingActive() << F("valid") << ExtUI::getLevelingIsValid() << Log::endl();
      if(!ExtUI::getLevelingActive() || !ExtUI::getLevelingIsValid()) {
        Status::set(GET_TEXT_F(ADVI3PP_MSG_LEVELING_REQUIRED), Status::STATUS_OPTIONS::RESET);
        return;
      }

      Pool::reset<Data>(Page::XTwist);
      Status::reset();
      Wait::homing([] () -> void {
        Pages::save_forward_page();
        pool().enabled_ = ExtUI::getXTwistEnabled();
        const float *values = ExtUI::getXTwistZValues();
        adv::copy(values, values + ExtUI::xTwistPoints, pool().z_offsets_.begin());

        ExtUI::setXTwistStartSpacing(MARGIN, X_BED_SIZE / 2.0 - MARGIN);
        for(auto i = 0; i < ExtUI::xTwistPoints; ++i) ExtUI::setXTwistZOffset(i, pool().enabled_ ? pool().z_offsets_[i] : 0);
        ExtUI::setXTwistEnabled(true);

        send_data();
        ExtUI::setSoftEndstopState(false);
        point_M_command();
        Pages::show(Page::XTwist);
      });
    }

    void back_command() {
      // enable enstops, raise head
      ExtUI::setSoftEndstopState(true);
      ExtUI::setAxisPosition_mm(4, ExtUI::Z, FEEDRATE_Z);
      ExtUI::setXTwistEnabled(pool().enabled_);
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

    //! Execute the Back command
    void abort() {
      // enable enstops, raise head
      ExtUI::setSoftEndstopState(true);
    }

    //! Handles the Save (Continue) command
    void save_command() {
      for(auto i = 0; i < ExtUI::xTwistPoints; ++i) ExtUI::setXTwistZOffset(i, pool().z_offsets_[i]);
      ExtUI::setXTwistEnabled(true);

      // enable enstops, raise head
      ExtUI::setSoftEndstopState(true);
      ExtUI::setAxisPosition_mm(4, ExtUI::Z, FEEDRATE_Z);

      Pages::save(Pages::SAVE_OPTIONS::SETTINGS, Pages::BACK_OPTIONS::FINISH_MOVE);
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

    float get_x_mm(Point x) {
      return (x == Point::L ? MARGIN : (x == Point::R ? X_BED_SIZE - MARGIN : (X_BED_SIZE / 2.0f)));
    }

    void move_x(Point x) {
      ExtUI::setAxisPosition_mm(4, ExtUI::Z, FEEDRATE_Z);
      ExtUI::setAxisPosition_mm(get_x_mm(x), ExtUI::X, FEEDRATE_X);
      ExtUI::setAxisPosition_mm(Y_BED_SIZE / 2.0f, ExtUI::Y, FEEDRATE_Y);
      ExtUI::setAxisPosition_mm(pool().z_offsets_[static_cast<size_t>(x)], ExtUI::Z, FEEDRATE_Z);

      pool().point_ = x;
      send_data();
    }

    void point_M_command() {
      move_x(Point::M);
    }

    void point_L_command() {
      move_x(Point::L);
    }

    void point_R_command() {
      move_x(Point::R);
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
    void adjust_height(float offset_value) {
      ExtUI::setAxisPosition_mm(ExtUI::getAxisPosition_mm(ExtUI::Z) + offset_value, ExtUI::Z, FEEDRATE_Z);
      pool().z_offsets_[static_cast<size_t>(pool().point_)] = ExtUI::getAxisPosition_mm(ExtUI::Z);
      send_data();
    }

    //! Send the current data (i.e. multiplier) to the LCD panel.
    void send_data() {
      WriteRamRequest{Variable::Value0}.write_words(
          pool().multiplier_,
          pool().z_offsets_[static_cast<uint8_t>(pool().point_)] * 100.f
      );
    }
  }
}

