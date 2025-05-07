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
#include "../../core/string.h"
#include "../../core/dgus.h"
#include "../../core/core.h"
#include "../../core/pool.h"
#include "sensor_settings.h"

#if ENABLED(ADVi3PP_PROBE)

namespace ADVi3pp::SensorSettings {

  inline namespace internals {
    struct SensorPosition { int16_t x, y; };

    constexpr size_t NB_SENSOR_POSITIONS = 3;
    constexpr uint16_t SENSOR_NAME_LENGTH = 14; //< Length of the field
    constexpr uint16_t KEY_CODE_PREVIOUS = 1;
    constexpr uint16_t KEY_CODE_NEXT = 2;
    constexpr uint16_t KEY_CODE_HIGH_SPEED = 3;
    constexpr uint16_t KEY_CODE_TOUCH_SW = 4;

    struct Data {
      uint16_t index_ = 0;
      bool highspeed_ = false;
      bool touch_sw_ = false;
    };

    inline Data& pool() { return Pool::get<Data>(Page::SensorSettings); }

    void show_command();
    void save_command();
    void previous_command();
    void next_command();
    void highspeed_command();
    void touch_sw_command();

    void send_values();
    void send_name();
    void get_values();
    void send_highspeed_value();
    void send_touch_sw_value();
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: Pages::back(Pages::BACK_OPTIONS::NONE); break;
      case KEY_CODE_SAVE: save_command(); break;
      case KEY_CODE_PREVIOUS: previous_command(); break;
      case KEY_CODE_NEXT: next_command(); break;
      case KEY_CODE_HIGH_SPEED: highspeed_command(); break;
      case KEY_CODE_TOUCH_SW: touch_sw_command(); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void show_command() {
      if(!Core::check_not_busy()) return;
      Pool::reset<Data>(Page::SensorSettings);
      pool().index_ = 0;
      pool().highspeed_ = ExtUI::isLevelingHighSpeed();
      pool().touch_sw_ = ExtUI::isLevelingTouchSw();
      send_name();
      send_values();
      send_highspeed_value();
      send_touch_sw_value();
      Pages::show(Page::SensorSettings);
    }

    //! Get the name of a sensor holder
    //! @param index Index of the holder
    //! @return The name (in Flash memory) of the holder
    const FlashChar* get_sensor_name(size_t index) {
      switch(index) {
        case 0: return GET_TEXT_F(ADVI3PP_STR_SENSOR_CURRENT);
        case 1: return GET_TEXT_F(ADVI3PP_STR_SENSOR_TEACHING_TECH);
        case 2: return GET_TEXT_F(ADVI3PP_STR_SENSOR_ADVI3PP);
        case 3:
    #if defined(BLTOUCH)
          return GET_TEXT_F(ADVI3PP_STR_SENSOR_INDIANAGIO);
    #elif defined(ADVi3PP_54)
          return GET_TEXT_F(ADVI3PP_STR_SENSOR_MARKII);
    #else
    #error "ADVi3PP_PROBE is defined but the kind of probe is unknown"
    #endif
        default: Log::error() << F("Invalid sensor index") << Log::endl(); return F("Error");
      }
    }

    //! Default position of the sensor for the different holders
    SensorPosition get_sensor_position(size_t index) {
        switch(index) {
    #if ENABLED(BLTOUCH)
        case 0: return {  +150, -4270 };    // Baseggio/Indianagio Front
        case 1: return { -2400, -3800 };    // Teaching Tech Left
        case 2: return { -2800, -4000 };    // ADVi3++ Left
    #elif defined(ADVi3PP_54)
    #else
        case 0: return {     0,  6000 };    // Mark II
        case 1: return { -2400, -3800 };    // Teaching Tech Left
        case 2: return { -2800, -4000 };    // ADVi3++ Left
    #error "ADVi3PP_PROBE is defined but the kind of probe is unknown"
    #endif
        default: Log::error() << F("Invalid sensor index") << Log::endl(); return {  +150, -4270 };
      }
    }

    //! Handles the Save (Continue) command
    void save_command() {
      get_values();
      ExtUI::setLevelingHighSpeed(pool().highspeed_);
      ExtUI::setLevelingTouchSw(pool().touch_sw_);
      Pages::save(Pages::SAVE_OPTIONS::SETTINGS | Pages::SAVE_OPTIONS::MESSAGE, Pages::BACK_OPTIONS::NONE);
    }


    //! Show the previous settings.
    void previous_command() {
      // not NB_SENSOR_POSITIONS-1 because we have also index #0 not counted in NB_SENSOR_POSITIONS
      if(pool().index_ <= 0) pool().index_ = NB_SENSOR_POSITIONS; else pool().index_ -= 1;
      send_name();
      send_values();
    }

    //! Show the next settings.
    void next_command() {
      // not NB_SENSOR_POSITIONS-1 because we have also index #0 not counted in NB_SENSOR_POSITIONS
      if(pool().index_ >= NB_SENSOR_POSITIONS) pool().index_ = 0; else pool().index_ += 1;
      send_name();
      send_values();
    }

    void highspeed_command() {
#if ENABLED(BLTOUCH)
      pool().highspeed_ = !pool().highspeed_;
      send_highspeed_value();
#endif
    }

    void touch_sw_command() {
#if ENABLED(BLTOUCH)
      pool().touch_sw_ = !pool().touch_sw_;
      send_touch_sw_value();
#endif
    }

    void send_highspeed_value() {
      WriteRamRequest{Variable::Value3}.write_word(pool().highspeed_ ? 1 : 0);
    }

    void send_touch_sw_value() {
      WriteRamRequest{Variable::Value4}.write_word(pool().touch_sw_ ? 1 : 0);
    }

    void send_values() {
      if(pool().index_ == 0) {
        auto x = lround(ExtUI::getProbeOffset_mm(ExtUI::X) * 100.0);
        auto y = lround(ExtUI::getProbeOffset_mm(ExtUI::Y) * 100.0);
        auto z = lround(ExtUI::getZOffset_mm() * 100.0);
        WriteRamRequest{Variable::Value0}.write_words(x, y, z);
      }
      else {
        const auto position = get_sensor_position(pool().index_ - 1);
        WriteRamRequest{Variable::Value0}.write_words(
            position.x,
            position.y,
            lround(ExtUI::getZOffset_mm() * 100.0)
        );
      }
    }

    void send_name() {
      WriteRamRequest{Variable::LongTextCentered0}.write_centered_text(get_sensor_name(pool().index_), SENSOR_NAME_LENGTH);
    }

    //! Get current data from the LCD Panel.
    void get_values() {
      ReadRam frame{Variable::Value0};
      if(!frame.send_receive(3)) return;
      auto x = frame.read_int() / 100.0;
      auto y = frame.read_int() / 100.0;
      auto z = frame.read_int() / 100.0;

      ExtUI::setProbeOffset_mm(x, ExtUI::X);
      ExtUI::setProbeOffset_mm(y, ExtUI::Y);
      ExtUI::setZOffset_mm(z);
    }

  }
}

#endif
