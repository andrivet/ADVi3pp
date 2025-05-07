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
#include "../../core/dgus.h"
#include "../../core/status.h"
#include "../../core/pool.h"
#include "../common/wait.h"
#include "vibrations.h"

namespace ADVi3pp::Vibrations {

  inline namespace internals {
    constexpr int XY_SLOW = 4000;
    constexpr int XY_MEDIUM = 6000;
    constexpr int XY_FAST = 8000;
    constexpr int Z_SLOW = 1000;
    constexpr int Z_MEDIUM = 1200;
    constexpr int Z_FAST = 1400;
    constexpr uint16_t KEY_CODE_X = 1;
    constexpr uint16_t KEY_CODE_Y = 2;
    constexpr uint16_t KEY_CODE_Z = 3;
    constexpr uint16_t KEY_CODE_XY = 4;
    constexpr uint16_t KEY_CODE_YX = 5;
    constexpr uint16_t KEY_CODE_SLOW = 6;
    constexpr uint16_t KEY_CODE_MEDIUM = 7;
    constexpr uint16_t KEY_CODE_FAST = 8;
    constexpr Variable VAR_SPEED = Variable::Value0;
    enum class Speed { Slow, Medium, Fast };

    struct Data {
      Speed speed_ = Speed::Medium;
    };

    inline Data& pool() { return Pool::get<Data>(Page::VibrationsTuning); }

    void show_command();
    void back_command();
    void x_command();
    void y_command();
    void xy_command();
    void yx_command();
    void z_command();
    void slow_command();
    void medium_command();
    void fast_command();

    bool get_values(int &min, int &max);
    int get_xy_speed();
    int get_z_speed();
  }

  bool handle_command(uint16_t key_code) {
    switch(key_code) {
      case KEY_CODE_SHOW: show_command(); break;
      case KEY_CODE_BACK: back_command(); break;
      case KEY_CODE_X: x_command(); break;
      case KEY_CODE_Y: y_command(); break;
      case KEY_CODE_XY: xy_command(); break;
      case KEY_CODE_YX: yx_command(); break;
      case KEY_CODE_Z: z_command(); break;
      case KEY_CODE_SLOW: slow_command(); break;
      case KEY_CODE_MEDIUM: medium_command(); break;
      case KEY_CODE_FAST: fast_command(); break;
      default: return false;
    }
    return true;
  }

  inline namespace internals {

    void show_command() {
      if(!Core::check_not_busy()) return;
      Pool::reset<Data>(Page::VibrationsTuning);
      Status::reset();
      WriteRamRequest{VAR_SPEED}.write_words(pool().speed_);
      Wait::homing([] () -> void {
        Pages::show(Page::VibrationsTuning);
      });
    }

    void back_command() {
      background_task.clear();
      Core::inject_commands(F("G1 Z10 F1200"));
      Pages::back(Pages::BACK_OPTIONS::FINISH_MOVE);
    }

    void slow_command() {
      pool().speed_ = Speed::Slow;
      WriteRamRequest{VAR_SPEED}.write_words(pool().speed_);
    }

    void medium_command() {
      pool().speed_ = Speed::Medium;
      WriteRamRequest{VAR_SPEED}.write_words(pool().speed_);
    }

    void fast_command() {
      pool().speed_ = Speed::Fast;
      WriteRamRequest{VAR_SPEED}.write_words(pool().speed_);
    }

    void move_start_xy(int x, int y) {
      ADVString<40> cmd;
      cmd.format(F("G1 X%i F6000\nG1 Y%i F6000"), x, y);
      Core::inject_commands(cmd.get());
      background_task.set([] () -> CALLBACK_RESULT {
        if(Core::is_busy()) return CALLBACK_RESULT::CONTINUE;

        int min, max;
        if(!get_values(min, max)) return CALLBACK_RESULT::CONTINUE;

        int min_x = (min < X_MIN_BED) ? X_MIN_BED : min;
        int max_x = (max > X_MAX_BED) ? X_MAX_BED : max;
        int min_y = (min < Y_MIN_BED) ? Y_MIN_BED : min;
        int max_y = (max > Y_MAX_BED) ? Y_MAX_BED : max;

        int new_x_position = (ExtUI::getAxisPosition_mm(ExtUI::X) == max_x) ? min_x : max_x;
        int new_y_position = (ExtUI::getAxisPosition_mm(ExtUI::Y) == max_y) ? min_y : max_y;

        ADVString<40> cmd;
        cmd.format(F("G1 X%i Y%i F%i"), new_x_position, new_y_position, get_xy_speed());
        Core::inject_commands(cmd.get());
        return CALLBACK_RESULT::CONTINUE;
      });
    }

    bool get_values(int &min, int &max) {
      ReadRam frame{VAR_SPEED};
      if(!frame.send_receive(1)) return false;
      pool().speed_ = frame.read_enum<Speed>();
      return true;
    }

    int get_xy_speed() {
      switch(pool().speed_) {
        case Speed::Slow: return XY_SLOW;
        case Speed::Medium: return XY_MEDIUM;
        case Speed::Fast: return XY_FAST;
        default: return XY_MEDIUM;
      }
    }

    int get_z_speed() {
      switch(pool().speed_) {
        case Speed::Slow: return Z_SLOW;
        case Speed::Medium: return Z_MEDIUM;
        case Speed::Fast: return Z_FAST;
        default: return Z_MEDIUM;
      }
    }

    void x_command() {
      Status::set(GET_TEXT_F(ADVI3PP_MSG_VIBRATIONS_X), Status::STATUS_OPTIONS::RESET | Status::STATUS_OPTIONS::PERSISTENT);
      background_task.set([] () -> CALLBACK_RESULT {
        if(Core::is_busy()) return CALLBACK_RESULT::CONTINUE;

        int min, max;
        if(!get_values(min, max)) return CALLBACK_RESULT::CONTINUE;

        if(min < X_MIN_BED) min = X_MIN_BED;
        if(max > X_MAX_BED) max = X_MAX_BED;

        int new_position = (ExtUI::getAxisPosition_mm(ExtUI::X) == max) ? min : max;

        ADVString<20> cmd;
        cmd.format(F("G1 X%i F%i"), new_position, get_xy_speed());
        Log::info() << cmd.get() << Log::endl();
        Core::inject_commands(cmd.get());
        return CALLBACK_RESULT::CONTINUE;
      });
    }

    void y_command() {
      Status::set(GET_TEXT_F(ADVI3PP_MSG_VIBRATIONS_Y), Status::STATUS_OPTIONS::RESET | Status::STATUS_OPTIONS::PERSISTENT);
      background_task.set([] () -> CALLBACK_RESULT {
        if(Core::is_busy()) return CALLBACK_RESULT::CONTINUE;

        int min, max;
        if(!get_values(min, max)) return CALLBACK_RESULT::CONTINUE;

        if(min < Y_MIN_BED) min = Y_MIN_BED;
        if(max > Y_MAX_BED) max = Y_MAX_BED;

        int new_position = (ExtUI::getAxisPosition_mm(ExtUI::Y) == max) ? min : max;

        ADVString<20> cmd;
        cmd.format(F("G1 Y%i F%i"), new_position, get_xy_speed());
        Core::inject_commands(cmd.get());
        return CALLBACK_RESULT::CONTINUE;
      });
    }

    void xy_command() {
      Status::set(GET_TEXT_F(ADVI3PP_MSG_VIBRATIONS_XY), Status::STATUS_OPTIONS::RESET | Status::STATUS_OPTIONS::PERSISTENT);
      background_task.set([] () -> CALLBACK_RESULT {
        move_start_xy(X_MIN_BED, Y_MIN_BED);
        return CALLBACK_RESULT::CONTINUE;
      });
    }

    void yx_command() {
      Status::set(GET_TEXT_F(ADVI3PP_MSG_VIBRATIONS_YX), Status::STATUS_OPTIONS::RESET | Status::STATUS_OPTIONS::PERSISTENT);
      background_task.set([] () -> CALLBACK_RESULT {
        move_start_xy(X_MIN_BED, Y_MAX_BED);
        return CALLBACK_RESULT::CONTINUE;
      });
    }

    void z_command() {
      Status::set(GET_TEXT_F(ADVI3PP_MSG_VIBRATIONS_Z), Status::STATUS_OPTIONS::RESET | Status::STATUS_OPTIONS::PERSISTENT);
      background_task.set([] () -> CALLBACK_RESULT {
        ADVString<40> cmd;
        cmd.format(F("G1 X%i F6000\nG1 Y%i F6000"), X_CENTER, Y_CENTER);
        Core::inject_commands(cmd.get());

        background_task.set([] () -> CALLBACK_RESULT {
          if(Core::is_busy()) return CALLBACK_RESULT::CONTINUE;

          int min, max;
          if(!get_values(min, max)) return CALLBACK_RESULT::CONTINUE;

          if(min < 5) min = 5;
          if(max > 150) max = 150; // Don't go to the limit, could be dangerous with some mods

          int new_position = (ExtUI::getAxisPosition_mm(ExtUI::Z) == max) ? min : max;

          ADVString<20> cmd;
          cmd.format(F("G1 Z%i F%i"), new_position, get_z_speed());
          Core::inject_commands(cmd.get());
          return CALLBACK_RESULT::CONTINUE;
        });

        return CALLBACK_RESULT::STOP;
      });
    }

  }
}
