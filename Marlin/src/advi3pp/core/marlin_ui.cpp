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

// Minimal implementation of MarlinUI for ADVi3++

#include "../parameters.h"
#include "logging.h"
#include "../screens/print/pause.h"
#include "../../lcd/ultralcd.h"

using namespace ADVi3pp;

int16_t MarlinUI::preheat_hotend_temp[NB_MATERIAL_PRESET];
int16_t MarlinUI::preheat_bed_temp[NB_MATERIAL_PRESET];
uint8_t MarlinUI::preheat_fan_speed[NB_MATERIAL_PRESET];
//uint8_t MarlinUI::alert_level;

extern PauseMode pause_mode;

namespace
{
    bool external_control = false;
    uint8_t progress = 0;
}


void lcd_pause_show_message(const PauseMessage message, const PauseMode mode, const uint8_t /*extruder*/)
{
    Log::log() << F("lcd_pause_show_message(")
        << static_cast<uint16_t>(message)
        << F(", ")
        << static_cast<uint16_t>(mode)
        << F(")")
        << Log::endl();
    if (mode != PAUSE_MODE_SAME) pause_mode = mode;
    pause.show_message(message);
}

void MarlinUI::return_to_status()
{
    Log::log() << F("return_to_status") << Log::endl();
}

void MarlinUI::reset_alert_level()
{
    Log::log() << F("reset_alert_level") << Log::endl();
}

void MarlinUI::set_alert_status_P(PGM_P const message)
{
    Log::log() << F("set_alert_status_P") << Log::endl();
}

void MarlinUI::abort_print()
{
    Log::log() << F("abort_print") << Log::endl();
}

bool MarlinUI::has_status()
{
    Log::log() << F("has_status") << Log::endl();
    return false;
}

void MarlinUI::set_status(const char* const message, const bool persist)
{
    Log::log() << F("set_status") << Log::endl();
}

void MarlinUI::set_status_P(PGM_P const message, const int8_t level)
{
    Log::log() << F("set_status_P") << Log::endl();
}

void MarlinUI::status_printf_P(const uint8_t level, PGM_P const fmt, ...)
{
    Log::log() << F("status_printf_P") << Log::endl();
}

void MarlinUI::reset_status(const bool no_welcome)
{
    Log::log() << F("reset_status") << Log::endl();
}

void MarlinUI::set_contrast(const int16_t value)
{
    Log::log() << F("set_contrast") << Log::endl();
}

int16_t MarlinUI::get_contrast()
{
    Log::log() << F("get_contrast") << Log::endl();
    return 0x40;
}

void MarlinUI::update_buttons()
{
    // Nothing to do here, UI updates are made within idle()
}

bool MarlinUI::button_pressed()
{
    Log::log() << F("button_pressed") << Log::endl();
    return false;
}

void MarlinUI::quick_feedback(const bool clear_buttons)
{
    Log::log() << F("quick_feedback") << Log::endl();
}

void MarlinUI::refresh()
{
    Log::log() << F("refresh") << Log::endl();
}

void MarlinUI::wait_for_release()
{
    Log::log() << F("wait_for_release") << Log::endl();
}

bool MarlinUI::use_click()
{
    Log::log() << F("use_click") << Log::endl();
    return false;
}

void MarlinUI::capture()
{
    Log::log() << F("capture") << Log::endl();
    external_control = true;
}

void MarlinUI::release()
{
    Log::log() << F("release") << Log::endl();
    external_control = false;
}

void MarlinUI::chirp()
{
    Log::log() << F("chirp") << Log::endl();
}

void MarlinUI::set_progress(const progress_t p)
{
    progress = p;
}

void MarlinUI::set_progress_done()
{
    Log::log() << F("set_progress_done") << Log::endl();
    progress = 100;
}

uint8_t MarlinUI::get_progress_percent()
{
    return progress;
}