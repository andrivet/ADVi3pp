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

#include <stdint.h>
#include "settings.h"
#include "../lib/ADVstd/bitmasks.h"

namespace ADVi3pp {

struct PidValue
{
  float Kp_, Ki_, Kd_;
  uint16_t temperature_;
};

struct Pid: Settings<Pid> {
  static const size_t NB_PIDs = 3;

  PidValue& get_pid(TemperatureKind kind, size_t index) { return get_pids(kind)[index]; }
  const PidValue& get_pid(TemperatureKind kind, size_t index) const { return get_pids(kind)[index]; }
  void add_pid(TemperatureKind kind, uint16_t temperature);
  void choose_best_pid(TemperatureKind kind, uint16_t temperature);
  void set_marlin_pid(TemperatureKind kind, size_t index) const;
  void get_marlin_pid(TemperatureKind kind, size_t index);

private:
  friend Parent;

  void do_write(EepromWrite& eeprom) const;
  bool do_validate(EepromRead& eeprom);
  void do_read(EepromRead& eeprom);
  void do_reset();
  uint16_t do_size_of() const;

  adv::array<PidValue, Pid::NB_PIDs>& get_pids(TemperatureKind kind) { return (kind == TemperatureKind::Hotend) ? hotend_pid_ : bed_pid_; }
  const adv::array<PidValue, Pid::NB_PIDs>& get_pids(TemperatureKind kind) const { return (kind == TemperatureKind::Hotend) ? hotend_pid_ : bed_pid_; }

private:
  adv::array<PidValue, NB_PIDs> hotend_pid_{};
  adv::array<PidValue, NB_PIDs> bed_pid_{};
};

extern Pid pid;

}