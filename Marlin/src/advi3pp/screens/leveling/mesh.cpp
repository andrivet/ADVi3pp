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
#include "../../../module/settings.h"
#include "../../../lcd/extui/ui_api.h"
#include "../../core/core.h"
#include "../../core/pool.h"
#include "../../core/status.h"
#include "../../core/progress.h"
#include "../common/wait.h"
#include "../common/set_temperature.h"
#include "mesh.h"


namespace ADVi3pp::Mesh {

  inline namespace internals {
    constexpr size_t NB_POINTS = GRID_MAX_POINTS_Y * GRID_MAX_POINTS_X;
    enum class Multiplier: uint8_t { M1, M2, M3 };

#if ENABLED(ADVi3PP_PROBE)
    constexpr Page PAGE = Page::AutomaticMesh;
    constexpr uint16_t KEY_CODE_RESET_PROBE = 1;
    constexpr Variable VAR_FIRST_MEASURE = Variable::Value0;
#else
    constexpr xyz_feedrate_t homing_feedrate_mm_m = HOMING_FEEDRATE_MM_M;
    constexpr float FEEDRATE_XY = MMM_TO_MMS(homing_feedrate_mm_m.x);
    constexpr float FEEDRATE_Z = MMM_TO_MMS(homing_feedrate_mm_m.z);
    constexpr Page PAGE = Page::ManualMesh;
    constexpr uint16_t KEY_CODE_PLUS = 1;
    constexpr uint16_t KEY_CODE_MINUS = 2;
    constexpr uint16_t KEY_CODE_OK = 3;
    constexpr uint16_t KEY_CODE_MULTIPLIER_1 = 4;
    constexpr uint16_t KEY_CODE_MULTIPLIER_2 = 5;
    constexpr uint16_t KEY_CODE_MULTIPLIER_3 = 6;
    constexpr Variable VAR_MULTIPLIER = Variable::Value0;
    constexpr Variable VAR_FIRST_MEASURE = Variable::Value1;
    constexpr Variable VAR_HEIGHT = Variable::ValueA;

    struct Data {
      Multiplier multiplier_ = Multiplier::M1;
      float old_offset_ = 0;
      millis_t last_click_time_ = 0;
    };
    inline Data& pool() { return Pool::get<Data>(PAGE); }
#endif

    bool lcd_leveling_ = false; // Do not use the pool

    void show_command();
    void back_command();

#if ENABLED(ADVi3PP_PROBE)
    void reset_command();
#else
    void minus_command();
    void plus_command();
    void ok_command();
    void multiplier1_command();
    void multiplier2_command();
    void multiplier3_command();

    float get_multiplier_value();
    void adjust_height(float offset);
#endif

    void send_height(double height);
    void send_measures(const adv::array<uint16_t, NB_POINTS> &data);
    void send_multiplier(Multiplier multiplier);
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: back_command(); break;
#if ENABLED(ADVi3PP_PROBE)
      case KEY_CODE_RESET_PROBE: reset_command(); break;
#else
      case KEY_CODE_OK: ok_command(); break;
      case KEY_CODE_PLUS: minus_command(); break;
      case KEY_CODE_MINUS: plus_command(); break;
      case KEY_CODE_MULTIPLIER_1: multiplier1_command(); break;
      case KEY_CODE_MULTIPLIER_2: multiplier2_command(); break;
      case KEY_CODE_MULTIPLIER_3: multiplier3_command(); break;
#endif
      default: return false;
    }
    return true;
  }

  void start() {
    Status::set(GET_TEXT_F(MSG_BED_LEVELING), Status::STATUS_OPTIONS::RESET);
    Progress::set_animation(true);
    Progress::reset();
  }

  void progress(uint8_t index) {
    Log::info() << F("Mesh::progress") << index << Log::endl();
    LCD_FORMAT_F(0, ADVI3PP_MSG_LEVELING_PROBING, index, NB_POINTS);

    if(!lcd_leveling_) return;

    Progress::set(index * 100 / NB_POINTS);

    adv::array<uint16_t, NB_POINTS> data{};
    data[index - 1] = 1;
    send_measures(data);
    send_height(ExtUI::getAxisPosition_mm(ExtUI::Z) * 100.0f);
  }

  //! Called by Marlin when G29 (automatic bed leveling) is finished.
  //! @param success Boolean indicating if the leveling was successful or not.
  void done(bool success) {
    Log::info() << F("done") << success << Log::endl();
    if(lcd_leveling_) {
      ExtUI::setTargetTemp_celsius(0, ExtUI::BED);
      Progress::set_animation(false);
      Progress::reset();
    }

    if(!success)
      Status::set(GET_TEXT_F(ADVI3PP_MSG_LEVELING_FAILURE), Status::STATUS_OPTIONS::RESET);
    else {
      ExtUI::saveSettings();
      ExtUI::setLevelingActive(true);
    }

    auto lcd_leveling = lcd_leveling_;
    lcd_leveling_ = false;
    if(!success) return;

    auto current = Pages::get_current_page();
    if(lcd_leveling && (current == Page::ManualMesh || current == Page::AutomaticMesh))
      Core::display(Page::SensorGrid, Core::DISPLAY_OPTIONS::CLEAR_CURRENT);
  }

  inline namespace internals {

    void show_command() {
      if(!Core::check_not_busy()) return;

      SetTemperature::bed([] (CALLBACK_SOURCE src) -> void {
        if(src == CALLBACK_SOURCE::BACK) {
          Pages::back(Pages::BACK_OPTIONS::NONE);
          return;
        }

        Status::reset();
        lcd_leveling_ = true;
        adv::array<uint16_t, GRID_MAX_POINTS_Y * GRID_MAX_POINTS_X> data{};
        send_measures(data);
        send_height(0);
        send_multiplier(Multiplier::M1);
        Wait::homing([] () -> void {
          Pages::show(PAGE);
#if ENABLED(ADVi3PP_PROBE)
          Core::inject_commands(ExtUI::isLevelingHighSpeed() ? F("G29\nG28 X Y") : F("G29 E\nG28 X Y"));
#else
          Core::inject_commands(F("G29 S1"));
#endif
        }, F("G28"));
      }, SetTemperature::OPTIONS::INIT_ZERO);
    }

    void back_command() {
      ExtUI::setTargetTemp_celsius(0, ExtUI::BED);
      Status::set(GET_TEXT_F(ADVI3PP_MSG_LEVELING_CANCEL), Status::STATUS_OPTIONS::RESET);
      ExtUI::cancelLeveling();
      Progress::set_animation(false);
      Progress::reset();
      Pages::back(Pages::BACK_OPTIONS::FINISH_MOVE);
    }

#if ENABLED(ADVi3PP_PROBE)
    void reset_command() {
      Status::set(GET_TEXT_F(ADVI3PP_MSG_SENSOR_RESET), Status::STATUS_OPTIONS::RESET);
      Core::inject_commands(F("M280 P0 S160"));
    }
#endif

    void send_height(double height) {
#if DISABLED(ADVi3PP_PROBE)
      if(lcd_leveling_) WriteRamRequest{VAR_HEIGHT}.write_word(height);
#endif
    }

    void send_measures(const adv::array<uint16_t, NB_POINTS> &data) {
      if(lcd_leveling_) WriteRamRequest{VAR_FIRST_MEASURE}.write_words_data(data.data(), data.size());
    }

    //! Send the current data (i.e. multiplier) to the LCD panel.
    void send_multiplier(Multiplier multiplier) {
#if DISABLED(ADVi3PP_PROBE)
      if(lcd_leveling_) {
      pool().multiplier_ = multiplier;
      WriteRamRequest{VAR_MULTIPLIER}.write_word(pool().multiplier_);
      }
#endif
    }

#if DISABLED(ADVi3PP_PROBE)
    // Use a function to save RAM
    float get_z_height_multiplier(size_t index) {
      switch(index) {
        case 0: return 0.02;
        case 1: return 0.10;
        case 2: return 1.0;
        default: Log::error() << (F("Invalid z height multiplier index")) << Log::endl(); break;
      }
      return 1.0;
    }

    //! Change the multiplier.
    void multiplier1_command() {
      send_multiplier(Multiplier::M1);
    }

    //! Change the multiplier.
    void multiplier2_command() {
      send_multiplier(Multiplier::M2);
    }

    //! Change the multiplier.
    void multiplier3_command() {
      send_multiplier(Multiplier::M3);
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
      if(!ELAPSED(millis(), pool().last_click_time_)) return;
      pool().last_click_time_ = millis();
      auto z = ExtUI::getAxisPosition_mm(ExtUI::Z) + offset;
      ExtUI::setAxisPosition_mm(z, ExtUI::Z, FEEDRATE_Z);
      send_height(z * 100.0f);
    }

    //! Change the position of the nozzle (-Z).
    void minus_command() {
      adjust_height(-get_multiplier_value());
    }

    //! Change the position of the nozzle (+Z).
    void plus_command() {
      adjust_height(+get_multiplier_value());
    }

    void ok_command() {
      Core::inject_commands(F("G29 S2"));
    }
#endif

  }
}
