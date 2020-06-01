/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2020 Sebastien Andrivet [https://github.com/andrivet/]
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

#include "pid_settings.h"
#include "../../core/dgus.h"
#include "../tuning/pid_tuning.h"

namespace ADVi3pp {

PidSettings pid_settings;


PidSettings::PidSettings()
{
    do_reset();
}

//! Handle PID Settings command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool PidSettings::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::PidSettingsHotend:   hotend_command(); break;
        case KeyValue::PidSettingsBed:      bed_command(); break;
        case KeyValue::PidSettingPrevious:  previous_command(); break;
        case KeyValue::PidSettingNext:      next_command(); break;
        default:                            return false;
    }

    return true;
}

//! Handle the select Hotend PID command
void PidSettings::hotend_command()
{
    save_data();
    kind_ = TemperatureKind::Hotend;
    send_data();
}

//! Handle the select Bed PID command
void PidSettings::bed_command()
{
    save_data();
    kind_ = TemperatureKind::Bed;
    send_data();
}

//! Handle the show previous PID values command
void PidSettings::previous_command()
{
    if(index_ <= 0)
        return;
    save_data();
    index_ -= 1;
    send_data();
}

//! Handle the show next PID values command
void PidSettings::next_command()
{
    if(index_ >= NB_PIDs - 1)
        return;
    save_data();
    index_ += 1;
    send_data();
}

//! Store current data in permanent memory (EEPROM)
//! @param eeprom EEPROM writer
void PidSettings::do_write(EepromWrite& eeprom) const
{
    for(size_t i = 0; i < NB_PIDs; ++i)
    {
        eeprom.write(bed_pid_[i]);
        eeprom.write(hotend_pid_[i]);
    }
}

//! Restore data from permanent memory (EEPROM).
//! @param eeprom EEPROM reader
void PidSettings::do_read(EepromRead& eeprom)
{
    for(size_t i = 0; i < NB_PIDs; ++i)
    {
        eeprom.read(bed_pid_[i]);
        eeprom.read(hotend_pid_[i]);
    }
}

//! Reset settings
void PidSettings::do_reset()
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
uint16_t PidSettings::do_size_of() const
{
    return NB_PIDs * 2 * sizeof(Pid);
}

//! Set the current PID values from what is recorded
void PidSettings::set_current_pid() const
{
    switch(kind_)
    {
        case TemperatureKind::Hotend:  set_current_hotend_pid(); break;
        case TemperatureKind::Bed:     set_current_bed_pid(); break;
        default: assert("Invalid temperature Kind");
    }
}

void PidSettings::set_current_hotend_pid() const
{
    const Pid& pid = hotend_pid_[index_];
    ExtUI::setPIDValues(pid.Kp_, pid.Ki_, pid.Kd_, ExtUI::E0);

    Log::log() << F("Set Hotend PID #") << index_ << F(" for temperature ") << pid.temperature_
               << F(", P = ") << pid.Kp_ << F(", I = ") << pid.Ki_ << F(", D = ") << pid.Kd_ << Log::endl();
}

void PidSettings::set_current_bed_pid() const
{
    const Pid& pid = bed_pid_[index_];
    ExtUI::setBedPIDValues(pid.Kp_, pid.Ki_, pid.Kd_);

    Log::log() << F("Set Bed PID #") << index_ << F(" for temperature ") << pid.temperature_
               << F(", P = ") << pid.Kp_ << F(", I = ") << pid.Ki_ << F(", D = ") << pid.Kd_ << Log::endl();
}

//! Record the current PID values
void PidSettings::get_current_pid()
{
    switch(kind_)
    {
        case TemperatureKind::Hotend:  get_current_hotend_pid(); break;
        case TemperatureKind::Bed:     get_current_bed_pid(); break;
        default: assert("Invalid temperature Kind");
    }
}

Pid* PidSettings::get_pid(TemperatureKind kind)
{
    switch(kind_)
    {
        case TemperatureKind::Hotend:  return hotend_pid_;
        case TemperatureKind::Bed:     return bed_pid_;
        default: assert("Invalid temperature Kind"); return hotend_pid_;
    }
}

const Pid* PidSettings::get_pid(TemperatureKind kind) const
{
    switch(kind_)
    {
        case TemperatureKind::Hotend:  return hotend_pid_;
        case TemperatureKind::Bed:     return bed_pid_;
        default: assert("Invalid temperature Kind"); return hotend_pid_;
    }
}

//! Record the current PID values
void PidSettings::get_current_hotend_pid()
{
    Pid& pid = hotend_pid_[index_];

    pid.Kp_ = ExtUI::getPIDValues_Kp(ExtUI::E0);
    pid.Ki_ = ExtUI::getPIDValues_Ki(ExtUI::E0);
    pid.Kd_ = ExtUI::getPIDValues_Kd(ExtUI::E0);

    Log::log() << F("Get Hotend PID #") << index_
               << F(", P = ") << pid.Kp_ << F(", I = ") << pid.Ki_ << F(", D = ") << pid.Kd_ << Log::endl();
}

//! Record the current PID values
void PidSettings::get_current_bed_pid()
{
    Pid& pid = bed_pid_[index_];

    pid.Kp_ = ExtUI::getBedPIDValues_Kp();
    pid.Ki_ = ExtUI::getBedPIDValues_Ki();
    pid.Kd_ = ExtUI::getBedPIDValues_Kd();

    Log::log() << F("Get Hotend PID #") << index_
               << F(", P = ") << pid.Kp_ << F(", I = ") << pid.Ki_ << F(", D = ") << pid.Kd_ << Log::endl();
}

//! Record new PID values for a given temperature
//! @param kind Kind of PID values: Hotend or Bed
//! @param temperature Temperature for these PID values
void PidSettings::add_pid(TemperatureKind kind, uint16_t temperature)
{
    kind_ = kind;
    Pid* pid = get_pid(kind_);
    for(size_t i = 0; i < NB_PIDs; ++i)
    {
        if(temperature == pid[i].temperature_)
        {
            Log::log() << (kind == TemperatureKind::Bed ? F("Bed") : F("Hotend"))
                       << F(" PID with temperature 0x") << temperature << F(" found, update settings") << Log::endl();
            index_ = i;
            get_current_pid();
            return;
        }
    }

    Log::log() << (kind == TemperatureKind::Bed ? F("Bed") : F("Hotend"))
               << F(" PID with temperature 0x") << temperature << F(" NOT found, update settings #0") << Log::endl();
    // Temperature not found, so move PIDs and forget the last one, set index to 0 and update values
    for(size_t i = NB_PIDs - 1; i > 0; --i)
        pid[i] = pid[i - 1];
    index_ = 0;
    pid[0].temperature_ = temperature;
    get_current_pid();
}

//! Choose the best PID values for the given temperature
//! @param kind Kind of PID values: Hotend or Bed
//! @param temperature Temperature for these PID values
void PidSettings::set_best_pid(TemperatureKind kind, uint16_t temperature)
{
    index_ = 0;
    kind_ = kind;

    uint16_t best_difference = 500;
    Pid* pid = get_pid(kind_);

    for(size_t i = 0; i < NB_PIDs; ++i)
    {
        auto difference = abs(temperature - pid[i].temperature_);
        if(difference < best_difference)
        {
            best_difference = difference;
            index_ = i;
        }
    }

    Log::log() << (kind_ == TemperatureKind::Bed ? F("Bed") : F("Hotend"))
               << F(" PID with smallest difference (") << best_difference << F(") is at index #") << index_ << Log::endl();
    set_current_pid();
}

//! Send the current data to the LCD panel.
void PidSettings::send_data() const
{
    const Pid& pid = get_pid(kind_)[index_];
    Log::log() << F("Send ") << (kind_ == TemperatureKind::Bed ? F("Bed") : F("Hotend")) << F(" PID #") << index_
               << F(", P = ") << pid.Kp_ << F(", I = ") << pid.Ki_ << F(", D = ") << pid.Kd_ << Log::endl();

    WriteRamDataRequest frame{Variable::Value0};
    frame << (kind_ == TemperatureKind::Hotend ? 0_u16 : 1_u16)
          << Uint16(pid.temperature_)
          << Uint16(pid.Kp_ * 100)
          << Uint16(pid.Ki_ * 100)
          << Uint16(pid.Kd_ * 100);
    frame.send();

    ADVString<8> indexes;
    indexes << index_ + 1 << F(" / ") << NB_PIDs;
    frame.reset(Variable::ShortText0);
    frame << indexes;
    frame.send();
}

//! Save the settings from the LCD Panel.
void PidSettings::save_data()
{
    Pid& pid = get_pid(kind_)[index_];

    ReadRamData response{Variable::Value0, 5};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (PID Settings)") << Log::endl();
        return;
    }

    Uint16 kind, temperature, p, i, d;
    response >> kind >> temperature >> p >> i >> d;

    assert(kind.word == (kind_ == TemperatureKind::Hotend ? 0 : 1));

    pid.Kp_ = static_cast<float>(p.word) / 100;
    pid.Ki_ = static_cast<float>(i.word) / 100;
    pid.Kd_ = static_cast<float>(d.word) / 100;

    set_current_pid();
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page PidSettings::do_prepare_page()
{
    send_data();
    return Page::PidSettings;
}

//! Save the PID settings
void PidSettings::do_save_command()
{
    save_data();

    switch(kind_)
    {
        case TemperatureKind::Hotend:  save_hotend_pid(); break;
        case TemperatureKind::Bed:     save_bed_pid(); break;
        default: assert("Invalid temperature Kind");
    }

    Parent::do_save_command();
}

//! Execute the Back command
void PidSettings::do_back_command()
{
    settings.restore();
    pid_tuning.send_data();
    Parent::do_back_command();
}

void PidSettings::save_hotend_pid() const
{
    auto pid = get_pid(kind_)[index_];
    ExtUI::setPIDValues(pid.Kp_, pid.Ki_, pid.Kd_, ExtUI::E0);
}

void PidSettings::save_bed_pid() const
{
    auto pid = get_pid(kind_)[index_];
    ExtUI::setBedPIDValues(pid.Kp_, pid.Ki_, pid.Kd_);
}

}
