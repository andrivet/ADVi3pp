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
    from_lcd();
    kind_ = TemperatureKind::Hotend;
    to_lcd();
}

//! Handle the select Bed PID command
void PidSettings::bed_command()
{
    from_lcd();
    kind_ = TemperatureKind::Bed;
    to_lcd();
}

//! Handle the show previous PID values command
void PidSettings::previous_command()
{
    if(index_ <= 0)
        return;
    from_lcd();
    index_ -= 1;
    to_lcd();
}

//! Handle the show next PID values command
void PidSettings::next_command()
{
    if(index_ >= NB_PIDs - 1)
        return;
    from_lcd();
    index_ += 1;
    to_lcd();
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

//! Validate data from permanent memory (EEPROM).
//! @param eeprom EEPROM reader
bool PidSettings::do_validate(EepromRead &eeprom)
{
    Pid pid{};
    for(size_t i = 0; i < NB_PIDs; ++i)
    {
        eeprom.read(pid);
        eeprom.read(pid);
    }
    return true;
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

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page PidSettings::do_prepare_page()
{
    to_lcd();
    return Page::PidSettings;
}

//! Save the PID settings
void PidSettings::do_save_command()
{
    from_lcd();
    assert(kind_ <= TemperatureKind::Hotend);
    set_marlin_pid();
    Parent::do_save_command();
}

//! Execute the Back command
void PidSettings::do_back_command()
{
    settings.restore();
    Parent::do_back_command();
}

//! Set the current PID values from what is recorded
void PidSettings::set_marlin_pid() const
{
    assert(kind_ <= TemperatureKind::Hotend);
    const Pid& pid = get_pid();

    if(kind_ == TemperatureKind::Hotend)
        ExtUI::setPIDValues(pid.Kp_, pid.Ki_, pid.Kd_, ExtUI::E0);
    else
        ExtUI::setBedPIDValues(pid.Kp_, pid.Ki_, pid.Kd_);

    Log::log() << F("Set") << (kind_ == TemperatureKind::Hotend ? F("Hotend") : F("Bed"))
               << F("PID") << index_ << F("for temperature") << pid.temperature_
               << F("P =") << pid.Kp_ << F("I =") << pid.Ki_ << F("D =") << pid.Kd_ << Log::endl();
}

//! Record the current PID values
void PidSettings::get_marlin_pid()
{
    assert(kind_ <= TemperatureKind::Hotend);
    Pid& pid = get_pid();

    if(kind_ == TemperatureKind::Hotend) {
        pid.Kp_ = ExtUI::getPIDValues_Kp(ExtUI::E0);
        pid.Ki_ = ExtUI::getPIDValues_Ki(ExtUI::E0);
        pid.Kd_ = ExtUI::getPIDValues_Kd(ExtUI::E0);
    }
    else {
        pid.Kp_ = ExtUI::getBedPIDValues_Kp();
        pid.Ki_ = ExtUI::getBedPIDValues_Ki();
        pid.Kd_ = ExtUI::getBedPIDValues_Kd();
    }

    Log::log() << F("Get") << (kind_ == TemperatureKind::Hotend ? F("Hotend") : F("Bed"))
               << F("PID") << index_ << F("for temperature") << pid.temperature_
               << F("P =") << pid.Kp_ << F("I =") << pid.Ki_ << F("D =") << pid.Kd_ << Log::endl();

}

adv::array<Pid, PidSettings::NB_PIDs>& PidSettings::get_pids()
{
    assert(kind_ <= TemperatureKind::Hotend);
    return (kind_ == TemperatureKind::Hotend) ? hotend_pid_ : bed_pid_;
}

const adv::array<Pid, PidSettings::NB_PIDs>& PidSettings::get_pids() const
{
    assert(kind_ <= TemperatureKind::Hotend);
    return (kind_ == TemperatureKind::Hotend) ? hotend_pid_ : bed_pid_;
}

Pid& PidSettings::get_pid() {
    return get_pids()[index_];
}

const Pid& PidSettings::get_pid() const {
    return get_pids()[index_];
}

//! Record new PID values for a given temperature
//! @param kind Kind of PID values: Hotend or Bed
//! @param temperature Temperature for these PID values
void PidSettings::add_pid(TemperatureKind kind, uint16_t temperature)
{
    kind_ = kind;
    auto& pid = get_pids();
    for(size_t i = 0; i < NB_PIDs; ++i)
    {
        if(temperature == pid[i].temperature_)
        {
            Log::log() << (kind == TemperatureKind::Bed ? F("Bed") : F("Hotend"))
                       << F("PID with temperature") << temperature << F("found, update settings") << Log::endl();
            index_ = i;
            get_marlin_pid();
            return;
        }
    }

    Log::log() << (kind == TemperatureKind::Bed ? F("Bed") : F("Hotend"))
               << F("PID with temperature") << temperature << F("NOT found, update settings #0") << Log::endl();
    // Temperature not found, so move PIDs and forget the last one, set index to 0 and update values
    for(size_t i = NB_PIDs - 1; i > 0; --i)
        pid[i] = pid[i - 1];
    index_ = 0;
    pid[0].temperature_ = temperature;
    get_marlin_pid();
}

//! Choose the best PID values for the given temperature
//! @param kind Kind of PID values: Hotend or Bed
//! @param temperature Temperature for these PID values
void PidSettings::choose_best_pid(TemperatureKind kind, uint16_t temperature)
{
    index_ = 0;
    kind_ = kind;

    uint16_t best_difference = 500;
    const auto& pid = get_pids();

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
               << F("PID with smallest difference (") << best_difference << F(") is at index #") << index_ << Log::endl();
    set_marlin_pid();
}

//! Send the current data to the LCD panel.
void PidSettings::to_lcd() const
{
    const Pid& pid = get_pid();
    Log::log() << F("Send") << (kind_ == TemperatureKind::Bed ? F("Bed") : F("Hotend"))
               << F(" PID") << index_
               << F("P =") << pid.Kp_ << F("I =") << pid.Ki_ << F("D =") << pid.Kd_ << Log::endl();

    WriteRamRequest{Variable::Value0}.write_words(adv::array<uint16_t, 5>
    {
        static_cast<uint16_t>(kind_ == TemperatureKind::Hotend ? 0u : 1u),
        pid.temperature_,
        static_cast<uint16_t>(pid.Kp_ * 100),
        static_cast<uint16_t>(pid.Ki_ * 100),
        static_cast<uint16_t>(pid.Kd_ * 100)
    });

    ADVString<8> indexes;
    indexes << index_ + 1 << F(" / ") << NB_PIDs;
    WriteRamRequest{Variable::ShortText0}.write_text(indexes);
}

//! Save the settings from the LCD Panel.
void PidSettings::from_lcd()
{
    ReadRam response{Variable::Value0};
    if(!response.send_receive(5))
    {
        Log::error() << F("Receiving Frame (PID Settings)") << Log::endl();
        return;
    }

    uint16_t kind = response.read_word();
    uint16_t temperature = response.read_word();
    uint16_t p = response.read_word();
    uint16_t i = response.read_word();
    uint16_t d = response.read_word();

    kind_ = kind ? TemperatureKind::Bed : TemperatureKind::Hotend;
    Pid& pid = get_pid();

    pid.Kp_ = static_cast<float>(p) / 100;
    pid.Ki_ = static_cast<float>(i) / 100;
    pid.Kd_ = static_cast<float>(d) / 100;
    pid.temperature_ = temperature;

    set_marlin_pid();
}

}
