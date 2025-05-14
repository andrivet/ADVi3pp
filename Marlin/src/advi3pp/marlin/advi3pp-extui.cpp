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

#include "../../inc/MarlinConfig.h"
#include "../../lcd/extui/ui_api.h"
#include "../core/core.h"
#include "../core/buzzer.h"
#include "../screens/common/wait.h"
#include "../core/status.h"
#include "../core/dimming.h"
#include "../screens/leveling/mesh.h"
#include "../screens/tuning/pid_tuning.h"
#include "../screens/print/sd_card.h"

namespace ExtUI {

  using namespace ADVi3pp;

  void onStartup() {
    Log::info() << F("ExtUI::onStartup") << Log::endl();
    Core::startup();
  }

  void onIdle() {
    Core::idle();
  }

  void onMediaMounted() {
    Log::info() << F("ExtUI::onMediaMounted") << Log::endl();
    SdCard::on_media_inserted();
  }

  void onMediaError() {
    Log::info() << F("ExtUI::onMediaError") << Log::endl();
    SdCard::on_media_error();
  }

  void onMediaRemoved() {
    Log::info() << F("ExtUI::onMediaRemoved") << Log::endl();
    SdCard::on_media_removed();
  }

  void onMediaOpenError(const char*) {
    SdCard::on_media_open_error();
  }

  void onHeatingError(const heater_id_t header_id) {
    Log::info() << F("ExtUI::onHeatingError") << Log::endl();
    Core::killed(GET_TEXT_F(MSG_ERR_HEATING_FAILED), header_id);
  }

  void onMinTempError(const heater_id_t header_id) {
    Log::info() << F("ExtUI::onMinTempError") << Log::endl();
    Core::killed(GET_TEXT_F(MSG_ERR_MINTEMP), header_id);
  }

  void onMaxTempError(const heater_id_t header_id) {
    Log::info() << F("ExtUI::onMaxTempError") << Log::endl();
    Core::killed(GET_TEXT_F(MSG_ERR_MAXTEMP), header_id);
  }

  void onPlayTone(const uint16_t frequency, const uint16_t duration) {
    Log::info() << F("ExtUI::onPlayTone ") << frequency << " " << duration << Log::endl();
    Buzzer::buzz_on_action(duration);
  }

  void onPrinterKilled(FSTR_P const error, FSTR_P const component) {
    Log::info() << F("ExtUI::onPrinterKilled ") << error << " " << component << Log::endl();
    Core::killed(error, component);
  }

  void onPrintTimerStarted() {
    Log::info() << F("ExtUI::onPrintTimerStarted") << Log::endl();
    Core::print_started();
  }

  void onPrintTimerPaused() {
    Log::info() << F("ExtUI::onPrintTimerPaused") << Log::endl();
    Core::print_paused();
  }

  void onPrintTimerStopped() {
    Log::info() << F("ExtUI::onPrintTimerStopped") << Log::endl();
    Core::print_done();
  }

  void onPrintDone() {
    Log::info() << F("ExtUI::onPrintDone") << Log::endl();
    Core::print_done();
  }

  void onFilamentRunout(const extruder_t extruder) {
    Log::info() << F("ExtUI::onFilamentRunout") << Log::endl();
  }

  void onUserConfirmRequired(const char * const msg) {
    const auto awaiting = ExtUI::awaitingUserConfirm();
    Log::info() << F("ExtUI::onUserConfirmRequired") << msg << "," << awaiting << Log::endl();
    Wait::wait_user(GET_TEXT_F(ADVI3PP_TITLE_CONFIRM), msg, awaiting);
  }

  void onChangeFilamentConfirmRequired() {
    Log::info() << F("ExtUI::onChangeFilamentConfirmRequired") << Log::endl();
    Core::pause_temperature();
  }

  void onPauseMode(
      const PauseMessage message,
      const PauseMode mode,
      const uint8_t extruder
  ) {
    Log::info() << F("ExtUI::onPauseMode") << message << mode << extruder << Log::endl();
    if (mode != PAUSE_MODE_SAME) pause_mode = mode;
    ExtUI::pauseModeStatus = message;
    const auto awaiting = ExtUI::awaitingUserConfirm();
    auto confirm_title = GET_TEXT_F(ADVI3PP_TITLE_CONFIRM);
    switch (message) {
      case PAUSE_MESSAGE_PARKING:  Status::set(GET_TEXT_F(MSG_PAUSE_PRINT_PARKING)); ExtUI::setUserConfirmed(false); break;
      case PAUSE_MESSAGE_CHANGING: Wait::wait_user(confirm_title, GET_TEXT_F(MSG_FILAMENT_CHANGE_INIT), awaiting); break;
      case PAUSE_MESSAGE_WAITING:  Status::set(GET_TEXT_F(MSG_ADVANCED_PAUSE_WAITING)); break;
      case PAUSE_MESSAGE_INSERT:   Wait::wait_user(confirm_title, GET_TEXT_F(MSG_FILAMENT_CHANGE_INSERT), awaiting); break;
      case PAUSE_MESSAGE_LOAD:     Status::set(GET_TEXT_F(MSG_FILAMENT_CHANGE_LOAD)); break;
      case PAUSE_MESSAGE_UNLOAD:   Wait::wait_user(confirm_title, GET_TEXT_F(MSG_FILAMENT_CHANGE_UNLOAD), awaiting); break;
      case PAUSE_MESSAGE_PURGE:    Wait::wait_user(GET_TEXT_F(ADVI3PP_MSG_TITLE_PURGE), GET_TEXT_F(MSG_FILAMENT_CHANGE_PURGE), awaiting); break;
      case PAUSE_MESSAGE_OPTION:   Core::display(Page::PauseOptions); break;
      case PAUSE_MESSAGE_RESUME:   Status::set(GET_TEXT_F(MSG_FILAMENT_CHANGE_RESUME)); break;
      case PAUSE_MESSAGE_HEAT:     Wait::wait_user(confirm_title, GET_TEXT_F(MSG_FILAMENT_CHANGE_HEAT), awaiting); break;
      case PAUSE_MESSAGE_HEATING:  Status::set(GET_TEXT_F(MSG_FILAMENT_CHANGE_HEATING)); break;
      case PAUSE_MESSAGE_STATUS:   ExtUI::onShowStatus(); break;
      default: break;
    }
  }

  void onShowStatus() {
    Log::info() << F("ExtUI::onShowStatus") << Log::endl();
  }

  void onStatusChanged(const char * const msg, bool persist) {
    //Log::info() << F("ExtUI::onStatusChanged(") << msg << F(", ") << persist << F(")") << Log::endl();
    Status::set(msg, persist ? Status::STATUS_OPTIONS::PERSISTENT | Status::STATUS_OPTIONS::RESET : Status::STATUS_OPTIONS::RESET );
    if(persist) Dimming::sleep_off();
  }

  void onHomingStart() {
    Log::info() << F("ExtUI::onHomingStart") << Log::endl();
    Wait::homing_start();
  }

  void onHomingDone() {
    Log::info() << F("ExtUI::onHomingDone") << Log::endl();
    Wait::homing_done();
  }

  void onFactoryReset() {
    Log::info() << F("ExtUI::onFactoryReset") << Log::endl();
  }

  void onStoreSettings(char *buff) {
    Log::info() << F("ExtUI::onStoreSettings") << Log::endl();
  }

  void onLoadSettings(const char *buff) {
    Log::info() << F("ExtUI::onLoadSettings") << Log::endl();
  }

  void onPostprocessSettings() {
    // Called after loading or resetting stored settings
    Log::info() << F("ExtUI::onPostprocessSettings") << Log::endl();
  }

  void onSettingsStored(bool success) {
    // Called after the entire EEPROM has been written,
    // whether successful or not.
    Log::info() << F("ExtUI::onSettingsStored") << Log::endl();
  }

  void onSettingsLoaded(bool success) {
    // Called after the entire EEPROM has been read,
    // whether successful or not.
    Log::info() << F("ExtUI::onSettingsLoaded") << Log::endl();
    Core::on_settings_loaded(success);
  }

  void onSettingsValidated(bool success) {
    Log::info() << F("ExtUI::onSettingsValidated") << Log::endl();
    Core::on_settings_validated(success);
  }

  void onLevelingStart() {
    Log::info() << F("ExtUI::onLevelingStart") << Log::endl();
    Mesh::start();
  }

  void onLevelingProgress(const int8_t index, const int8_t xpos, const int8_t ypos) {
    Log::info() << F("ExtUI::onLevelingProgress") << index << xpos << ypos << Log::endl();
    Mesh::progress(index);
  }

  void onLevelingDone(bool success) {
    Log::info() << F("ExtUI::onLevelingDone") << success << Log::endl();
    Mesh::done(success);
  }

  void onMeshUpdate(const int8_t xpos, const int8_t ypos, const_float_t zval) {
    // Called when any mesh point is updated
  }

  void onMeshUpdate(const int8_t xpos, const int8_t ypos, probe_state_t state) {
    // Called to indicate a special condition
    Log::info() << F("ExtUI::onMeshUpdate") << Log::endl();
  }

  #if ENABLED(PREVENT_COLD_EXTRUSION)
  void onSetMinExtrusionTemp(const celsius_t) {
    Log::info() << F("ExtUI::onSetMinExtrusionTemp") << Log::endl();
  }
  #endif

  #if ENABLED(POWER_LOSS_RECOVERY)
  void onSetPowerLoss(const bool onoff) {
    Log::info() << F("ExtUI::onSetPowerLoss") << Log::endl();
    Core::on_power_loss_set(onoff);
  }

  void onPowerLoss() {
    Log::info() << F("ExtUI::onPowerLoss") << Log::endl();
    Core::on_power_loss();
  }

  void onPowerLossResume() {
    // Called on resume from power-loss
    Log::info() << F("ExtUI::onPowerLossResume") << Log::endl();
    Core::on_power_loss_resume();
  }
  #endif

  #if ENABLED(PSU_CONTROL)
  void onPowerOff() {
    Log::info() << F("ExtUI::onPowerOff") << Log::endl();
    Core::on_power_off();
  }
  #endif

  void onPIDTuning(const pidresult_t rst) {
    Log::info() << F("ExtUI::onPidTuning") << Log::endl();

    if(rst == PID_STARTED || rst == PID_BED_STARTED)
      PidTuning::on_start(rst == PID_BED_STARTED);
    else
      PidTuning::on_finished(rst);
  }

  void onStartM303(const int count, const heater_id_t hid, const celsius_t temp) {
    Log::info() << F("ExtUI::onStartM303") << Log::endl();
  }

  void onPIDTuningProgress(int cycleIndex, int nbCycles) {
    Log::info() << F("ExtUI::onPidTuningProgress") << cycleIndex << nbCycles << Log::endl();
    PidTuning::on_progress(cycleIndex, nbCycles);
  }

  void onPIDTuningReportTemp(int /*heater*/) {
    Log::info() << F("ExtUI::onPidTuningReportTemp") << Log::endl();
    // Nothing to do
  }

  void onSteppersDisabled() {
    Log::info() << F("ExtUI::onSteppersDisabled") << Log::endl();
  }

  void onSteppersEnabled() {
    Log::info() << F("ExtUI::onSteppersEnabled") << Log::endl();
  }

  void onAxisDisabled(const axis_t axis) {
    Log::info() << F("ExtUI::onAxisDisabled") << axis << Log::endl();
  }

  void onAxisEnabled(const axis_t axis) {
    //Log::info() << F("ExtUI::onAxisEnabled") << axis << Log::endl();
  }

}
