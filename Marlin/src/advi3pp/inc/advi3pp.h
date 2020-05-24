/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin)
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

#pragma once

#include "../../lcd/extui/ui_api.h"

namespace ADVi3pp {

struct Facade
{
    static void on_startup();
    static void on_idle();
    static void on_killed(PGM_P const error, PGM_P const component);
    static void on_media_inserted();
    static void on_media_error();
    static void on_media_removed();
    static void on_play_tone(const uint16_t frequency, const uint16_t duration);
    static void on_print_started();
    static void on_print_paused();
    static void on_print_stopped();
    static void on_filament_runout(const ExtUI::extruder_t extruder);
    static void on_user_confirm_required(const char* msg);
    static void on_status_changed(const char* msg);
    static void on_factory_reset();
    static void on_store_settings(ExtUI::eeprom_write write, int& eeprom_index, uint16_t& working_crc);
    static void on_load_settings(ExtUI::eeprom_read read, int& eeprom_index, uint16_t& working_crc);
    static uint16_t on_sizeof_settings();
    static void on_settings_written(bool success);
    static void on_settings_loaded(bool success);
    static void on_mesh_updated(const int8_t xpos, const int8_t ypos, const float zval);
#if ENABLED(POWER_LOSS_RECOVERY)
    static void on_power_less_resume();
#endif
    static void on_pid_tuning(const ExtUI::result_t rst);
};

}
