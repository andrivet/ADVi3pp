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
#include "dimming.h"
#include "dgus.h"
#include "logging.h"
#include "task.h"

namespace ADVi3pp::Dimming {
  
  inline namespace internals {
    bool dimmed_ = false;
    Elapse elapse_{250};

    void reset_touch();
  }

  bool is_enabled() { return ui.sleep_timeout_enabled; }
  uint8_t get_dimming_time() { return ui.sleep_timeout_minutes; }
  uint8_t get_normal_brightness() { return ui.brightness; }
  uint8_t get_dimming_brightness() { return ui.sleep_timeout_brightness; }

  void set_settings(bool dimming, uint8_t dimming_time, uint8_t normal_brightness, uint8_t dimming_brightness) {
    ui.sleep_timeout_enabled = dimming;
    ui.set_screen_timeout(dimming_time);
    ui.set_brightness(normal_brightness);
    ui.set_dimming_brightness(dimming_brightness);
    send_brightness_to_lcd();
  }

  void init() {
    dimmed_ = false;
    send_brightness_to_lcd(LCD_BRIGHTNESS_MAX);
  }

  void send() {
    if(!is_enabled() || !dimmed_ || elapse_.is_pending()) return;
    ReadRegisterRequest{Register::TouchPanelFlag}.write(1);
  }

  bool receive() {
    SuspendLogging no_log{};
    bool received = false;

    ReadRegisterResponse response{Register::TouchPanelFlag};
    if(response.receive(false)) {
      received = true;

      // 0x5A means the panel was touched, we have to write 0 to clear the flag
      if(response.read_byte() == 0x5A) {
        no_log.resume();
        reset_touch();
        ui.refresh_screen_timeout();
        return true;
      }
    }

    if(!dimmed_ && is_enabled())
      ui.check_screen_timeout();

    return received;
  }

  //! Set the brightness of the LCD panel
  void send_brightness_to_lcd(uint8_t brightness) {
    WriteRegisterRequest{Register::Brightness}.write_byte(brightness);
  }

  //! Set the brightness of the LCD panel
  void send_brightness_to_lcd() {
    send_brightness_to_lcd(dimmed_ ? get_dimming_brightness() : get_normal_brightness());
  }

  void sleep_on() {
    if(dimmed_) return;
    dimmed_ = true;
    reset_touch();
    send_brightness_to_lcd();
  }

  void sleep_off() {
    if(!dimmed_) return;
    dimmed_ = false;
    send_brightness_to_lcd();
  }

  inline namespace internals {

    void reset_touch() {
      WriteRegisterRequest{Register::TouchPanelFlag}.write_byte(0);
    }

  }
}
