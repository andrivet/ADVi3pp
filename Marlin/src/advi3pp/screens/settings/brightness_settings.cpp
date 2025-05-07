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
#include "../../core/dimming.h"
#include "../../core/math.h"
#include "brightness_settings.h"

namespace ADVi3pp::BrightnessSettings {

  inline namespace internals {
    constexpr uint16_t KEY_CODE_DIMMING = 1;
    constexpr Variable VAR_ENABLE = Variable::Value0;
    constexpr uint8_t MIN_BRIGHTNESS = 5; // %
    constexpr uint8_t MAX_BRIGHTNESS = 100; // %

    void show_command();
    void back_command();
    void save_command();
    void dimming_command();

    uint8_t percent_2_raw(uint16_t percent);
    uint16_t raw_2_percent(uint8_t raw);
    void send_values(bool enabled, uint16_t time, uint8_t normal, uint8_t dimmed);
    bool get_values(bool& enabled, uint16_t &time, uint8_t &normal, uint8_t &dimmed);
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: back_command(); break;
      case KEY_CODE_SAVE: save_command(); break;
      case KEY_CODE_DIMMING: dimming_command(); break;
      default: return false;
    }
    return true;
  }

  //! Handle the change brightness command.
  void handle_normal_brightness_command(uint16_t brightness) {
    Dimming::send_brightness_to_lcd(percent_2_raw(brightness));
  }

  //! Handle the change brightness command.
  void handle_dimming_brightness_command(uint16_t brightness) {
    Dimming::send_brightness_to_lcd(percent_2_raw(brightness));
  }

  inline namespace internals {

    void show_command() {
      auto enabled = Dimming::is_enabled();
      auto dimming_time = Dimming::get_dimming_time();
      auto normal_brightness = raw_2_percent(Dimming::get_normal_brightness());
      auto dimming_brightness = raw_2_percent(Dimming::get_dimming_brightness());
      send_values(enabled, dimming_time, normal_brightness, dimming_brightness);
      Pages::show(Page::BrightnessSettings);
    }

    void back_command() {
      Dimming::send_brightness_to_lcd(); // Restore previous brightness
      Pages::back(Pages::BACK_OPTIONS::NONE);
    }

    uint8_t percent_2_raw(uint16_t percent) {
      return minmax<uint8_t>(lround(percent * LCD_BRIGHTNESS_MAX / 100.0), 0, LCD_BRIGHTNESS_MAX);
    }

    uint16_t raw_2_percent(uint8_t raw) {
      return minmax<uint8_t>(lround(raw * 100.0 / LCD_BRIGHTNESS_MAX), MIN_BRIGHTNESS, MAX_BRIGHTNESS);
    }

    void save_command() {
      bool enabled;
      uint16_t dimming_time; uint8_t normal_brightness, dimming_brightness;
      if(get_values(enabled, dimming_time, normal_brightness, dimming_brightness))
        Dimming::set_settings(
            enabled,
            dimming_time,
            percent_2_raw(normal_brightness),
            percent_2_raw(dimming_brightness)
        );
      Dimming::send_brightness_to_lcd();
      Pages::save(Pages::SAVE_OPTIONS::SETTINGS | Pages::SAVE_OPTIONS::MESSAGE, Pages::BACK_OPTIONS::NONE);
    }

    void send_values(bool enabled, uint16_t time, uint8_t normal, uint8_t dimmed) {
      WriteRamRequest{VAR_ENABLE}.write_words(enabled, time);
      WriteRamRequest{Variable::NormalBrightness}.write_words(normal, dimmed);
    }

    bool get_values(bool& enable, uint16_t &time, uint8_t &normal, uint8_t &dimmed) {
      ReadRam frame{VAR_ENABLE};
      if(!frame.send_receive(2)) return false;
      enable = frame.read_bool();
      time = frame.read_uint();

      ReadRam frame2{Variable::NormalBrightness};
      if(!frame2.send_receive(2)) return false;
      normal = frame2.read_uint();
      dimmed = frame2.read_uint();
      return true;
    }

    //! Handle the Dimming (On/Off) command
    void dimming_command() {
      ReadRam frame{VAR_ENABLE};
      if(!frame.send_receive(1)) return;
      bool enable = frame.read_bool();
      WriteRamRequest{VAR_ENABLE}.write_words(enable ? 0 : 1);
    }

  }
}
