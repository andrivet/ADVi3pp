/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2022 Sebastien Andrivet [https://github.com/andrivet/]
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

#include "../../parameters.h"
#include "move.h"
#include "../../core/core.h"

namespace ADVi3pp {

namespace {
  const millis_t   CLICK_DELAY = 200;
  const float      OFFSET_X = 5;
  const float      OFFSET_Y = 5;
  const float      OFFSET_Z = 0.5;
  const float      OFFSET_E = 1;
  const feedRate_t FEEDRATE_X = 14;
  const feedRate_t FEEDRATE_Y = 14;
  const feedRate_t FEEDRATE_Z = 4;
  const feedRate_t FEEDRATE_E = 2;
}

Move move;

ENABLE_BITMASK_OPERATOR(Move::Direction);

// Do not use a struct table because it will take RAM space. So use fonctions instead.

float Move::get_min() const {
  switch(direction_ & ~Direction::MINUS) {
    case Direction::X_PLUS: return X_MIN_BED;
    case Direction::Y_PLUS: return Y_MIN_BED;
    case Direction::Z_PLUS: return Z_MIN_POS;
    case Direction::E_PLUS: return -EXTRUDE_MAXLENGTH;
    default: Log::log() << "Invalid Direction: " << static_cast<uint8_t>(direction_) << Log::endl(); return 0;
  }
}

float Move::get_max() const {
  switch(direction_ & ~Direction::MINUS) {
    case Direction::X_PLUS: return X_MAX_BED;
    case Direction::Y_PLUS: return Y_MAX_BED;
    case Direction::Z_PLUS: return Z_MAX_POS;
    case Direction::E_PLUS: return EXTRUDE_MAXLENGTH;
    default: Log::log() << "Invalid Direction: " << static_cast<uint8_t>(direction_) << Log::endl(); return 0;
  }
}

float Move::get_offset() const {
  bool minus = test_one_bit(direction_, Direction::MINUS);
  switch(direction_ & ~Direction::MINUS) {
    case Direction::X_PLUS: return minus ? -OFFSET_X : OFFSET_X;
    case Direction::Y_PLUS: return minus ? -OFFSET_Y : OFFSET_Y;
    case Direction::Z_PLUS: return minus ? -OFFSET_Z : OFFSET_Z;
    case Direction::E_PLUS: return minus ? -OFFSET_E : OFFSET_E;
    default: Log::log() << "Invalid Direction: " << static_cast<uint8_t>(direction_) << Log::endl(); return 0;
  }
}

feedRate_t Move::get_feedrate() const {
  switch(direction_ & ~Direction::MINUS) {
    case Direction::X_PLUS: return FEEDRATE_X;
    case Direction::Y_PLUS: return FEEDRATE_Y;
    case Direction::Z_PLUS: return FEEDRATE_Z;
    case Direction::E_PLUS: return FEEDRATE_E;
    default: Log::log() << "Invalid Direction: " << static_cast<uint8_t>(direction_) << Log::endl(); return 0;
  }
}

float Move::getPosition() const {
  switch(direction_ & ~Direction::MINUS) {
    case Direction::X_PLUS: return ExtUI::getAxisPosition_mm(ExtUI::X);
    case Direction::Y_PLUS: return ExtUI::getAxisPosition_mm(ExtUI::Y);
    case Direction::Z_PLUS: return ExtUI::getAxisPosition_mm(ExtUI::Z);
    case Direction::E_PLUS: return ExtUI::getAxisPosition_mm(ExtUI::E0);
    default: Log::log() << "Invalid Direction: " << static_cast<uint8_t>(direction_) << Log::endl(); return 0;
  }
}

void Move::setPosition() const {
  Log::log() << "setPosition" << target_position_ << ", " << millis() << Log::endl();
  auto feedrate = get_feedrate();
  switch(direction_ & ~Direction::MINUS) {
    case Direction::X_PLUS: ExtUI::setAxisPosition_mm(target_position_, ExtUI::X, feedrate); break;
    case Direction::Y_PLUS: ExtUI::setAxisPosition_mm(target_position_, ExtUI::Y, feedrate); break;
    case Direction::Z_PLUS: ExtUI::setAxisPosition_mm(target_position_, ExtUI::Z, feedrate); break;
    case Direction::E_PLUS: ExtUI::setAxisPosition_mm(target_position_, ExtUI::E0, feedrate); break;
    default: Log::log() << "Invalid Direction: " << static_cast<uint8_t>(direction_) << Log::endl(); break;
  }
}


//! Execute a move command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool Move::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::MoveXHome:           x_home_command(); break;
        case KeyValue::MoveYHome:           y_home_command(); break;
        case KeyValue::MoveZHome:           z_home_command(); break;
        case KeyValue::MoveAllHome:         all_home_command(); break;
        case KeyValue::DisableMotors:       disable_motors_command(); break;
        default: Log::log() << "Invalid KeyValue" << Log::endl(); return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page Move::do_prepare_page()
{
    if(!core.ensure_not_printing())
        return Page::None;
    ExtUI::finishAndDisableHeaters(); // To circumvent homing problems
    direction_ = Direction::None;
    background_task.set(Callback{this, &Move::task}, 50);
    return Page::Move;
}

constexpr millis_t move_delay(float offset, feedRate_t feedrate) {
  return 1000 * abs(offset) / feedrate;
}

void Move::move(Move::Direction movement) {
  // First move?
  if(direction_ != movement) {
    direction_ = movement;

    float min = get_min();
    float max = get_max();
    float offset = get_offset();
    feedRate_t feedrate = get_feedrate();

    target_position_ = getPosition() + offset;
    target_position_ = (target_position_ < min) ? min : (target_position_ > max) ? max : target_position_;

    setPosition();

    target_move_time_ = millis() + move_delay(offset, feedrate);
  }

  last_click_time_ = millis();
}

void Move::reset_move() {
  direction_ = Direction::None;
}

void Move::task() {
  // No move to do?
  if(direction_ == Direction::None)
    return;

  // No click for too long time?
  if(ELAPSED(millis(), last_click_time_ + CLICK_DELAY)) {
    reset_move();
    return;
  }

  // Not soon end of move?
  if(!ELAPSED(millis(), target_move_time_ - 20))
    return;

  float min = get_min();
  float max = get_max();
  float offset = get_offset();
  feedRate_t feedrate = get_feedrate();

  target_position_ += offset;
  target_position_ = (target_position_ < min) ? min : (target_position_ > max) ? max : target_position_;

  setPosition();

  target_move_time_ += move_delay(offset, feedrate);
}

//! Move the nozzle (+X)
void Move::x_plus_command() {
  move(Direction::X_PLUS);
}

//! Move the nozzle (-X)
void Move::x_minus_command() {
  move(Direction::X_MINUS);
}

//! Move the nozzle (+Y)
void Move::y_plus_command() {
  move(Direction::Y_PLUS);
}

//! Move the nozzle (-Y)
void Move::y_minus_command() {
  move(Direction::Y_MINUS);
}

//! Move the nozzle (+Z)
void Move::z_plus_command() {
  move(Direction::Z_PLUS);
}

//! Move the nozzle (-Z)
void Move::z_minus_command() {
  move(Direction::Z_MINUS);
}

//! Extrude some filament.
void Move::e_plus_command() {
  move(Direction::E_PLUS);
}

//! Unextrude some filament.
void Move::e_minus_command() {
  move(Direction::E_MINUS);
}

//! Disable the motors.
void Move::disable_motors_command() {
  reset_move();
  core.inject_commands(F("M84")); // Disable steppers
  ExtUI::setAllAxisUnhomed();
  ExtUI::setAllAxisPositionUnknown();
}

//! Go to home on the X axis.
void Move::x_home_command() {
  reset_move();
  core.inject_commands(F("G28 X F6000"));
}

//! Go to home on the Y axis.
void Move::y_home_command() {
  reset_move();
  core.inject_commands(F("G28 Y F6000"));
}

//! Go to home on the Z axis.
void Move::z_home_command() {
  reset_move();
  core.inject_commands(F("G28 Z F1200"));
}

//! Go to home on all axis.
void Move::all_home_command() {
  reset_move();
  core.inject_commands(F("G28 F6000"));
}

}
