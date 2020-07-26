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
#include "../../core/string.h"
#include "../../core/dgus.h"
#include "sensor_settings.h"

namespace ADVi3pp {

SensorSettings sensor_settings;

#ifdef ADVi3PP_PROBE

namespace { const size_t NB_SENSOR_POSITIONS = 3; }

//! Get the name of a sensor holder
//! @param index Index of the holder
//! @return The name (in Flash memory) of the holder
const FlashChar* get_sensor_name(size_t index)
{
    // Note: F macro can be used only in a function, this is why this is coded like this
    auto your                = F("Your Sensor");
    auto teaching_tech_side  = F("Teaching Tech Left");
    auto advi3pp             = F("ADVi3++ Left");

#if defined(BLTOUCH)
    auto baseggio            = F("Indianagio Front");
    static const FlashChar* names[NB_SENSOR_POSITIONS + 1] = {your, baseggio, teaching_tech_side, advi3pp};
#elif defined(ADVi3PP_54)
    auto mark2               = F("Mark II");
    static const FlashChar* names[NB_SENSOR_POSITIONS + 1] = {your, mark2, teaching_tech_side, advi3pp};
#else
#error "ADVi3PP_PROBE is defined but the kind of probe is unknown"
#endif

    assert(index <= NB_SENSOR_POSITIONS);
    return names[index];
}

//! Default position of the sensor for the different holders
const SensorPosition SENSOR_POSITION[NB_SENSOR_POSITIONS] =
{
#if ENABLED(BLTOUCH)
    {  +150, -4270 },    // Baseggio/Indianagio Front
    { -2400, -3800 },    // Teaching Tech Left
    { -2800, -4000 }     // ADVi3++ Left
#elif defined(ADVi3PP_54)
    {     0,  6000 },    // Mark II
    { -2400, -3800 },    // Teaching Tech Left
    { -2800, -4000 }     // ADVi3++ Left
#else
#error "ADVi3PP_PROBE is defined but the kind of probe is unknown"
#endif
};
#endif

#ifdef ADVi3PP_PROBE

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
    index_ = 0;
    send_name();
    send_values();
    pages.save_forward_page();
    return Page::SensorSettings;
}

//! Handles the Save (Continue) command
void SensorSettings::do_save_command()
{
    get_values();
    Parent::do_save_command();
}


//! Show the previous settings.
void SensorSettings::previous_command()
{
    if(index_ <= 0)
        return;
    index_ -= 1;
    send_name();
    send_values();
}

//! Show the next settings.
void SensorSettings::next_command()
{
    if(index_ >= NB_SENSOR_POSITIONS) // not -1 because we have also index #0 not counted in NB_SENSOR_POSITIONS
        return;
    index_ += 1;
    send_name();
    send_values();
}

void SensorSettings::send_values() const
{
    WriteRamDataRequest frame{Variable::Value0};

    if(index_ == 0)
    {
        frame << Uint16{ExtUI::getProbeOffset_mm(ExtUI::X) * 100}
              << Uint16{ExtUI::getProbeOffset_mm(ExtUI::Y) * 100}
              << Uint16{ExtUI::getZOffset_mm() * 100};
    }
    else
    {
        frame << Uint16{SENSOR_POSITION[index_ - 1].x}
              << Uint16{SENSOR_POSITION[index_ - 1].y}
              << Uint16{ExtUI::getZOffset_mm() * 100};;
    }

    frame.send();
}

void SensorSettings::send_name() const
{
    ADVString<28> title{get_sensor_name(index_)};
    WriteRamDataRequest frame{Variable::LongTextCentered0};
    frame.center(title);
    frame.send();
}

//! Get current data from the LCD Panel.
void SensorSettings::get_values()
{
    ReadRamData frame{Variable::Value0, 3};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Sensor Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z;
    frame >> x >> y >> z;

    ExtUI::setProbeOffset_mm(static_cast<int16_t>(x.word) / 100.0, ExtUI::X);
    ExtUI::setProbeOffset_mm(static_cast<int16_t>(y.word) / 100.0, ExtUI::Y);
    ExtUI::setZOffset_mm(static_cast<int16_t>(z.word) / 100.0);
}

#else

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page SensorSettings::do_prepare_page()
{
    return Page::NoSensor;
}

#endif

}
