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

#include "../inc/MarlinConfigPre.h"
#include "extui/ui_api.h"
#include "src/advi3pp/inc/advi3pp.h"

namespace ExtUI {

void onStartup()
{
    ADVi3pp::Facade::on_startup();
}

void onIdle()
{
    ADVi3pp::Facade::on_idle();
}

void onPrinterKilled(PGM_P const error, PGM_P const component)
{
    ADVi3pp::Facade::on_killed(error, component);
}

void onMediaInserted()
{
    ADVi3pp::Facade::on_media_inserted();
}

void onMediaError()
{
    ADVi3pp::Facade::on_media_error();
}

void onMediaRemoved()
{
    ADVi3pp::Facade::on_media_removed();
}

void onPlayTone(const uint16_t frequency, const uint16_t duration)
{
    ADVi3pp::Facade::on_play_tone(frequency, duration);
}

void onPrintTimerStarted()
{
    ADVi3pp::Facade::on_print_started();
}

void onPrintTimerPaused()
{
    ADVi3pp::Facade::on_print_paused();
}

void onPrintTimerStopped()
{
    ADVi3pp::Facade::on_print_stopped();
}

void onFilamentRunout(const extruder_t extruder)
{
    ADVi3pp::Facade::on_filament_runout(extruder);
}

void onUserConfirmRequired(const char * const msg)
{
    ADVi3pp::Facade::on_user_confirm_required(msg);
}

void onStatusChanged(const char * const msg)
{
    ADVi3pp::Facade::on_status_changed(msg);
}

void onFactoryReset()
{
    ADVi3pp::Facade::on_factory_reset();
}

void onStoreSettings(char *buff)
{
    // Called when saving to EEPROM (i.e. M500). If the ExtUI needs
    // permanent data to be stored, it can write up to eeprom_data_size bytes
    // into buff.

    // Example:
    //  static_assert(sizeof(myDataStruct) <= ExtUI::eeprom_data_size);
    //  memcpy(buff, &myDataStruct, sizeof(myDataStruct));
}

void onStoreSettingsEx(ExtUI::eeprom_write write, int& eeprom_index, uint16_t& working_crc)
{
    ADVi3pp::Facade::on_store_settings(write, eeprom_index, working_crc);
}

void onLoadSettings(const char *buff)
{
    // Called while loading settings from EEPROM. If the ExtUI
    // needs to retrieve data, it should copy up to eeprom_data_size bytes
    // from buff

    // Example:
    //  static_assert(sizeof(myDataStruct) <= ExtUI::eeprom_data_size);
    //  memcpy(&myDataStruct, buff, sizeof(myDataStruct));
}

void onLoadSettingsEx(ExtUI::eeprom_read read, int& eeprom_index, uint16_t& working_crc)
{
    ADVi3pp::Facade::on_load_settings(read, eeprom_index, working_crc);
}

uint16_t getSizeofSettings()
{
    return ADVi3pp::Facade::on_sizeof_settings();
}

void onConfigurationStoreWritten(bool success)
{
    // Called after the entire EEPROM has been written,
    // whether successful or not.
    ADVi3pp::Facade::on_settings_written(success);
}

void onConfigurationStoreRead(bool success)
{
    // Called after the entire EEPROM has been read,
    // whether successful or not.
    ADVi3pp::Facade::on_settings_loaded(success);
}

void onMeshUpdate(const int8_t xpos, const int8_t ypos, const float zval)
{
    // Called when any mesh points are updated
    ADVi3pp::Facade::on_mesh_updated(xpos, ypos, zval);
}

#if ENABLED(POWER_LOSS_RECOVERY)
void onPowerLossResume()
{
    // Called on resume from power-loss
    ADVi3pp::Facade::on_power_less_resume();
}
#endif

void onPidTuning(const result_t rst)
{
    // Called for temperature PID tuning result
    ADVi3pp::Facade::on_pid_tuning(rst);
}

void onPidTuningProgress(int cycleIndex, int nbCycles)
{
    // TODO implementation
}

void onPidTuningReportTemp(int heater)
{
    // TODO implementation
}


}

