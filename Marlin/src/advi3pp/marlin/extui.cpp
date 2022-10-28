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
#include "../screens/core/wait.h"
#include "../screens/leveling/automatic.h"
#include "../screens/tuning/pid_tuning.h"

namespace ExtUI {

using namespace ADVi3pp;

void onStartup() {
  Log::log() << F("ExtUI::onStartup") << Log::endl();
  core.startup();
}

void onIdle() {
  core.idle();
}

void onMediaInserted() {
  core.media_inserted();
}

void onMediaError() {
  core.media_error();
}

void onMediaRemoved() {
  core.media_removed();
}

void onMediaOpenError(const char* filename) {
  status.set(F("Error opening file"));
}

void onPlayTone(const uint16_t frequency, const uint16_t duration) {
  Log::log() << F("ExtUI::onPlayTone ") << frequency << " " << duration << Log::endl();
  buzzer.buzz_on_action(duration);
}

void onPrinterKilled(float temp, FSTR_P const error, FSTR_P const component) {
  Log::log() << F("ExtUI::onPrinterKilled ") << temp << error << " " << component << Log::endl();
  core.killed(temp, error, component);
}

void onPrintTimerStarted() {
  pages.go_to_print();
}

void onPrintTimerPaused() {
  Log::log() << F("ExtUI::onPrintTimerPaused") << Log::endl();
}

void onPrintTimerStopped() {
  Log::log() << F("ExtUI::onPrintTimerStopped") << Log::endl();
}

void onPrintDone() {
  Log::log() << F("ExtUI::onPrintDone") << Log::endl();
}

void onFilamentRunout(const extruder_t extruder) {
  Log::log() << F("ExtUI::onFilamentRunout") << Log::endl();
}

void onUserConfirmRequired(const char * const msg) {
  Log::log() << F("ExtUI::onUserConfirmRequired") << msg << "," << ExtUI::awaitingUserConfirm() << Log::endl();

  wait.wait_user(msg);
}

void onStatusChanged(const char * const msg) {
  Log::log() << F("ExtUI::onStatusChanged") << msg << Log::endl();
  if(msg == NULL || *msg == 0)
    status.reset_and_clear();
  else
    status.set(msg);
}

void onHomingStart() {
  Log::log() << F("ExtUI::onHomingStart") << Log::endl();
}

void onHomingDone() {
  Log::log() << F("ExtUI::onHomingDone") << Log::endl();
}

void onSteppersDisabled() {
  Log::log() << F("ExtUI::onSteppersDisabled") << Log::endl();
}

void onSteppersEnabled() {
  Log::log() << F("ExtUI::onSteppersEnabled") << Log::endl();
}

void onFactoryReset() {
  Log::log() << F("ExtUI::onFactoryReset") << Log::endl();
  settings.on_factory_reset();
}

void onStoreSettings(char * /*buff*/) {
  Log::log() << F("ExtUI::onStoreSettings") << Log::endl();
  // Implement onStoreSettingsEx instead
}

void onStoreSettingsEx(ExtUI::eeprom_write write, int& eeprom_index, uint16_t& working_crc) {
  settings.on_store_settings(write, eeprom_index, working_crc);
}

void onLoadSettings(const char */*buff*/) {
  Log::log() << F("ExtUI::onLoadSettings") << Log::endl();
  // Implement onLoadSettingsEx instead
}

bool onLoadSettingsEx(ExtUI::eeprom_read read, int& eeprom_index, uint16_t& working_crc, bool validating) {
    return settings.on_load_settings(read, eeprom_index, working_crc, validating);
}

uint16_t getSizeofSettings() {
    return settings.on_sizeof_settings();
}

void onPostprocessSettings() {
  // Called after loading or resetting stored settings
  Log::log() << F("ExtUI::onPostprocessSettings") << Log::endl();
}

void onSettingsStored(bool success) {
  // Called after the entire EEPROM has been written,
  // whether successful or not.
  Log::log() << F("ExtUI::onSettingsStored") << Log::endl();
  settings.on_settings_written(success);
}

void onSettingsLoaded(bool success) {
  // Called after the entire EEPROM has been read,
  // whether successful or not.
  Log::log() << F("ExtUI::onSettingsLoaded") << Log::endl();
  settings.on_settings_loaded(success);
}

void onSettingsValidated(bool success) {
  settings.on_settings_validated(success);
}

void onLevelingStart() {
  automatic_leveling.on_start();
}

void onLevelingProgress(const int8_t index, const int8_t xpos, const int8_t ypos) {
  automatic_leveling.on_progress(index, xpos, ypos);
}

void onLevelingDone(bool success) {
  automatic_leveling.on_done(success);
}

void onMeshUpdate(const int8_t xpos, const int8_t ypos, const_float_t zval) {
  // Called when any mesh point is updated
  Log::log() << F("ExtUI::onMediaOpenError") << Log::endl();
}

void onMeshUpdate(const int8_t xpos, const int8_t ypos, probe_state_t state) {
  // Called to indicate a special condition
  Log::log() << F("ExtUI::onMeshUpdate") << Log::endl();
}

#if ENABLED(POWER_LOSS_RECOVERY)
void onPowerLossResume()
{
    // Called on resume from power-loss
    Facade::on_power_less_resume();
}
#endif

void onPidTuning(const result_t rst) {
  Log::log() << F("ExtUI::onPidTuning") << Log::endl();

  if(rst == PID_STARTED)
      pid_tuning.on_start();
  else
      pid_tuning.on_finished(rst);
}

void onPidTuningProgress(int cycleIndex, int nbCycles) {
  Log::log() << F("ExtUI::onPidTuningProgress") << Log::endl();
  pid_tuning.on_progress(cycleIndex, nbCycles);
}

void onPidTuningReportTemp(int /*heater*/) {
  Log::log() << F("ExtUI::onPidTuningReportTemp") << Log::endl();
  // Nothing to do
}

}
