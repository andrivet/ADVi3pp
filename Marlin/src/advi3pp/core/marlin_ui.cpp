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
#include "../../lcd/ultralcd.h"

int16_t MarlinUI::preheat_hotend_temp[NB_MATERIAL_PRESET];
int16_t MarlinUI::preheat_bed_temp[NB_MATERIAL_PRESET];
uint8_t MarlinUI::preheat_fan_speed[NB_MATERIAL_PRESET];
//uint8_t MarlinUI::alert_level;
bool MarlinUI::external_control;

void lcd_pause_show_message(const PauseMessage message, const PauseMode mode, const uint8_t extruder)
{

}

void MarlinUI::return_to_status()
{

}

void MarlinUI::reset_alert_level()
{

}

void MarlinUI::set_alert_status_P(PGM_P const message)
{

}

void MarlinUI::abort_print()
{

}

bool MarlinUI::has_status()
{

}

void MarlinUI::set_status(const char* const message, const bool persist)
{

}

void MarlinUI::set_status_P(PGM_P const message, const int8_t level)
{

}

void MarlinUI::status_printf_P(const uint8_t level, PGM_P const fmt, ...)
{

}

void MarlinUI::reset_status(const bool no_welcome)
{

}

void MarlinUI::set_contrast(const int16_t value)
{

}

int16_t MarlinUI::get_contrast()
{

}

void MarlinUI::update_buttons()
{

}

bool MarlinUI::button_pressed()
{

}

void MarlinUI::quick_feedback(const bool clear_buttons)
{

}

void MarlinUI::refresh()
{

}

void MarlinUI::wait_for_release()
{

}

bool MarlinUI::use_click()
{

}

void MarlinUI::chirp()
{

}

void MarlinUI::set_progress(const progress_t p)
{

}

void MarlinUI::set_progress_done()
{

}

uint8_t MarlinUI::get_progress_percent()
{

}