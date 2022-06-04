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
    WriteRamRequest{Variable::Value0}.write_words(temperature_, kind_ != TemperatureKind::Hotend);
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

    ReadRam frame{Variable::Value0};
    if(!frame.send_receive(2))
    {
        Log::error() << F("Receiving Frame (Target Temperature)") << Log::endl();
        return;
    }

    temperature_ = frame.read_word();
    [[maybe_unused]] uint16_t  kind = frame.read_word();
    assert(static_cast<TemperatureKind>(kind) == kind_);

    state_ |= State::FromLCDMenu;

    if(kind_ == TemperatureKind::Hotend)
        ExtUI::setTargetFan_percent(100, ExtUI::FAN0); // Turn on fan (only for hotend)

    background_task.set(Callback{this, &PidTuning::step3_command});
}

void PidTuning::step3_command() {
    background_task.clear();

    temperatures.show(Callback{this, &PidTuning::cancel_pid});

    if(kind_ == TemperatureKind::Hotend)
        ExtUI::startPIDTune(temperature_, ExtUI::E0);
    else
        ExtUI::startBedPIDTune(temperature_);
}

//! Cancel PID process.
void PidTuning::cancel_pid()
{
    status.set(F("Canceling PID tuning"));
    ExtUI::cancelWaitForHeatup();
    ExtUI::setTargetFan_percent(0, ExtUI::FAN0);
    state_ = State::None;
}

void PidTuning::on_start() {
    status.set(F("Starting PID tuning..."));
    state_ |= State::Processing;
}

void PidTuning::on_progress(int cycle, int nb)
{
    ADVString<18> format{F("PID tuning %i / %i")};
    ADVString<18> progress{};
    progress.format(format.get(), cycle, nb);
    status.set(progress.get());
}

const FlashChar* get_message(ExtUI::result_t result)
{
    switch(result)
    {
        case ExtUI::PID_BAD_EXTRUDER_NUM:   return F(STR_PID_BAD_HEATER_ID);
        case ExtUI::PID_TEMP_TOO_HIGH:      return F(STR_PID_TEMP_TOO_HIGH);
        case ExtUI::PID_TUNING_TIMEOUT:     return F(STR_PID_TIMEOUT);
        case ExtUI::PID_DONE:               return F("PID tuning successful");
        default: Log::error() << F("Unknown result_t ") << static_cast<uint16_t>(result) << Log::endl(); break;
    }

    return F("");
}

//! PID automatic tuning is finished.
void PidTuning::on_finished(ExtUI::result_t result)
{
    if((state_ & ~State::FromLCDMenu) != State::Processing)
        return;
    state_ = State::None;

    auto message = get_message(result);
    status.set(message);
	
    ExtUI::setTargetFan_percent(0, ExtUI::FAN0);
    if(result != ExtUI::PID_DONE)
        return;

    pid_settings.add_pid(kind_, temperature_);

    pid_settings.show();
}


}
