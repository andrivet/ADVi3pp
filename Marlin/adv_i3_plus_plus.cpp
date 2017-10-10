/**
 * Marlin 3D Printer Firmware For Wanhao Duplicator i3 Plus (ADVi3++)
 *
 * Copyright (C) 2017 Sebastien Andrivet [https://github.com/andrivet/]
 *
 * Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
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
#include "configuration_store.h"
#include "temperature.h"
#include "cardreader.h"
#include "stepper.h"
#include "watchdog.h"

#include "adv_i3_plus_plus.h"
#include "adv_i3_plus_plus_utils.h"
#include "adv_i3_plus_plus_private.h"

inline namespace { const uint16_t advi3_pp_version = 0x0100; } // 1.0.0

using namespace advi3pp;

// ---------------------------
// ------ i3PlusPrinter ------
// ---------------------------

inline namespace { i3PlusPrinterImpl i3plus; }

//! Initialize the printer and its LCD.
void i3PlusPrinter::setup()
{
    i3plus.setup();
}

//! Read data from the LCD and act accordingly.
void i3PlusPrinter::task()
{
    i3plus.task();
}

//! Show the given page on the LCD screen.
//! @param [in] page The page to be displayed on the LCD screen
void i3PlusPrinter::show_page(Page page)
{
    i3plus.show_page(page);
}

//! Update the graphs on the LCD screen.
void i3PlusPrinter::update_graph_data()
{
    i3plus.update_graph_data();
}

//! PID automatic tuning is finished.
void i3PlusPrinter::auto_pid_finished()
{
    i3plus.auto_pid_finished();
}

//! Store presets in permanent memory.
//! @param write Function to use for the actual writing
//! @param eeprom_index
//! @param working_crc
void i3PlusPrinter::store_presets(eeprom_write write, int eeprom_index, uint16_t& working_crc)
{
    i3plus.store_presets(write, eeprom_index, working_crc);
}

//! Restore presets from permanent memory.
//! @param read Function to use for the actual reading
//! @param eeprom_index
//! @param working_crc
void i3PlusPrinter::restore_presets(eeprom_read read, int eeprom_index, uint16_t& working_crc)
{
    i3plus.restore_presets(read, eeprom_index, working_crc);
}

//! Reset presets.
void i3PlusPrinter::reset_presets()
{
    i3plus.reset_presets();
}

//! Called when a temperature error occured and display the error on the LCD.
void i3PlusPrinter::temperature_error()
{
    i3plus.temperature_error();
}


// -------------------------------
// ------ i3PlusPrinterImpl ------
// -------------------------------

//! Initialize the printer and its LCD
//!
void i3PlusPrinterImpl::setup()
{
    Serial2.begin(115200);
    send_versions();
    show_page(Page::Boot);
}

//! Read data from the LCD and act accordingly
//!
void i3PlusPrinterImpl::task()
{
    read_serial();
    millis_t ms = millis();
    execute_looped_operation(ms);
    status_update(ms);
}

//! Store presets in permanent memory.
//! @param write Function to use for the actual writing
//! @param eeprom_index
//! @param working_crc
void i3PlusPrinterImpl::store_presets(eeprom_write write, int eeprom_index, uint16_t& working_crc)
{
    for(auto& preset: presets_)
    {
        write(eeprom_index, reinterpret_cast<uint8_t*>(&preset.hotend), sizeof(preset.hotend), &working_crc);
        write(eeprom_index, reinterpret_cast<uint8_t*>(&preset.bed), sizeof(preset.hotend), &working_crc);
    }
}

//! Restore presets from permanent memory.
//! @param read Function to use for the actual reading
//! @param eeprom_index
//! @param working_crc
void i3PlusPrinterImpl::restore_presets(eeprom_read read, int eeprom_index, uint16_t& working_crc)
{
    for(auto& preset: presets_)
    {
        read(eeprom_index, reinterpret_cast<uint8_t*>(&preset.hotend), sizeof(preset.hotend), &working_crc);
        read(eeprom_index, reinterpret_cast<uint8_t*>(&preset.bed), sizeof(preset.hotend), &working_crc);
    }
}

//! Reset presets.
void i3PlusPrinterImpl::reset_presets()
{
    presets_[0].hotend = DEFAULT_PREHEAT_PRESET1_HOTEND;
    presets_[1].hotend = DEFAULT_PREHEAT_PRESET2_HOTEND;
    presets_[2].hotend = DEFAULT_PREHEAT_PRESET3_HOTEND;

    presets_[0].bed = DEFAULT_PREHEAT_PRESET1_BED;
    presets_[1].bed = DEFAULT_PREHEAT_PRESET2_BED;
    presets_[2].bed = DEFAULT_PREHEAT_PRESET3_BED;
}

//! PID automatic tuning is finished.
void i3PlusPrinterImpl::auto_pid_finished()
{
    show_page(advi3pp::Page::AutoPidFinished);
    enqueue_and_echo_command("M106 S0");
    settings.save();
}

//! Start the bed leveling process.
void i3PlusPrinterImpl::leveling_init(millis_t ms)
{
    if (axis_homed[X_AXIS] && axis_homed[Y_AXIS] && axis_homed[Z_AXIS]) //stuck if leveling problem?
    {
        op_mode_ = OpMode::None;
        show_page(Page::Leveling);
    }
    else
        next_op_time_ = ms + 200;
}

//! Unload the filament if the temperature is high enough.
void i3PlusPrinterImpl::unload_filament(millis_t ms)
{
    if (thermalManager.current_temperature[0] >= thermalManager.target_temperature[0] - 10)
        enqueue_and_echo_commands_P(PSTR("G1 E-1 F120"));
    next_op_time_ = ms + 500;
}

//! Load the filament if the temperature is high enough.
void i3PlusPrinterImpl::load_filament(millis_t ms)
{
    if (thermalManager.current_temperature[0] >= thermalManager.target_temperature[0] - 10)
        enqueue_and_echo_commands_P(PSTR("G1 E1 F120"));
    next_op_time_ = ms + 500;
}

//! If there is an operating running, execute its next step
//! @param ms Time of the operation
void i3PlusPrinterImpl::execute_looped_operation(millis_t ms)
{
    if (!ELAPSED(ms, next_op_time_))
        return;

    switch (op_mode_)
    {
        case OpMode::LevelInit:
            leveling_init(ms);
            break;
        case OpMode::UnloadFilament:
            unload_filament(ms);
            break;
        case OpMode::LoadFilament:
            load_filament(ms);
            break;
        default:
            break;
    }
}

namespace
{
    int16_t scale(int16_t value, int16_t valueScale, int16_t targetScale)
    {
        return value * targetScale / valueScale;
    }
}

//! Update the status of the printer on thr LCD
//! @param ms Time of the operation
void i3PlusPrinterImpl::status_update(millis_t ms)
{
    if (!ELAPSED(ms, next_update_))
        return;

    next_update_ = ms + 500;

    Frame frame{Command::WriteRamData};
    frame << Variable::TargetHotEnd
          << Uint16(thermalManager.target_temperature[0])
          << Uint16(thermalManager.degHotend(0))
          << Uint16(thermalManager.target_temperature_bed)
          << Uint16(thermalManager.degBed())
          << Uint16(scale(fanSpeeds[0], 256, 100))
          << Uint16(card.percentDone());
    frame.send();

    switch (temp_graph_update_)
    {
        case GraphUpdate::Next:
            temp_graph_update_ = GraphUpdate::Skip;
            update_graph_data();
            break;

        case GraphUpdate::Skip:
            temp_graph_update_ = GraphUpdate::Next;
            break;

        default:
            break;
    }
}

//! Show the given page on the LCD screen
//! @param [in] page The page to be displayed on the LCD screen
void i3PlusPrinterImpl::show_page(Page page)
{
    Frame frame{Command::WriteRegisterData};
    frame << Register::PictureID << 00_u8 << page;
    frame.send();
}

//! Retrieve the current page on the LCD screen
Page i3PlusPrinterImpl::get_current_page()
{
    Frame frame{Command::ReadRegisterData};
    frame << Register::PictureID << 2_u8; // read 2 bytes
    frame.send();

    Frame response;
    if (!response.receive())
        return Page::None;

    Uint16 page; response >> page;
    return static_cast<Page>(page.word);
}

//! Read a frame from the LCD and act accordingly
void i3PlusPrinterImpl::read_serial()
{
    Frame frame;
    if(!frame.receive())
        return;

    // TODO: Check value of command (KeyCode)

    Action action; Uint8 length; Uint16 value;
    frame >> action >> length >> value;
    auto key_value = static_cast<KeyValue>(value.word);

    // TODO: Check that length == 1, that Hi(action) == 0x04

    switch(action)
    {
        case Action::SdCard:              sd_card(key_value); break;
        case Action::SdCardSelectFile:    sd_card_select_file(key_value); break;
        case Action::PrintStop:           print_stop(key_value); break;
        case Action::PrintPause:          print_pause(key_value); break;
        case Action::PrintResume:         print_resumt(key_value); break;
        case Action::Preheat:             preheat(key_value); break;
        case Action::Cooldown:            cooldown(key_value); break;
        case Action::MotorsSettings:      motors_or_pid_settings(key_value); break;
        case Action::SaveSettings:        save_motors_or_pid_settings(key_value); break;
        case Action::FactoryReset:        factory_reset(key_value); break;
        case Action::PrintSettings:       print_settings(key_value); break;
        case Action::SavePrintSettings:   save_print_settings(key_value); break;
        case Action::LoadUnloadBack:      load_unload_back(key_value); break;
        case Action::Level:               level(key_value); break;
        case Action::Filament:            filament(key_value); break;
        case Action::XPlus:               move_x_plus(key_value); break;
        case Action::XMinus:              move_x_minus(key_value); break;
        case Action::YPlus:               move_y_plus(key_value); break;
        case Action::YMinus:              move_y_minus(key_value); break;
        case Action::ZPlus:               move_z_plus(key_value); break;
        case Action::ZMinus:              move_z_minus(key_value); break;
        case Action::EPlus:               move_e_plus(key_value); break;
        case Action::EMinus:              move_e_minus(key_value); break;
        case Action::DisableMotors:       disable_motors(key_value); break;
        case Action::HomeX:               home_X(key_value); break;
        case Action::HomeY:               home_y(key_value); break;
        case Action::HomeZ:               home_z(key_value); break;
        case Action::HomeAll:             home_all(key_value); break;
        case Action::Statistics:          statistics(key_value); break;
        case Action::PidTuning:           pid_tuning(key_value); break;
        case Action::TemperatureGraph:    temperature_graph(key_value); break;
        case Action::Print:               print(key_value); break;
        case Action::About:               about(key_value); break;
        case Action::LcdUpdate:           lcd_update_mode(key_value);
    }
}

//! LCD SD card menu
void i3PlusPrinterImpl::sd_card(KeyValue key_value)
{
    static const uint16_t NB_VISIBLE_FILES = 5;

    if(card.sdprinting)
    {
        show_page(Page::Print);
        return;
    }

    if(key_value == KeyValue::Show)
    {
        card.initsd();
        if(card.cardOK)
            last_file_index_ = card.getnrfilenames() - 1;
    }

    if(!card.cardOK)
        return;

    uint16_t nb_files = card.getnrfilenames();
    if(nb_files > NB_VISIBLE_FILES)
    {
        switch(key_value)
        {
            case KeyValue::Up:
                if ((last_file_index_ + NB_VISIBLE_FILES) < nb_files)
                    last_file_index_ += NB_VISIBLE_FILES;
                break;

            case KeyValue::Down:
                if (last_file_index_ >= NB_VISIBLE_FILES)
                    last_file_index_ -= NB_VISIBLE_FILES;
                break;

            default:
                break;
        }
    }

    Frame frame{Command::WriteRamData};
    frame << Variable::FileName1;

    Name<> name;
    for(uint8_t index = 0; index < NB_VISIBLE_FILES; ++index)
    {
        get_file_name(index, name);
        frame << name;
    }

    frame.send();

    show_page(Page::SdCard);
};

template<size_t S>
void i3PlusPrinterImpl::get_file_name(uint8_t index, Name<S>& name)
{
    card.getfilename(last_file_index_ - index);
    name = card.longFilename;
};

void i3PlusPrinterImpl::sd_card_select_file(KeyValue key_value)
{
    if(!card.cardOK)
        return;

    auto file_index = static_cast<uint16_t>(key_value);
    if(file_index > last_file_index_)
        return;
    card.getfilename(last_file_index_ - file_index);
    Name<> name{card.longFilename};

    Frame frame{Command::WriteRamData};
    frame << Variable::SelectedFileName << name;
    frame.send();

    card.openFile(card.filename, true);
    card.startFileprint();
    print_job_timer.start();

    temp_graph_update_ = GraphUpdate::Next;
    show_page(Page::Print);
}

void i3PlusPrinterImpl::print_stop(KeyValue)
{
    card.stopSDPrint();
    clear_command_queue();
    quickstop_stepper();
    print_job_timer.stop();
    thermalManager.disable_all_heaters();
#if FAN_COUNT > 0
    for(auto& fanSpeed : fanSpeeds)
        fanSpeed = 0;
#endif
    temp_graph_update_ = GraphUpdate::No;
    show_page(Page::Main);
}

void i3PlusPrinterImpl::print_pause(KeyValue)
{
    card.pauseSDPrint();
    print_job_timer.pause();
#if ENABLED(PARK_HEAD_ON_PAUSE)
    enqueue_and_echo_commands_P(PSTR("M125"));
#endif
}

void i3PlusPrinterImpl::print_resumt(KeyValue)
{
#if ENABLED(PARK_HEAD_ON_PAUSE)
    enqueue_and_echo_commands_P(PSTR("M24"));
#else
    card.startFileprint();
    print_job_timer.start();
#endif
}

void i3PlusPrinterImpl::preheat(KeyValue key_value)
{
    if(key_value == KeyValue::Show)
    {
        Frame frame{Command::WriteRamData};
        frame << Variable::Preset1Bed;
        for(auto& preset : presets_)
            frame << Uint8(preset.hotend) << Uint8(preset.bed);
        frame.send();
        show_page(Page::Preheat);
    }
    else
    {
        Frame frame{Command::ReadRamData};
        frame << Variable::Preset1Bed << 6_u8;
        frame.send();

        Frame response;
        if(!response.receive())
            return;

        Uint16 hotend, bed;
        for(auto& preset : presets_)
        {
            response >> hotend >> bed;
            preset.hotend = hotend.word;
            preset.bed = bed.word;
        }

        enqueue_and_echo_commands_P(PSTR("M500"));

        auto presetIndex = static_cast<uint16_t>(key_value);
        if(presetIndex >= NB_PRESETS)
            return;
        const Preset& preset = presets_[presetIndex];

        Name<> command;

        command << "M104 S" << preset.hotend;
        enqueue_and_echo_command(command.c_str());

        command << "M140 S" << preset.bed;
        enqueue_and_echo_command(command.c_str());
    }
}

void i3PlusPrinterImpl::cooldown(KeyValue)
{
    thermalManager.disable_all_heaters();
}

void i3PlusPrinterImpl::motors_or_pid_settings(KeyValue key_value)
{
    Frame frame{Command::WriteRamData};
    frame << Variable::MotorSettingsX
          << Uint16(planner.axis_steps_per_mm[X_AXIS] * 10)
          << Uint16(planner.axis_steps_per_mm[Y_AXIS] * 10)
          << Uint16(planner.axis_steps_per_mm[Z_AXIS] * 10)
          << Uint16(planner.axis_steps_per_mm[E_AXIS] * 10)
          << Uint16(PID_PARAM(Kp, 0) * 10)
          << Uint16(unscalePID_i(PID_PARAM(Ki, 0)) * 10)
          << Uint16(unscalePID_d(PID_PARAM(Kd, 0)) * 10);
    frame.send();

    show_page(key_value == KeyValue::PidSettings ? Page::PidSettings: Page::MotoSettings);
}

void i3PlusPrinterImpl::save_motors_or_pid_settings(KeyValue)
{
    Frame frame{Command::ReadRamData};
    frame << Variable::MotorSettingsX << 0x07_u8;
    frame.send();

    Frame response;
    if(!response.receive())
        return;

    Uint16 x, y, z, e, p, i, d;
    response >> x >> y >> z >> e >> p >> i >> d;

    planner.axis_steps_per_mm[X_AXIS] = static_cast<float>(x.word) / 10;
    planner.axis_steps_per_mm[Y_AXIS] = static_cast<float>(y.word) / 10;
    planner.axis_steps_per_mm[Z_AXIS] = static_cast<float>(z.word) / 10;
    planner.axis_steps_per_mm[E_AXIS] = static_cast<float>(e.word) / 10;

    PID_PARAM(Kp, 0) = static_cast<float>(p.word) / 10;
    PID_PARAM(Ki, 0) = scalePID_i(static_cast<float>(i.word) / 10);
    PID_PARAM(Kd, 0) = scalePID_d(static_cast<float>(d.word) / 10);

    enqueue_and_echo_commands_P(PSTR("M500"));
    show_page(Page::System);
}

void i3PlusPrinterImpl::factory_reset(KeyValue)
{
    enqueue_and_echo_commands_P(PSTR("M502"));
    enqueue_and_echo_commands_P(PSTR("M500"));
}

void i3PlusPrinterImpl::print_settings(KeyValue)
{
    Frame frame{Command::WriteRamData};
    frame << Variable::PrintSettingsSpeed
          << Uint16(feedrate_percentage)
          << Uint16(thermalManager.degTargetHotend(0))
          << Uint16(thermalManager.degTargetBed())
          << Uint16(fanSpeeds[0] * 100 / 256);
    frame.send();
    show_page(Page::PrintSettings);
}

void i3PlusPrinterImpl::save_print_settings(KeyValue)
{
    Frame frame{Command::ReadRamData};
    frame << Variable::PrintSettingsSpeed << 0x04_u8;
    frame.send();

    Frame response;
    if(!response.receive())
        return;

    Uint16 speed, hotend, bed, fan;
    response >> speed >> hotend >> bed >> fan;

    feedrate_percentage = speed.word;
    thermalManager.setTargetHotend(hotend.word, 0);
    thermalManager.setTargetBed(bed.word);
    fanSpeeds[0] = fan.word * 256 / 100;

    show_page(Page::Print);
}

void i3PlusPrinterImpl::load_unload_back(KeyValue)
{
    op_mode_ = OpMode::None;
    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G90")); // absolute mode
    thermalManager.setTargetHotend(0, 0);
    show_page(Page::Filament);
}

void i3PlusPrinterImpl::level(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::LevelStart:
            show_page(Page::LevelingStart);
            axis_homed[X_AXIS] = axis_homed[Y_AXIS] = axis_homed[Z_AXIS] = false;
            enqueue_and_echo_commands_P(PSTR("G90")); //absolute mode
            enqueue_and_echo_commands_P((PSTR("G28")));//homeing
            next_op_time_ = millis() + 200;
            op_mode_ = OpMode::LevelInit;
            break;

        case KeyValue::LevelStep1:
            enqueue_and_echo_commands_P((PSTR("G1 Z10 F2000")));
            enqueue_and_echo_commands_P((PSTR("G1 X30 Y30 F6000")));
            enqueue_and_echo_commands_P((PSTR("G1 Z0 F1000")));
            break;

        case KeyValue::LevelStep2:
            enqueue_and_echo_commands_P((PSTR("G1 Z10 F2000")));
            enqueue_and_echo_commands_P((PSTR("G1 X170 Y170 F6000")));
            enqueue_and_echo_commands_P((PSTR("G1 Z0 F1000")));
            break;

        case KeyValue::LevelStep3:
            enqueue_and_echo_commands_P((PSTR("G1 Z10 F2000")));
            enqueue_and_echo_commands_P((PSTR("G1 X170 Y30 F6000")));
            enqueue_and_echo_commands_P((PSTR("G1 Z0 F1000")));
            break;

        case KeyValue::LevelStep4:
            enqueue_and_echo_commands_P((PSTR("G1 Z10 F2000")));
            enqueue_and_echo_commands_P((PSTR("G1 X30 Y170 F6000")));
            enqueue_and_echo_commands_P((PSTR("G1 Z0 F1000")));
            break;

        case KeyValue::LevelFinish:
            enqueue_and_echo_commands_P((PSTR("G1 Z30 F2000")));
            show_page(Page::Tools);
            break;
    }
}

void i3PlusPrinterImpl::filament(KeyValue key_value)
{
    if(key_value == KeyValue::Show)
    {
        Frame frame{Command::WriteRamData};
        frame << Variable::TargetTemperature << 200_u16;
        frame.send();
        show_page(Page::Filament);
        return;
    }

    Frame frame{Command::ReadRamData};
    frame << Variable::TargetTemperature << 1_u8;
    frame.send();

    Frame response;
    if(!response.receive())
        return;

    Uint16 hotend;
    response >> hotend;

    thermalManager.setTargetHotend(hotend.word, 0);
    enqueue_and_echo_commands_P(PSTR("G91")); // relative mode

    next_op_time_ = millis() + 500;

    op_mode_ = key_value == KeyValue::Load ? OpMode::LoadFilament : OpMode::UnloadFilament;

}

void i3PlusPrinterImpl::move_x_plus(KeyValue)
{
    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 X5 F3000"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

void i3PlusPrinterImpl::move_x_minus(KeyValue)
{
    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 X-5 F3000"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

void i3PlusPrinterImpl::move_y_plus(KeyValue)
{
    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 Y5 F3000"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

void i3PlusPrinterImpl::move_y_minus(KeyValue)
{
    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 Y-5 F3000"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

void i3PlusPrinterImpl::move_z_plus(KeyValue)
{
    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0.5 F3000"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

void i3PlusPrinterImpl::move_z_minus(KeyValue)
{
    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 Z-0.5 F3000"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

void i3PlusPrinterImpl::move_e_plus(KeyValue)
{
    if (thermalManager.degHotend(0) >= 180)
    {
        clear_command_queue();
        enqueue_and_echo_commands_P(PSTR("G91"));
        enqueue_and_echo_commands_P(PSTR("G1 E1 F120"));
        enqueue_and_echo_commands_P(PSTR("G90"));
    }
}

void i3PlusPrinterImpl::move_e_minus(KeyValue)
{
    if (thermalManager.degHotend(0) >= 180)
    {
        clear_command_queue();
        enqueue_and_echo_commands_P(PSTR("G91"));
        enqueue_and_echo_commands_P(PSTR("G1 E-1 F120"));
        enqueue_and_echo_commands_P(PSTR("G90"));
    }
}

void i3PlusPrinterImpl::disable_motors(KeyValue)
{
    enqueue_and_echo_commands_P(PSTR("M84"));
    axis_homed[X_AXIS] = axis_homed[Y_AXIS] = axis_homed[Z_AXIS] = false;
}

void i3PlusPrinterImpl::home_X(KeyValue)
{
    enqueue_and_echo_commands_P(PSTR("G28 X0"));
}

void i3PlusPrinterImpl::home_y(KeyValue)
{
    enqueue_and_echo_commands_P(PSTR("G28 Y0"));
}

void i3PlusPrinterImpl::home_z(KeyValue)
{
    enqueue_and_echo_commands_P(PSTR("G28 Z0"));
}

void i3PlusPrinterImpl::home_all(KeyValue)
{
    enqueue_and_echo_commands_P(PSTR("G28"));
}

void i3PlusPrinterImpl::statistics(KeyValue)
{
    send_stats();
    show_page(Page::Statistics);
}

void i3PlusPrinterImpl::pid_tuning(KeyValue key_value)
{
    if(key_value == KeyValue::Show)
    {
        Frame frame{Command::WriteRamData};
        frame << Variable::TargetTemperature << 200_u16;
        frame.send();
        show_page(Page::AutoPidTuning);
        return;
    };

    Frame frame{Command::ReadRamData};
    frame << Variable::TargetTemperature << 1_u8;
    frame.send();

    Frame response;
    if(!response.receive())
        return;
    Uint16 hotend; response >> hotend;

    enqueue_and_echo_command("M106 S255");
    Name<> command; command << "M303 S" << hotend.word << "E0 C8 U1";
    enqueue_and_echo_command(command.c_str());
    temp_graph_update_ = GraphUpdate::Next;
};

void i3PlusPrinterImpl::temperature_graph(KeyValue key_value)
{
    if(key_value == KeyValue::Back)
    {
        temp_graph_update_ = GraphUpdate::No;
        show_page(last_page_);
    }

    last_page_ = get_current_page();
    temp_graph_update_ = GraphUpdate::Next;
    show_page(Page::Temperature);
}

void i3PlusPrinterImpl::print(KeyValue)
{
    temp_graph_update_ = GraphUpdate::Next;
    if(!card.sdprinting)
    {
        Frame frame{Command::WriteRamData};
        frame << Variable::SelectedFileName << Name<>{"No SD print"};
        frame.send();
    }

    show_page(Page::Print);
}

void i3PlusPrinterImpl::lcd_update_mode(KeyValue)
{
    show_page(Page::LcdUpdate);

    while(true)
    {
        watchdog_reset();
        if(Serial.available())
            Serial2.write(Serial.read());
    }
}

Name<16> i3PlusPrinterImpl::get_lcd_firmware_version()
{
    Name<16> lcd_version;

    Frame frame{Command::ReadRegisterData};
    frame << Register::Version << 1_u8; // read 1 byte
    frame.send();

    Frame response;
    if(response.receive())
    {
        Uint8 version; response >> version;
        lcd_version << (version.byte / 0x10) << '.' << (version.byte % 0x10);
    }
    else
        lcd_version << "Unknown";

    return lcd_version;
}

Name<16> convert_version(uint16_t hex_version)
{
    Name<16> version;
    version << hex_version / 0x0100 << "." << (hex_version % 0x100) / 0x10 << "." << hex_version % 0x10;
    return version;
}

void i3PlusPrinterImpl::send_versions()
{
    Name<16> marlin_version{SHORT_BUILD_VERSION};
    Name<16> motherboard_version = convert_version(advi3_pp_version);
    Name<16> advi3pp_lcd_version = convert_version(adv_i3_pp_lcd_version_);
    Name<16> lcd_firmware_version = get_lcd_firmware_version();

    Frame frame{Command::WriteRamData};
    frame << Variable::LcdVersion << marlin_version << motherboard_version << advi3pp_lcd_version << lcd_firmware_version;
    frame.send();
}

void i3PlusPrinterImpl::about(KeyValue key_value)
{
    adv_i3_pp_lcd_version_ = static_cast<uint16_t>(key_value);
    send_versions();

    if(adv_i3_pp_lcd_version_ != advi3_pp_version)
        show_page(Page::Mismatch);
    else
        show_page(Page::About);
}

void i3PlusPrinterImpl::send_stats()
{
    printStatistics stats = print_job_timer.getStats();

    Frame frame{Command::WriteRamData};
    frame << Variable::TotalPrints << Uint16(stats.totalPrints) << Uint16(stats.finishedPrints);
    frame.send();

#if ENABLED(PRINTCOUNTER)
    duration_t duration = stats.printTime;
    frame << Variable::TotalPrintTime << Name<16>{duration};
    frame.send();

    duration = stats.longestPrint;
    frame << Variable::LongestPrintTime << Name<16>{duration};
    frame.send();

    Name<> filament_used;
    filament_used << static_cast<unsigned int>(stats.filamentUsed / 1000)
                  << "."
                  << static_cast<unsigned int>(stats.filamentUsed / 100) % 10;
    frame << Variable::TotalFilament << Name<>{filament_used.c_str()};
    frame.send();
#endif
}

void i3PlusPrinterImpl::update_graph_data()
{
    Frame frame{Command::WriteCurveData};
    frame << 0x03_u8 << Uint16{thermalManager.degHotend(0)} << Uint16{thermalManager.degBed()};
    frame.send();
}

void i3PlusPrinterImpl::temperature_error()
{
    show_page(advi3pp::Page::ThermalRunawayError);
}
