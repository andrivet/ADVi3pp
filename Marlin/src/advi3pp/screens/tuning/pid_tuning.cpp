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
#include "pid_tuning.h"
#include "../../core/core.h"
#include "../../core/dgus.h"
#include "../../core/status.h"
#include "../settings/pid_settings.h"
#include "../print/temperatures.h"

namespace ADVi3pp {

PidTuning pid_tuning;

//! Handle PID tuning command
//! @param key_value    The step of the PID tuning
//! @return             True if the action was handled
bool PidTuning::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::PidTuningStep2:  step2_command(); break;
        case KeyValue::PidTuningHotend: hotend_command(); break;
        case KeyValue::PidTuningBed:    bed_command(); break;
        default:                        return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page PidTuning::do_prepare_page()
{
    if(!core.ensure_not_printing())
        return Page::None;
    pages.save_forward_page();
    hotend_command();
    status.reset();
    return Page::PidTuning;
}

//! Send the current data to the LCD panel.
void PidTuning::send_data()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(temperature_)
          << Uint16(kind_ != TemperatureKind::Hotend);
    frame.send();
}

//! Select the hotend PID
void PidTuning::hotend_command()
{
    temperature_ = settings.get_last_used_temperature(TemperatureKind::Hotend);
    kind_ = TemperatureKind::Hotend;
    send_data();
}

//! Select the bed PID
void PidTuning::bed_command()
{
    temperature_ = settings.get_last_used_temperature(TemperatureKind::Bed);
    kind_ = TemperatureKind::Bed;
    send_data();
}

//! Show step #2 of PID tuning
void PidTuning::step2_command()
{
    status.reset();

    ReadRamData frame{Variable::Value0, 2};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Target Temperature)") << Log::endl();
        return;
    }

    Uint16 temperature, kind; frame >> temperature >> kind; temperature_ = temperature.word; // kind is not used here, it is already set
    if(kind_ == TemperatureKind::Hotend)
        ExtUI::injectCommands_P(PSTR("M106 S255")); // Turn on fan (only for hotend)

    ADVString<20> auto_pid_command;
    auto_pid_command << F("M303 S") << temperature_
                     << (kind_ == TemperatureKind::Hotend ? F(" E0 U1") : F(" E-1 U1"));
    ExtUI::injectCommands_P(auto_pid_command.get());

    inTuning_ = true;
    temperatures.show(WaitCallback{this, &PidTuning::cancel_pid});
}

//! Cancel PID process.
bool PidTuning::cancel_pid()
{
    ExtUI::cancelWaitForHeatup();
    inTuning_ = false;
    return false;
}

//! PID automatic tuning is finished.
void PidTuning::finished(bool success)
{
    Log::log() << F("Auto PID finished: ") << (success ? F("success") : F("failed")) << Log::endl();
    ExtUI::injectCommands_P(PSTR("M106 S0"));
    if(!success)
    {
        status.set(F("PID tuning failed"));
        return;
    }

    status.reset();
    pid_settings.add_pid(kind_, temperature_);
    bool inTuning = inTuning_;
    inTuning_ = false;
    pid_settings.show(inTuning ? ShowOptions::None : ShowOptions::SaveBack);
}


}
