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

#pragma once

#include "../../inc/MarlinConfig.h"
#include "string.h"
#include "task.h"
#include "enums.h"
#include "../../module/temperature.h"

class GCodeParser;

namespace ADVi3pp {

  // ----------------------------------------------------------------------------
  // Once - Execute only one time
  // ----------------------------------------------------------------------------

  struct Once {
    operator bool();
  private:
    bool once_ = true;
  };

  // ----------------------------------------------------------------------------
  // Core
  // ----------------------------------------------------------------------------

  namespace Core {
    enum class PIN_STATE: uint8_t { Low = 0, High = 1 };
    enum class PIN_DIRECTION: uint8_t { Input = 0, Output = 1 };
    enum class PIN_PULL: uint8_t { High = 1, Low = 2, HiZ = 0, Up = 1, None = 0 };
    enum class DISPLAY_OPTIONS { NONE = 0x0000, CLEAR_TEMPORARIES = 0x0001, BACK_ALL = 0x0002, CLEAR_CURRENT = 0x0004 };

    constexpr long MAX_X = X_MAX_POS;
    constexpr long MIN_X = X_MIN_POS;
    constexpr long MAX_Y = Y_MAX_POS;
    constexpr long MIN_Y = Y_MIN_POS;
    constexpr long MAX_Z = Z_MAX_POS;
    constexpr long MIN_Z = Z_MIN_POS;
    constexpr long MAX_E = EXTRUDE_MAXLENGTH * 0.9;
    constexpr long MIN_E = -EXTRUDE_MAXLENGTH * 0.9;

    void startup();
    void idle();
    void killed(const FlashChar* error, const FlashChar* component);
    void killed(const FlashChar* error, heater_id_t header_id);
    void send_lcd_zero();

    bool is_busy();
    bool is_printing();
    bool is_print_paused();

    bool check_not_busy();
    void inject_commands(const FlashChar* commands);
    void inject_commands(const char *commands);
    void process(Page page, uint16_t key_code, uint16_t arg);
    void print_started();
    void print_paused();
    void print_done();
    void pause_temperature();
    void display(Page page, DISPLAY_OPTIONS options = DISPLAY_OPTIONS::NONE, uint16_t arg = 0);

    PIN_STATE get_pin_state(uint8_t pin);
    PIN_DIRECTION get_pin_direction(uint8_t pin);
    PIN_PULL get_pin_pull(uint8_t pin);
    float ensure_z_enough_room();

    void on_settings_loaded(bool success);
    void on_settings_validated(bool success);
    bool are_settings_invvalid();

    void on_power_loss_set(bool set);
    void on_power_loss();
    void on_power_loss_resume();
    void on_power_off();
  }
}

ENABLE_BITMASK_OPERATOR(ADVi3pp::Core::DISPLAY_OPTIONS);
