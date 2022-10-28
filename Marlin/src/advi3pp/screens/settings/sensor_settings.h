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

#pragma once

#include "../core/screen.h"

namespace ADVi3pp {

struct SensorPosition { int16_t x, y; };

//! Sensor Settings Page
struct SensorSettings: Screen<SensorSettings> {
#ifdef ADVi3PP_PROBE
  static constexpr Page PAGE =  Page::SensorSettings;
  static constexpr Action ACTION = Action::SensorSettings;
#else
  static constexpr Page PAGE = Page::NoSensor;
  static constexpr Action ACTION = Action::NoSensor;
#endif

private:
  void on_enter();

#ifdef ADVi3PP_PROBE
  bool on_dispatch(KeyValue value);
  void on_save_command();

  void previous_command();
  void next_command();
  void highspeed_command();
  void send_values() const;
  void send_name() const;
  void get_values();
  void send_highspeed_value() const;
#endif

private:
  uint16_t index_ = 0;
  bool highspeed_ = false;

  friend Parent;
};

extern SensorSettings sensor_settings;

}
