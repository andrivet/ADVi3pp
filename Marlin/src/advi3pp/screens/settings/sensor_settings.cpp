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

#include "../../parameters.h"
#include "sensor_settings.h"

namespace ADVi3pp {

SensorSettings sensor_settings;

#ifdef ADVi3PP_PROBE
//! Default position of the sensor for the different holders
const SensorPosition DEFAULT_SENSOR_POSITION[SensorSettings::NB_SENSOR_POSITIONS] =
{
#if defined(ADVi3PP_MARK2)
        {     0,  6000 },    // Mark II
        { -2400, -3800 },    // Teaching Tech L. Side
        {     0,     0 }     // Custom
#elif defined(ADVi3PP_BLTOUCH)
        {  +150, -4270 },    // Baseggio/Indianagio Front
        { -2400, -3800 },    // Teaching Tech L. Side
        {     0,     0 }     // Custom
#endif
};
#endif

#ifdef ADVi3PP_PROBE

//! Constructor
SensorSettings::SensorSettings()
{
    do_reset();
}

//! Handle Sensor Settings command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool SensorSettings::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::SensorSettingsPrevious:  previous_command(); break;
        case KeyValue::SensorSettingsNext:      next_command(); break;
        default:                                return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page SensorSettings::do_prepare_page()
{
    send_data();
    pages.save_forward_page();
    return Page::SensorSettings;
}

//! Handles the Save (Continue) command
void SensorSettings::do_save_command()
{
    get_data();
    Parent::do_save_command();
}


//! Show the previous settings.
void SensorSettings::previous_command()
{
    if(index_ <= 0)
        return;
    get_data();
    index_ -= 1;
    send_data();
}

//! Show the next settings.
void SensorSettings::next_command()
{
    if(index_ >= NB_SENSOR_POSITIONS - 1)
        return;
    get_data();
    index_ += 1;
    send_data();
}

//! Send current data to the LCD Panel.
void SensorSettings::send_data() const
{
    ADVString<32> title{get_sensor_name(index_)};

    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(positions_[index_].x) << Uint16(positions_[index_].y) << Uint16(zprobe_zoffset * 100);
    frame.send();

    frame.reset(Variable::LongTextCentered0);
    frame << title.align(Alignment::Center);
    frame.send();
}

//! Send current data from the LCD Panel.
void SensorSettings::get_data()
{
    ReadRamData frame{Variable::Value0, 3};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Sensor Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z;
    frame >> x >> y >> z;

    positions_[index_].x = static_cast<int16_t>(x.word);
    positions_[index_].y = static_cast<int16_t>(y.word);
    zprobe_zoffset = static_cast<int16_t>(z.word) / 100.0;
}

//! Get the current offset of the nozzle (depending of the sensor holder).
//! @return The offset between the nozzle and the sensor.
int SensorSettings::x_probe_offset_from_extruder() const
{
    return static_cast<int>(positions_[index_].x / 100.0 + 0.5); // 0.5 for rounding
}

//! Get the current offset of the nozzle (depending of the sensor holder).
//! @return The offset between the nozzle and the sensor.
int SensorSettings::y_probe_offset_from_extruder() const
{
    return static_cast<int>(positions_[index_].y / 100.0 + 0.5); // 0.5 for rounding
}

//! Get the position of the bed (depending of the sensor holder).
//! @return The position reachable (left).
int SensorSettings::left_probe_bed_position()
{
    return max(X_MIN_BED + MIN_PROBE_EDGE, X_MIN_POS + x_probe_offset_from_extruder());
}

//! Get the position of the bed (depending of the sensor holder).
//! @return The position reachable (right).
int SensorSettings::right_probe_bed_position()
{
    return min(X_MAX_BED - MIN_PROBE_EDGE, X_MAX_POS + x_probe_offset_from_extruder());
}

//! Get the position of the bed (depending of the sensor holder).
//! @return The position reachable (front).
int SensorSettings::front_probe_bed_position()
{
    return max(Y_MIN_BED + MIN_PROBE_EDGE, Y_MIN_POS + y_probe_offset_from_extruder());
}

//! Get the position of the bed (depending of the sensor holder).
//! @return The position reachable (bottom).
int SensorSettings::back_probe_bed_position()
{
    return min(Y_MAX_BED - MIN_PROBE_EDGE, Y_MAX_POS + y_probe_offset_from_extruder());
}

//! Reset settings
void SensorSettings::do_reset()
{
    index_ = 0;
    for(size_t i = 0; i < NB_SENSOR_POSITIONS; ++i)
        positions_[i] = DEFAULT_SENSOR_POSITION[i];
}

#else

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page SensorSettings::do_prepare_page()
{
    return Page::NoSensor;
}

#endif

//! Store current data in permanent memory (EEPROM)
//! @param eeprom EEPROM writer
void SensorSettings::do_write(EepromWrite& eeprom) const
{
    eeprom.write(index_);
    for(size_t i = 0; i < NB_SENSOR_POSITIONS; ++i)
    {
        eeprom.write(positions_[i].x);
        eeprom.write(positions_[i].y);
    }
}

//! Restore data from permanent memory (EEPROM).
//! @param eeprom EEPROM reader
void SensorSettings::do_read(EepromRead& eeprom)
{
    eeprom.read(index_);
    for(size_t i = 0; i < NB_SENSOR_POSITIONS; ++i)
    {
        eeprom.read(positions_[i].x);
        eeprom.read(positions_[i].y);
    }
}

//! Reset settings
void SensorSettings::do_reset()
{
    index_ = 0;
    for(size_t i = 0; i < NB_SENSOR_POSITIONS; ++i)
        positions_[i] = SensorPosition{};
}

//! Return the amount of data (in bytes) necessary to save settings in permanent memory (EEPROM).
//! @return Number of bytes
uint16_t SensorSettings::do_size_of() const
{
    return sizeof(index_) + NB_SENSOR_POSITIONS * sizeof(SensorPosition);
}


}
