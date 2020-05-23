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

#include "../inc/advi3pp.h"
#include "core.h"
#include "graphs.h"
#include "dimming.h"
#include "status.h"
#include "pages.h"
#include "background_task.h"
#include "../screens/settings/eeprom_mismatch.h"
#include "../screens/info/versions.h"

namespace ADVi3pp {

Core core;

void Facade::on_startup()
{
    core.send_gplv3_7b_notice(); // You are not authorized to remove or alter this notice
    graphs.clear();
    dimming.reset(true);
    status.reset();
    core.show_boot_page();
    status.set(F("ADVi3++ is ready"));
}

void Facade::on_idle()
{
    core.read_lcd_serial();
    dimming.check();
    task.execute_background_task();
    core.update_progress();
    core.send_status_data();
    graphs.update();
}

void Facade::on_killed(PGM_P const error, PGM_P const component)
{
}

void Facade::on_media_inserted()
{
}

void Facade::on_media_error()
{
}

void Facade::on_media_removed()
{
}

void Facade::on_play_tone(const uint16_t frequency, const uint16_t duration)
{
}

void Facade::on_print_started()
{
}

void Facade::on_print_paused()
{
}

void Facade::on_print_stopped()
{
}

void Facade::on_filament_runout(const ExtUI::extruder_t extruder)
{
}

void Facade::on_user_confirm_required(const char* msg)
{
}

void Facade::on_status_changed(const char* msg)
{
}

void Facade::on_factory_reset()
{
}

void Facade::on_store_settings(char* buff)
{
}

void Facade::on_load_settings(const char* buff)
{
}

void Facade::on_settings_written(bool success)
{
}

void Facade::on_settings_loaded(bool success)
{
}

void Facade::on_mesh_updated(const int8_t xpos, const int8_t ypos, const float zval)
{
}

#if ENABLED(POWER_LOSS_RECOVERY)
void Facade::on_power_less_resume()
{
}
#endif

void Facade::on_pid_tuning(const ExtUI::result_t rst)
{
}



//! Note to forks author:
//! Under GPLv3 provision 7(b), you are not authorized to remove or alter this notice.
void Core::send_gplv3_7b_notice()
{
    SERIAL_ECHOLNPGM("Based on ADVi3++, Copyright (C) 2017-2020 Sebastien Andrivet");
}

//! Display the Boot animation (page)
void Core::show_boot_page()
{
    if(!eeprom_mismatch.check())
        return;

    versions.send_versions();

    pages.show_page(Page::Boot, ShowOptions::None);
}

//! Update the progress bar if the printer is printing for the SD card
void Core::update_progress()
{
    // TODO
}

//! Read a frame from the LCD and act accordingly.
void Core::read_lcd_serial()
{
    // TODO
}

//! Update the status of the printer on the LCD.
void Core::send_status_data()
{
    // TODO
}

}
