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
#include "move.h"

namespace ADVi3pp {
  enum class Direction: uint8_t {
    None,
    X_PLUS,
    Y_PLUS,
    Z_PLUS,
    E_PLUS,
    MINUS   = 0x80,
    X_MINUS = MINUS + X_PLUS,
    Y_MINUS = MINUS + Y_PLUS,
    Z_MINUS = MINUS + Z_PLUS,
    E_MINUS = MINUS + E_PLUS
  };
}
ENABLE_BITMASK_OPERATOR(ADVi3pp::Direction);

namespace ADVi3pp::Move {

  inline namespace internals {
    constexpr feedRate_t FEEDRATE_X = 20;
    constexpr feedRate_t FEEDRATE_Y = 20;
    constexpr feedRate_t FEEDRATE_Z = 10;
    constexpr feedRate_t FEEDRATE_E = 2;
    constexpr unsigned TASK_DELAY = 250; // Has to be more than DEFAULT_TASK_DELAY
    constexpr uint16_t KEY_CODE_DISABLE_MOTORS = 1;
    constexpr uint16_t KEY_CODE_X_MINUS = 2;
    constexpr uint16_t KEY_CODE_X_PLUS = 3;
    constexpr uint16_t KEY_CODE_Y_MINUS = 5;
    constexpr uint16_t KEY_CODE_Y_PLUS = 4;
    constexpr uint16_t KEY_CODE_Z_MINUS = 6;
    constexpr uint16_t KEY_CODE_Z_PLUS = 7;
    constexpr uint16_t KEY_CODE_E_MINUS = 8;
    constexpr uint16_t KEY_CODE_E_PLUS = 9;
    constexpr uint16_t KEY_CODE_HOME_X = 0x0A;
    constexpr uint16_t KEY_CODE_HOME_Y = 0x0B;
    constexpr uint16_t KEY_CODE_HOME_Z = 0x0C;
    constexpr uint16_t KEY_CODE_HOME_ALL = 0x0D;

    struct Data {
      Direction direction_ = Direction::None;
    };

    inline Data& pool() { return Pool::get<Data>(Page::Move); }

    void show_command();
    void back_command();
    void x_plus_command();
    void x_minus_command();
    void x_home_command();
    void y_plus_command();
    void y_minus_command();
    void y_home_command();
    void z_plus_command();
    void z_minus_command();
    void z_home_command();
    void e_plus_command();
    void e_minus_command();
    void all_home_command();
    void disable_motors_command();

    float get_target();
    feedRate_t get_feedrate();
    float get_position();
    void set_position();
    void move(Direction direction);
    void stop_move();
  }

  //! Execute a move command
  //! @param key_value    The sub-action to handle
  //! @return             True if the action was handled
  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: back_command(); break;
      case KEY_CODE_DISABLE_MOTORS: disable_motors_command(); break;
      case KEY_CODE_X_MINUS: x_minus_command(); break;
      case KEY_CODE_X_PLUS: x_plus_command(); break;
      case KEY_CODE_Y_MINUS: y_minus_command(); break;
      case KEY_CODE_Y_PLUS: y_plus_command(); break;
      case KEY_CODE_Z_MINUS: z_minus_command(); break;
      case KEY_CODE_Z_PLUS: z_plus_command(); break;
      case KEY_CODE_E_MINUS: e_minus_command(); break;
      case KEY_CODE_E_PLUS: e_plus_command(); break;
      case KEY_CODE_HOME_X: x_home_command(); break;
      case KEY_CODE_HOME_Y: y_home_command(); break;
      case KEY_CODE_HOME_Z: z_home_command(); break;
      case KEY_CODE_HOME_ALL: all_home_command(); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void show_command() {
      if(!Core::check_not_busy()) return;
      Pool::reset<Data>(Page::Move);
      Status::reset();
      ExtUI::finishAndDisableHeaters(); // To circumvent homing problems
      pool().direction_ = Direction::None;
      Pages::show(Page::Move);
    }

    void back_command() {
      Pages::back(Pages::BACK_OPTIONS::FINISH_MOVE);
    }

    //! Disable the motors.
    void disable_motors_command() {
      // If busy, do nothing
      if(Core::is_busy()) {
        Status::set(GET_TEXT_F(ADVI3PP_MSG_BUSY));
        return;
      }

      stop_move();
      Core::inject_commands(F("M84")); // Disable steppers
      ExtUI::setAllAxisUnhomed();
      ExtUI::setAllAxisPositionUnknown();
      Status::set(GET_TEXT_F(ADVI3PP_MSG_MOTORS_DISABLED), Status::STATUS_OPTIONS::RESET);
    }

    //! Go to home on the X axis.
    void x_home_command() {
      Wait::homing(F("G28 X"));
    }

    //! Go to home on the Y axis.
    void y_home_command() {
      Wait::homing(F("G28 Y"));
    }

    //! Go to home on the Z axis.
    void z_home_command() {
      Wait::homing(F("G28 Z"));
    }

    //! Go to home on all axis.
    void all_home_command() {
      Wait::homing(F("G28"));
    }

    //! Move the nozzle (+X)
    void x_plus_command() {
      move(Direction::X_PLUS);
    }

    //! Move the nozzle (-X)
    void x_minus_command() {
      move(Direction::X_MINUS);
    }

    //! Move the nozzle (+Y)
    void y_plus_command() {
      move(Direction::Y_PLUS);
    }

    //! Move the nozzle (-Y)
    void y_minus_command() {
      move(Direction::Y_MINUS);
    }

    //! Move the nozzle (+Z)
    void z_plus_command() {
      move(Direction::Z_PLUS);
    }

    //! Move the nozzle (-Z)
    void z_minus_command() {
      move(Direction::Z_MINUS);
    }

    //! Extrude some filament.
    void e_plus_command() {
      move(Direction::E_PLUS);
    }

    //! Unextrude some filament.
    void e_minus_command() {
      move(Direction::E_MINUS);
    }

    // Do not use a struct table because it will take RAM space. So use fonctions instead.
    float get_target() {
      switch(pool().direction_) {
        case Direction::X_PLUS: return Core::MAX_X;
        case Direction::Y_PLUS: return Core::MAX_Y;
        case Direction::Z_PLUS: return Core::MAX_Z;
        case Direction::E_PLUS: return Core::MAX_E;
        case Direction::X_MINUS: return Core::MIN_X;
        case Direction::Y_MINUS: return Core::MIN_Y;
        case Direction::Z_MINUS: return Core::MIN_Z;
        case Direction::E_MINUS: return Core::MIN_E;
        default: Log::error() << F("Invalid Direction: ") << static_cast<uint8_t>(pool().direction_) << Log::endl(); return 0;
      }
    }

    feedRate_t get_feedrate() {
      switch(get_cleared_bits(pool().direction_, Direction::MINUS)) {
        case Direction::X_PLUS: return FEEDRATE_X;
        case Direction::Y_PLUS: return FEEDRATE_Y;
        case Direction::Z_PLUS: return FEEDRATE_Z;
        case Direction::E_PLUS: return FEEDRATE_E;
        default: Log::error() << F("Invalid Direction: ") << static_cast<uint8_t>(pool().direction_) << Log::endl(); break;
      }
      return FEEDRATE_X;
    }

    float get_position() {
      switch(get_cleared_bits(pool().direction_, Direction::MINUS)) {
        case Direction::X_PLUS: return ExtUI::getAxisPosition_mm(ExtUI::X);
        case Direction::Y_PLUS: return ExtUI::getAxisPosition_mm(ExtUI::Y);
        case Direction::Z_PLUS: return ExtUI::getAxisPosition_mm(ExtUI::Z);
        case Direction::E_PLUS: return ExtUI::getAxisPosition_mm(ExtUI::E0);
        default: Log::error() << F("Invalid Direction: ") << static_cast<uint8_t>(pool().direction_) << Log::endl(); break;
      }
      return 0;
    }

    void set_position() {
      float new_position = get_target();
      auto current_position = get_position();
      if(current_position == new_position) return;
      auto feedrate = get_feedrate();
      Log::info() << F("Move::set_position, new") << new_position << F("current") << current_position << Log::endl();
      switch(get_cleared_bits(pool().direction_,Direction::MINUS)) {
        case Direction::X_PLUS: ExtUI::setAxisPosition_mm(new_position, ExtUI::X, feedrate); break;
        case Direction::Y_PLUS: ExtUI::setAxisPosition_mm(new_position, ExtUI::Y, feedrate); break;
        case Direction::Z_PLUS: ExtUI::setAxisPosition_mm(new_position, ExtUI::Z, feedrate); break;
        case Direction::E_PLUS: ExtUI::setAxisPosition_mm(new_position, ExtUI::E0, feedrate); break;
        default: Log::error() << F("Invalid Direction: ") << static_cast<uint8_t>(pool().direction_) << Log::endl(); break;
      }
    }

    void move(Direction direction) {
      // First move?
      if(pool().direction_ != direction) {
        Log::info() << F("First move") << Log::endl();
        pool().direction_ = direction;
        background_task.set([] () -> CALLBACK_RESULT {
          Log::info() << F("Stop moving") << Log::endl();
          stop_move();
          return CALLBACK_RESULT::STOP;
        }, TASK_DELAY);
        set_position();
      }
      else {
        Log::info() << F("Keep moving") << Log::endl();
        background_task.rearm();
      }
    }

    void stop_move() {
      ExtUI::stopMove(); // Warning: will call idle() and thus is reentrant
      Log::info() << F("New position") << get_position() << Log::endl();
      pool().direction_ = Direction::None;
    }
  }
}
