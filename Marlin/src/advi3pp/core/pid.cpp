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

#include "../parameters.h"
#include "logging.h"
#include "pid.h"


namespace ADVi3pp {

Pid pid;


//! Store current data in permanent memory (EEPROM)
//! @param eeprom EEPROM writer
void Pid::do_write(EepromWrite& eeprom) const
{
  for(size_t i = 0; i < NB_PIDs; ++i)
  {
    eeprom.write(bed_pid_[i]);
    eeprom.write(hotend_pid_[i]);
  }
}

//! Validate data from permanent memory (EEPROM).
//! @param eeprom EEPROM reader
bool Pid::do_validate(EepromRead &eeprom)
{
  PidValue pid{};
  for(size_t i = 0; i < NB_PIDs; ++i)
  {
    eeprom.read(pid);
    eeprom.read(pid);
  }
  return true;
}

//! Restore data from permanent memory (EEPROM).
//! @param eeprom EEPROM reader
void Pid::do_read(EepromRead& eeprom)
{
  for(size_t i = 0; i < NB_PIDs; ++i)
  {
    eeprom.read(bed_pid_[i]);
    eeprom.read(hotend_pid_[i]);
  }
}

//! Reset settings
void Pid::do_reset()
{
  for(size_t i = 0; i < NB_PIDs; ++i)
  {
    bed_pid_[i].temperature_ = default_bed_temperature;
    bed_pid_[i].Kp_ = DEFAULT_bedKp;
    bed_pid_[i].Ki_ = DEFAULT_bedKi;
    bed_pid_[i].Kd_ = DEFAULT_bedKd;

    hotend_pid_[i].temperature_ = default_hotend_temperature;
    hotend_pid_[i].Kp_ = DEFAULT_Kp;
    hotend_pid_[i].Ki_ = DEFAULT_Ki;
    hotend_pid_[i].Kd_ = DEFAULT_Kd;
  }
}

//! Return the amount of data (in bytes) necessary to save settings in permanent memory (EEPROM).
//! @return Number of bytes
uint16_t Pid::do_size_of() const
{
  return NB_PIDs * 2 * sizeof(PidValue);
}


//! Record new PID values for a given temperature
//! @param kind Kind of PID values: Hotend or Bed
//! @param temperature Temperature for these PID values
void Pid::add_pid(TemperatureKind kind, uint16_t temperature)
{
  auto& pid = get_pids(kind);
  for(size_t i = 0; i < NB_PIDs; ++i)
  {
    if(temperature == pid[i].temperature_)
    {
      get_marlin_pid(kind, i);
      return;
    }
  }

  // Temperature not found, so move PIDs and forget the last one, set index to 0 and update values
  for(size_t i = NB_PIDs - 1; i > 0; --i)
    pid[i] = pid[i - 1];

  pid[0].temperature_ = temperature;
  get_marlin_pid(kind, 0);
}

//! Choose the best PID values for the given temperature
//! @param kind Kind of PID values: Hotend or Bed
//! @param temperature Temperature for these PID values
void Pid::choose_best_pid(TemperatureKind kind, uint16_t temperature)
{
  size_t index = 0;

  uint16_t best_difference = 500;
  const auto& pid = get_pids(kind);

  for(size_t i = 0; i < NB_PIDs; ++i)
  {
    auto difference = abs(temperature - pid[i].temperature_);
    if(difference < best_difference)
    {
      best_difference = difference;
      index = i;
    }
  }

  set_marlin_pid(kind, index);
}

  //! Set the current PID values from what is recorded
void Pid::set_marlin_pid(TemperatureKind kind, size_t index) const
{
  assert(kind <= TemperatureKind::Hotend);
  const PidValue& pid = get_pid(kind, index);

  if(kind == TemperatureKind::Hotend)
    ExtUI::setPIDValues(pid.Kp_, pid.Ki_, pid.Kd_, ExtUI::E0);
  else
    ExtUI::setBedPIDValues(pid.Kp_, pid.Ki_, pid.Kd_);

  Log::log() << F("Set") << (kind == TemperatureKind::Hotend ? F("Hotend") : F("Bed"))
             << F("PID") << index << F("for temperature") << pid.temperature_
             << F("P =") << pid.Kp_ << F("I =") << pid.Ki_ << F("D =") << pid.Kd_ << Log::endl();
}

//! Record the current PID values
void Pid::get_marlin_pid(TemperatureKind kind, size_t index)
{
  assert(kind <= TemperatureKind::Hotend);
  PidValue& pid = get_pid(kind, index);

  if(kind == TemperatureKind::Hotend) {
    pid.Kp_ = ExtUI::getPIDValues_Kp(ExtUI::E0);
    pid.Ki_ = ExtUI::getPIDValues_Ki(ExtUI::E0);
    pid.Kd_ = ExtUI::getPIDValues_Kd(ExtUI::E0);
  }
  else {
    pid.Kp_ = ExtUI::getBedPIDValues_Kp();
    pid.Ki_ = ExtUI::getBedPIDValues_Ki();
    pid.Kd_ = ExtUI::getBedPIDValues_Kd();
  }

  Log::log() << F("Get") << (kind == TemperatureKind::Hotend ? F("Hotend") : F("Bed"))
             << F("PID") << index << F("for temperature") << pid.temperature_
             << F("P =") << pid.Kp_ << F("I =") << pid.Ki_ << F("D =") << pid.Kd_ << Log::endl();

}


}