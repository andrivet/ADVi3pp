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
#include "io.h"
#include "../../core/core.h"
#include "../../core/dgus.h"
#include "../../core/status.h"

namespace ADVi3pp::Io {

  inline namespace internals {
  #if defined(ADVi3PP_51)
    constexpr unsigned NB_DIGITAL_PINS = 5;
  #else
      constexpr unsigned NB_DIGITAL_PINS = 4;
  #endif
    constexpr unsigned NB_ANALOG_PINS = 3;

    void show_command();

    uint8_t get_digital_pin(size_t index);
    uint8_t get_analog_pin(size_t index);
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: Pages::back(Pages::BACK_OPTIONS::NONE); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void show_command() {
#if defined(ADVi3PP_51)
      const auto page = Page::IO_51;
#else
      const auto page = Page::IO_52;
#endif

      Status::reset();
      background_task.set([] () -> CALLBACK_RESULT {
        adv::array<uint16_t, NB_DIGITAL_PINS> values{};

        // Direction (input, output)
        for(size_t i = 0; i < NB_DIGITAL_PINS; ++i)
          values[i] = static_cast<uint16_t>(Core::get_pin_direction(get_digital_pin(i)));
        WriteRamRequest{Variable::Value0}.write_words_data(values.data(), NB_DIGITAL_PINS);

        // State (on, off)
        for(size_t i = 0; i < NB_DIGITAL_PINS; ++i)
          values[i] = static_cast<uint16_t>(Core::get_pin_state(get_digital_pin(i)));
        WriteRamRequest{Variable::Value16}.write_words_data(values.data(), NB_DIGITAL_PINS);

        // Pull-up, Hi-Z, Sink, Source
        for(size_t i = 0; i < NB_DIGITAL_PINS; ++i)
          values[i] = static_cast<uint16_t>(Core::get_pin_pull(get_digital_pin(i)));
        WriteRamRequest{Variable::Value32}.write_words_data(values.data(), NB_DIGITAL_PINS);

        // Analog values
        for(size_t i = 0; i < NB_ANALOG_PINS; ++i)
          values[i] = analogRead(get_analog_pin(i));
        WriteRamRequest{Variable::Value48}.write_words_data(values.data(), NB_ANALOG_PINS);

        return CALLBACK_RESULT::CONTINUE;
      });
      Pages::show(page);
    }

    #if defined(ADVi3PP_51)
    uint8_t get_digital_pin(size_t index) {
      switch(index) {
        case 0: return 54;     // Xmin - PF0 / A0 - D54
        case 1: return 24;     // Ymin - PA2 / AD2 - D24
        case 2: return 23;     // Zmin - PA1 / AD1 - D23
        case 3: return 25;     // BLTouch - PA3 / AD3 - D25
        case 4: return 49;     // SD Detect - PL0 - D49
        default: Log::error() << F("get_digital_pin: Invalid digit index") << Log::endl(); break;
      }
      return 0;
    }
    #else
        uint8_t get_digital_pin(size_t index) {
      switch(index) {
        case 0: return 54;     // Xmin - PF0 / A0 - D54
        case 1: return 24;     // Ymin - PA2 / AD2 - D24
        case 2: return  6;     // Zmin - PH3 / Dß6
        case 3: return 49;     // SD Detect - PL0 - D49
        default: Log::error() << F("get_digital_pin: Invalid digit index") << index << Log::endl(); break;
      }
      return 0;
    }
    #endif

    uint8_t get_analog_pin(size_t index) {
      switch(index) {
        case 0: return 55;   // Extruder - A1
        case 1: return 68;   // Bed - A14
        case 2: return 54;   // Xmin - A0
        default: Log::error() << F("get_analog_pin: Invalid digit index") << index << Log::endl(); break;
      }
      return 0;
    }

  }
}

namespace ADVi3pp::IoExt {

  inline namespace internals {
    constexpr unsigned NB_DIGITAL_PINS = 8;
    constexpr unsigned NB_ANALOG_PINS = 1;

    void show_command();

    uint8_t get_digital_pin(size_t index);
    uint8_t get_analog_pin(size_t index);
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: Pages::back(Pages::BACK_OPTIONS::NONE); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void show_command() {
      const auto page = Page::IO_EXT;

      Status::reset();
      background_task.set([] () -> CALLBACK_RESULT {
        adv::array<uint16_t, NB_DIGITAL_PINS> values{};

        // Direction (input, output)
        for(size_t i = 0; i < NB_DIGITAL_PINS; ++i)
          values[i] = static_cast<uint16_t>(Core::get_pin_direction(get_digital_pin(i)));
        WriteRamRequest{Variable::Value0}.write_words_data(values.data(), NB_DIGITAL_PINS);

        // State (on, off)
        for(size_t i = 0; i < NB_DIGITAL_PINS; ++i)
          values[i] = static_cast<uint16_t>(Core::get_pin_state(get_digital_pin(i)));
        WriteRamRequest{Variable::Value16}.write_words_data(values.data(), NB_DIGITAL_PINS);

        // Pull-up, Hi-Z, Sink, Source
        for(size_t i = 0; i < NB_DIGITAL_PINS; ++i)
          values[i] = static_cast<uint16_t>(Core::get_pin_pull(get_digital_pin(i)));
        WriteRamRequest{Variable::Value32}.write_words_data(values.data(), NB_DIGITAL_PINS);

        // Analog values
        for(size_t i = 0; i < NB_ANALOG_PINS; ++i)
          values[i] = analogRead(get_analog_pin(i));
        WriteRamRequest{Variable::Value48}.write_words_data(values.data(), NB_ANALOG_PINS);

        return CALLBACK_RESULT::CONTINUE;
      });
      Pages::show(page);
    }

    uint8_t get_digital_pin(size_t index) {
      switch(index) {
        case 0: return 40;     // Ext 1 - PG1 / !RD - D40
        case 1: return 56;     // Ext 2 - PF2 / A2 - D56
        case 2: return 36;     // Ext 3 - PC1 / D36
        case 3: return 37;     // Ext 4 - PC0 / D37
        case 4: return 34;     // Ext 5 - PC3 / D34
        case 5: return 35;     // Ext 6 - PC2 / D35
        case 6: return 32;     // Ext 7 - PC5 / D32
        case 7: return 33;     // Ext 8 - PC4 / D33
        default: Log::error() << F("get_digital_pin: Invalid digit index") << index << Log::endl(); break;
      }
      return 0;
    }

    uint8_t get_analog_pin(size_t index) {
      switch(index) {
        case 0: return 56;   // Ext 2 - A2
        default: Log::error() << F("get_analog_pin: Invalid digit index") << index << Log::endl(); break;
      }
      return 0;
    }

  }
}
