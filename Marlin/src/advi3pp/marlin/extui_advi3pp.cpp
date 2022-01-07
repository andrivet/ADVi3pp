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

#include "../../inc/MarlinConfigPre.h"
#include "../../lcd/extui/ui_api.h"
#include "../core/core.h"
#include "../core/buzzer.h"
#include "../core/status.h"
#include "../core/settings.h"
#include "../screens/leveling/automatic.h"
#include "../screens/tuning/pid_tuning.h"

namespace ExtUI {

void onStartup()
{
    ADVi3pp::core.startup();
}

void onIdle()
{
    ADVi3pp::core.idle();
}

void onPrinterKilled(FSTR_P const error, FSTR_P const component)
{
    ADVi3pp::core.killed(error);
}

// There is no way to detect media changes, so this part is not implemented
void onMediaInserted() { }
void onMediaError() { }
void onMediaRemoved() { }

void onMediaOpenError(const char* filename) {
    ADVi3pp::status.set(F("Error opening file"));
}

void onPlayTone(const uint16_t frequency, const uint16_t duration)
{
    ADVi3pp::buzzer.buzz_on_action();
}

void onPrintTimerStarted()
{
}

void onPrintTimerPaused()
{
}

void onPrintTimerStopped()
{
}

void onFilamentRunout(const extruder_t extruder)
{
}

void onUserConfirmRequired(const char * const msg)
{
    ADVi3pp::status.set(msg); // TODO: Check this
    ADVi3pp::core.wait_user_confirm();
}

void onStatusChanged(const char * const msg)
{
    ADVi3pp::status.set(msg); // TODO: Check this
}

void onHomingStart() {}
void onHomingComplete() {}
void onPrintFinished() {}

void onFactoryReset()
{
    ADVi3pp::settings.on_factory_reset();
}

void onStoreSettings(char * /*buff*/)
{
    // Implement onStoreSettingsEx instead
}

void onStoreSettingsEx(ExtUI::eeprom_write write, int& eeprom_index, uint16_t& working_crc)
{
    ADVi3pp::settings.on_store_settings(write, eeprom_index, working_crc);
}

void onLoadSettings(const char */*buff*/)
{
    // Implement onLoadSettingsEx instead
}

bool onLoadSettingsEx(ExtUI::eeprom_read read, int& eeprom_index, uint16_t& working_crc, bool validating)
{
    return ADVi3pp::settings.on_load_settings(read, eeprom_index, working_crc, validating);
}

uint16_t getSizeofSettings()
{
    return ADVi3pp::settings.on_sizeof_settings();
}

void onPostprocessSettings() {
    // Called after loading or resetting stored settings
}

void onConfigurationStoreWritten(bool success)
{
    // Called after the entire EEPROM has been written,
    // whether successful or not.
    ADVi3pp::settings.on_settings_written(success);
}

void onConfigurationStoreRead(bool success)
{
    // Called after the entire EEPROM has been read,
    // whether successful or not.
    ADVi3pp::settings.on_settings_loaded(success);
}

void onConfigurationStoreValidated(bool success)
{
    ADVi3pp::settings.on_settings_validated(success);
}

void onMeshLevelingStart() {}

void onMeshUpdate(const int8_t xpos, const int8_t ypos, const_float_t zval)
{
    // Called when any mesh point is updated
}

void onMeshUpdate(const int8_t xpos, const int8_t ypos, probe_state_t state) {
    // Called to indicate a special condition
}

void onAutomaticLevelingFinished(bool success)
{
#if HAS_LEVELING
    ADVi3pp::automatic_leveling.leveling_finished(success);
#endif
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
    if(rst == PID_STARTED)
        ADVi3pp::pid_tuning.on_start();
    else
        ADVi3pp::pid_tuning.on_finished(rst);
}

void onPidTuningProgress(int cycleIndex, int nbCycles)
{
    ADVi3pp::pid_tuning.on_progress(cycleIndex, nbCycles);
}

void onPidTuningReportTemp(int /*heater*/)
{
    // Nothing to do
}

void onSteppersDisabled() {}
void onSteppersEnabled()  {}

}
