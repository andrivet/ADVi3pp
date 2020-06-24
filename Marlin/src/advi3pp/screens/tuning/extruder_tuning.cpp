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
#include "extruder_tuning.h"
#include "../../core/core.h"
#include "../../core/dgus.h"
#include "../core/wait.h"
#include "../settings/step_settings.h"

namespace ADVi3pp {

ExtruderTuning extruder_tuning;

//! Handle Extruder Tuning command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool ExtruderTuning::do_dispatch(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::TuningStart:     start_command(); return true;
        case KeyValue::TuningSettings:  settings_command(); return true;
        default:                        break;
    }

    // Do this after since we handle Save before
    if(Parent::do_dispatch(key_value))
        return true;

    return false;
}

void ExtruderTuning::send_data()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(settings.get_last_used_temperature(TemperatureKind::Hotend));
    frame.send();
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page ExtruderTuning::do_prepare_page()
{
    if(!core.ensure_not_printing())
        return Page::None;
    send_data();
    return Page::ExtruderTuningTemp;
}

//! Start extruder tuning.
void ExtruderTuning::start_command()
{
    ReadRamData frame{Variable::Value0, 1};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Target Temperature)") << Log::endl();
        return;
    }

    Uint16 temperature; frame >> temperature;
    wait.show(F("Heating the extruder..."), WaitCallback{this, &ExtruderTuning::cancel}, ShowOptions::None);
    ExtUI::setTargetTemp_celsius(temperature.word, ExtUI::E0);

    task.set_background_task(BackgroundTask(this, &ExtruderTuning::heating_task));
}

//! Extruder tuning background task.
void ExtruderTuning::heating_task()
{
    if(ExtUI::getActualTemp_celsius(ExtUI::E0) < ExtUI::getTargetTemp_celsius(ExtUI::E0) - 10)
        return;
    task.clear_background_task();

    wait.set_message(F("Wait until the extrusion is finished..."));
    core.inject_commands(F("G1 Z20 F1200\nM83\nG92 E0\nG1 E100 F50"));   // raise head, relative E mode, reset E axis, Extrude slowly

    task.set_background_task(BackgroundTask(this, &ExtruderTuning::extruding_task));
}

//! Extruder tuning background task.
void ExtruderTuning::extruding_task()
{
    if(ExtUI::getAxisPosition_mm(ExtUI::E0) < tuning_extruder_filament || core.is_busy())
        return;
    task.clear_background_task();

    extruded_ = ExtUI::getAxisPosition_mm(ExtUI::E0);

    ExtUI::setTargetTemp_celsius(0, ExtUI::E0);
    task.clear_background_task();
    finished();
}

//! Record the amount of filament extruded.
void ExtruderTuning::finished()
{
    Log::log() << F("Filament extruded ") << extruded_ << Log::endl();
    core.inject_commands(F("M82\nG92 E0"));       // absolute E mode, reset E axis

    task.clear_background_task();

    // Always set to default 20mm
    WriteRamDataRequest frame{Variable::Value0};
    frame << 200_u16; // 20.0
    frame.send();

    pages.show_page(Page::ExtruderTuningMeasure, ShowOptions::None);
}

//! Cancel Extruder tuning process.
bool ExtruderTuning::cancel()
{
    ExtUI::cancelWaitForHeatup();
    task.clear_background_task();
    ExtUI::setTargetTemp_celsius(0, ExtUI::E0);
    return false;
}

//! Execute the Back command
void ExtruderTuning::do_back_command()
{
    task.clear_background_task();

    ExtUI::setTargetTemp_celsius(0, ExtUI::E0);
    core.inject_commands(F("M82\nG92 E0"));       // absolute E mode, reset E axis

    Parent::do_back_command();
}

//! Compute the extruder (E axis) new value and show the steps settings.
void ExtruderTuning::settings_command()
{
    ReadRamData frame{Variable::Value0, 1};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Measures)") << Log::endl();
        return;
    }

    Uint16 e; frame >> e;
    auto new_value = ExtUI::getAxisSteps_per_mm(ExtUI::E0) * extruded_ / (extruded_ + tuning_extruder_delta - e.word / 10.0);

    Log::log()
            << F("Adjust: old = ") << ExtUI::getAxisSteps_per_mm(ExtUI::E0)
            << F(", expected = ") << extruded_
            << F(", measured = ") << (extruded_ + tuning_extruder_delta - e.word)
            << F(", new = ") << new_value << Log::endl();

    ExtUI::setAxisSteps_per_mm(new_value, ExtUI::E0);
    steps_settings.show(ShowOptions::None);
}

}
