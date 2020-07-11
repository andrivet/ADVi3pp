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

#include "../parameters.h"
#include "../inc/advi3pp.h"
#include "../../gcode/parser.h"
#include "../versions.h"
#include "core.h"
#include "graphs.h"
#include "dimming.h"
#include "status.h"
#include "pages.h"
#include "task.h"
#include "dgus.h"
#include "buzzer.h"
#include "../screens/core/no_sensor.h"
#include "../screens/controls/controls.h"
#include "../screens/core/wait.h"
#include "../screens/controls/load_unload.h"
#include "../screens/controls/preheat.h"
#include "../screens/controls/move.h"
#include "../screens/controls/manual_leveling.h"
#include "../screens/controls/automatic_leveling.h"
#include "../screens/controls/leveling_grid.h"
#include "../screens/print/print.h"
#include "../screens/print/sd_card.h"
#include "../screens/print/change_filament.h"
#include "../screens/print/temperatures.h"
#include "../screens/tuning/extruder_tuning.h"
#include "../screens/tuning/pid_tuning.h"
#include "../screens/tuning/sensor_tuning.h"
#include "../screens/tuning/sensor_z_height.h"
#include "../screens/tuning/diagnosis.h"
#include "../screens/settings/eeprom_mismatch.h"
#include "../screens/settings/factory_reset.h"
#include "../screens/settings/sensor_settings.h"
#include "../screens/settings/lcd_settings.h"
#include "../screens/settings/print_settings.h"
#include "../screens/settings/pid_settings.h"
#include "../screens/settings/step_settings.h"
#include "../screens/settings/feedrate_settings.h"
#include "../screens/settings/acceleration_settings.h"
#include "../screens/settings/jerk_settings.h"
#include "../screens/settings/linear_advance_settings.h"
#include "../screens/info/versions.h"
#include "../screens/info/statistics.h"
#include "../screens/info/copyrights.h"

namespace ADVi3pp {

Core core;


void Facade::on_startup()
{
    core.startup();
}

void Facade::on_idle()
{
    core.idle();
}

void Facade::on_killed(PGM_P error, PGM_P component)
{
    core.killed(reinterpret_cast<const FlashChar*>(error));
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

void Facade::on_store_settings(ExtUI::eeprom_write write, int& eeprom_index, uint16_t& working_crc)
{
    settings.write(write, eeprom_index, working_crc);
}

void Facade::on_load_settings(ExtUI::eeprom_read read, int& eeprom_index, uint16_t& working_crc)
{
    settings.read(read, eeprom_index, working_crc);
}

uint16_t Facade::on_sizeof_settings()
{
    return settings.size_of();
}

void Facade::on_factory_reset()
{
    settings.reset();
}

void Facade::on_settings_written(bool success)
{
}

void Facade::on_settings_loaded(bool success)
{
    if(!success)
        eeprom_mismatch.show(ShowOptions::None);
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

void Facade::process_command()
{
    switch(GCodeParser::codenum)
    {
        case 0: print.process_pause_resume_code(); break;
        case 1: print.process_stop_code(); break;
        default: Log::error() << F("Invalid command ") << static_cast<uint16_t>(GCodeParser::codenum) << Log::endl(); break;
    }
}

void Core::startup()
{
    pinMode(LED_PIN, OUTPUT); // To help debugging when serial is not available
    Frame::open();
}

bool Core::init()
{
    if(init_)
        return false;
    init_ = true;

    Frame::check_lcd_connectivity();
    send_gplv3_7b_notice(); // You are not authorized to remove or alter this notice
    graphs.clear();
    dimming.reset(true);
    versions.send_versions();

    SERIAL_ECHO_START();

    ADVString<32> welcome;
    welcome << F("ADVi3++ ");
    core.convert_version(welcome, advi3_pp_version);
    welcome << F(" is ready");
    SERIAL_ECHOLN(welcome.get());
    status.set(welcome.align(Alignment::Center).get());

    pages.show_page(Page::Boot, ShowOptions::None);
    
    return true;
}

void Core::idle()
{
    init();

    receive_lcd_serial_data();
    dimming.check();
    task.execute_background_task();
    update_progress();
    send_lcd_serial_data();
    graphs.update();
}

void Core::killed(const FlashChar* error)
{
    status.set(error);
    send_lcd_serial_data(true);
    pages.show_page(Page::Killed);
}

//! Note to forks author:
//! Under GPLv3 provision 7(b), you are not authorized to remove or alter this notice.
void Core::send_gplv3_7b_notice()
{
    SERIAL_ECHO_START();
    SERIAL_ECHOLNPGM("Based on ADVi3++, Copyright (C) 2017-2020 Sebastien Andrivet");
}

//! Update the progress bar if the printer is printing for the SD card
void Core::update_progress()
{
    // TODO Not sure it is necessary
}

//! Read a frame from the LCD and act accordingly.
void Core::receive_lcd_serial_data()
{
    // Format of the frame (example):
    // header | length | command | action | nb words | key code
    // -------|--------|---------|--------|----------|---------
    //      2 |      1 |       1 |      2 |        1 |        2   bytes
    //  5A A5 |     06 |      83 |  04 60 |       01 |    01 50

    IncomingFrame frame;
    if(!frame.available())
        return;

    if(!frame.receive())
    {
        Log::error() << F("reading incoming Frame") << Log::endl();
        return;
    }

    buzzer.buzz_on_press();
    dimming.reset();

    Command command{}; Action action{}; Uint8 nb_words; Uint16 value;
    frame >> command >> action >> nb_words >> value;
    auto key_value = static_cast<KeyValue>(value.word);

#ifdef ADVi3PP_LOG_FRAMES
    Log::log() << F("=R=> ") << nb_words.byte << F(" words, Action = 0x") << static_cast<uint16_t>(action)
               << F(", KeyValue = 0x") << value.word << Log::endl();
#endif

    switch(action)
    {
        case Action::Controls:              controls.handle(key_value); break;
        case Action::PrintCommand:          print.handle(key_value); break;
        case Action::Wait:                  wait.handle(key_value); break;
        case Action::LoadUnload:            load_unload.handle(key_value); break;
        case Action::Preheat:               preheat.handle(key_value); break;
        case Action::Move:                  move.handle(key_value); break;
        case Action::SdCard:                sd_card.handle(key_value); break;
        case Action::FactoryReset:          factory_reset.handle(key_value); break;
        case Action::ManualLeveling:        manual_leveling.handle(key_value); break;
        case Action::ExtruderTuning:        extruder_tuning.handle(key_value); break;
        case Action::PidTuning:             pid_tuning.handle(key_value); break;
        case Action::SensorSettings:        sensor_settings.handle(key_value); break;
        case Action::NoSensor:              no_sensor.handle(key_value); break;
        case Action::LCD:                   lcd_settings.handle(key_value); break;
        case Action::Statistics:            statistics.handle(key_value); break;
        case Action::Versions:              versions.handle(key_value); break;
        case Action::PrintSettings:         print_settings.handle(key_value); break;
        case Action::PIDSettings:           pid_settings.handle(key_value); break;
        case Action::StepsSettings:         steps_settings.handle(key_value); break;
        case Action::FeedrateSettings:      feedrates_settings.handle(key_value); break;
        case Action::AccelerationSettings:  accelerations_settings.handle(key_value); break;
        case Action::JerkSettings:          jerks_settings.handle(key_value); break;
        case Action::Copyrights:            copyrights.handle(key_value); break;
        case Action::SensorTuning:          sensor_tuning.handle(key_value); break;
        case Action::AutomaticLeveling:     automatic_leveling.handle(key_value); break;
        case Action::SensorGrid:            leveling_grid.handle(key_value); break;
        case Action::SensorZHeight:         sensor_z_height.handle(key_value); break;
        case Action::ChangeFilament:        change_filament.handle(key_value); break;
        case Action::EEPROMMismatch:        eeprom_mismatch.handle(key_value); break;
        case Action::LinearAdvanceSettings: linear_advance_settings.handle(key_value); break;
        case Action::Diagnosis:             diagnosis.handle(key_value); break;
        case Action::Temperatures:          temperatures.handle(key_value); break;

        case Action::MoveXPlus:             move.x_plus_command(); break;
        case Action::MoveXMinus:            move.x_minus_command(); break;
        case Action::MoveYPlus:             move.y_plus_command(); break;
        case Action::MoveYMinus:            move.y_minus_command(); break;
        case Action::MoveZPlus:             move.z_plus_command(); break;
        case Action::MoveZMinus:            move.z_minus_command(); break;
        case Action::MoveEPlus:             move.e_plus_command(); break;
        case Action::MoveEMinus:            move.e_minus_command(); break;
        case Action::BabyMinus:             print_settings.baby_minus_command(); break;
        case Action::BabyPlus:              print_settings.baby_plus_command(); break;
        case Action::ZHeightMinus:          sensor_z_height.minus(); break;
        case Action::ZHeightPlus:           sensor_z_height.plus(); break;
        case Action::FeedrateMinus:         print_settings.feedrate_minus_command(); break;
        case Action::FeedratePlus:          print_settings.feedrate_plus_command(); break;
        case Action::FanMinus:              print_settings.fan_minus_command(); break;
        case Action::FanPlus:               print_settings.fan_plus_command(); break;
        case Action::HotendMinus:           print_settings.hotend_minus_command(); break;
        case Action::HotendPlus:            print_settings.hotend_plus_command(); break;
        case Action::BedMinus:              print_settings.bed_minus_command(); break;
        case Action::BedPlus:               print_settings.bed_plus_command(); break;
        case Action::LCDBrightness:         lcd_settings.change_brightness(static_cast<int16_t>(key_value)); break;

        default:                            Log::error() << F("Invalid action ") << static_cast<uint16_t>(action) << Log::endl(); break;
    }
}

//! Update the status of the printer on the LCD.
void Core::send_lcd_serial_data(bool force_update)
{
    // Right time for an update or force update?
    if(!force_update && !task.is_update_time())
        return;

    // The progress bar is split into two parts because of a limitation of the DWIN panel
    // so compute the progress of each part.
    int16_t progress_bar_low  = ExtUI::getProgress_percent() >= 50 ? 10 : ExtUI::getProgress_percent() / 5;
    int16_t progress_var_high = ExtUI::getProgress_percent() < 50 ? 0 : (ExtUI::getProgress_percent() - 50) / 5;

#ifdef ADVi3PP_PROBE
    uint16_t probe_state = ExtUI::getLevelingActive() ? 2 : 1;
#else
    uint16_t probe_state = 0;
#endif

    // Send the current status in one frame
    WriteRamDataRequest frame{Variable::TargetBed};
    frame << Uint16(ExtUI::getTargetTemp_celsius(ExtUI::BED))
          << Uint16(ExtUI::getActualTemp_celsius(ExtUI::BED))
          << Uint16(ExtUI::getTargetTemp_celsius(ExtUI::E0))
          << Uint16(ExtUI::getActualTemp_celsius(ExtUI::E0))
          << Uint16(ExtUI::getActualFan_percent(ExtUI::FAN0))
          << Uint16(round(ExtUI::getAxisPosition_mm(ExtUI::Z) * 100))
          << Uint16(progress_bar_low)
          << Uint16(progress_var_high)
          << 0_u16 // TODO
          << Uint16(probe_state)
          << Uint16(ExtUI::getFeedrate_percent());
    frame.send(false);

    status.compute_progress();
    status.send();
}

bool Core::ensure_not_printing()
{
    if(!ExtUI::isPrinting())
        return true;

    wait.show_back(F("Not accessible when printing"));
    return false;
}

bool Core::is_busy()
{
    return ExtUI::isMoving() || ExtUI::commandsInQueue(); // TODO it is enough (we are not looking at busy_state)?
}

void Core::inject_commands(const FlashChar* commands)
{
    ExtUI::injectCommands_P(reinterpret_cast<const char*>(commands));
}

}
