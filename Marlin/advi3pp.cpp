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

#include "advi3pp.h"
#include "advi3pp_utils.h"
#include "advi3pp_impl.h"

#ifdef DEBUG
#pragma message "This is a DEBUG build"
#endif

#ifdef SIMULATOR
#pragma message "This is a SIMULATOR build"
#endif

namespace
{
    const uint16_t advi3_pp_version = 0x0210;                       // 2.1.0
    const uint16_t advi3_pp_oldest_lcd_compatible_version = 0x0210; // 2.1.0
    const uint16_t advi3_pp_newest_lcd_compatible_version = 0x0210; // 2.1.0
    // Modify also DETAILED_BUILD_VERSION in Version.h

    const unsigned long advi3_pp_baudrate = 115200;
    const uint16_t nb_visible_sd_files = 5;
    const uint16_t calibration_cube_size = 20; // 20 mm
    const uint16_t calibration_extruder_filament = 100; // 10 cm
	const uint16_t calibration_extruder_delta = 20; // 2 cm
}

namespace advi3pp {

inline namespace { PrinterImpl printer; };

// --------------------------------------------------------------------
// Printer
// --------------------------------------------------------------------

//! Initialize the printer and its LCD.
void Printer::setup()
{
    printer.setup();
}

//! Read data from the LCD and act accordingly.
void Printer::task()
{
    printer.task();
}

//! PID automatic tuning is finished.
void Printer::auto_pid_finished()
{
    printer.auto_pid_finished();
}

//! Store presets in permanent memory.
//! @param write Function to use for the actual writing
//! @param eeprom_index
//! @param working_crc
void Printer::store_presets(eeprom_write write, int& eeprom_index, uint16_t& working_crc)
{
    printer.store_presets(write, eeprom_index, working_crc);
}

//! Restore presets from permanent memory.
//! @param read Function to use for the actual reading
//! @param eeprom_index
//! @param working_crc
void Printer::restore_presets(eeprom_read read, int& eeprom_index, uint16_t& working_crc)
{
    printer.restore_presets(read, eeprom_index, working_crc);
}

//! Reset presets.
void Printer::reset_presets()
{
    printer.reset_presets();
}

//! Called when a temperature error occurred and display the error on the LCD.
void Printer::temperature_error()
{
    printer.temperature_error();
}

void Printer::send_temperatures_data()
{
    printer.send_temperatures_data();
}

// --------------------------------------------------------------------
// PrinterImpl
// --------------------------------------------------------------------

//! Initialize the printer and its LCD
void PrinterImpl::setup()
{
#ifdef DEBUG
    Log::log() << F("This is a DEBUG build");
#endif
#ifdef SIMULATOR
    Log::log() << F("This is a SIMULATOR build");
#endif

    Serial2.begin(advi3_pp_baudrate);
    send_versions();
    clear_graphs();
    show_page(Page::Boot, false);
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Presets
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Store presets in permanent memory.
//! @param write Function to use for the actual writing
//! @param eeprom_index
//! @param working_crc
void PrinterImpl::store_presets(eeprom_write write, int& eeprom_index, uint16_t& working_crc)
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
void PrinterImpl::restore_presets(eeprom_read read, int& eeprom_index, uint16_t& working_crc)
{
    for(auto& preset: presets_)
    {
        read(eeprom_index, reinterpret_cast<uint8_t*>(&preset.hotend), sizeof(preset.hotend), &working_crc);
        read(eeprom_index, reinterpret_cast<uint8_t*>(&preset.bed), sizeof(preset.hotend), &working_crc);
    }
}

//! Reset presets.
void PrinterImpl::reset_presets()
{
    presets_[0].hotend = DEFAULT_PREHEAT_PRESET1_HOTEND;
    presets_[1].hotend = DEFAULT_PREHEAT_PRESET2_HOTEND;
    presets_[2].hotend = DEFAULT_PREHEAT_PRESET3_HOTEND;

    presets_[0].bed = DEFAULT_PREHEAT_PRESET1_BED;
    presets_[1].bed = DEFAULT_PREHEAT_PRESET2_BED;
    presets_[2].bed = DEFAULT_PREHEAT_PRESET3_BED;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Background tasks
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Set the next (minimal) background task time
//! @param delta    Duration to be added to the current time to compute the next (minimal) background task time
void PrinterImpl::set_next_background_task_time(unsigned int delta)
{
    next_op_time_ = millis() + delta;
}

//! Set the next (minimal) update time
//! @param delta    Duration to be added to the current time to compute the next (minimal) update time
void PrinterImpl::set_next_update_time(unsigned int delta)
{
    next_update_time_ = millis() + delta;
}

//! Set the next background task and its delay
//! @param task     The next background task
//! @param delta    Duration to be added to the current time to execute the background tast
void PrinterImpl::set_background_task(BackgroundTask task, unsigned int delta)
{
    background_task_ = task;
    set_next_background_task_time(delta);
}

//! Reset the background task
void PrinterImpl::clear_background_task()
{
    background_task_ = BackgroundTask::None;
}

//! If there is an operating running, execute its next step
void PrinterImpl::execute_background_task()
{
    if(!ELAPSED(millis(), next_op_time_))
        return;

    switch(background_task_)
    {
        case BackgroundTask::None:                  break;
        case BackgroundTask::Leveling:              leveling_task(); break;
        case BackgroundTask::LoadFilament:          load_filament_task(); break;
        case BackgroundTask::UnloadFilament:        unload_filament_task(); break;
        case BackgroundTask::ExtruderCalibration:   extruder_calibration_task(); break;
        default:                                    Log::error() << F("Invalid background task ") << static_cast<uint16_t>(background_task_) << Log::endl(); break;
    }
}

namespace
{
    //! Transform a value from a scale to another one.
    //! @param value        Value to be transformed
    //! @param valueScale   Current scale of the value (maximal)
    //! @param targetScale  Target scale
    //! @return             The scaled value
    int16_t scale(int16_t value, int16_t valueScale, int16_t targetScale) { return value * targetScale / valueScale; }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Pages management
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Show the given page on the LCD screen
//! @param [in] page The page to be displayed on the LCD screen
void PrinterImpl::show_page(Page page, bool save_back)
{
    Log::log() << F("Show page ") << static_cast<uint8_t>(page) << Log::endl();

    if(save_back)
        back_pages_.push(get_current_page());

    WriteRegisterDataRequest frame{Register::PictureID};
    frame << 00_u8 << page;
    frame.send(true);
}

//! Retrieve the current page on the LCD screen
Page PrinterImpl::get_current_page()
{
    ReadRegisterDataRequest frame{Register::PictureID, 2};
    frame.send();

    ReadRegisterDataResponse response;
    if(!response.receive(frame))
    {
        Log::error() << F("Reading PictureID") << Log::endl();
        return Page::None;
    }

    Uint16 page; response >> page;
    Log::log() << F("Current page index = ") << page.word << Log::endl();
    return static_cast<Page>(page.word);
}

//! Set page to display after the completion of an operation.
void PrinterImpl::save_forward_page()
{
    forward_page_ = get_current_page();
}

//! Show the "Back" page on the LCD display.
void PrinterImpl::show_back_page()
{
    if(back_pages_.is_empty())
    {
        Log::error() << F("No Back page defined" )<< Log::endl();
        return;
    }

    show_page(back_pages_.pop(), false);
}

//! Show the "Next" page on the LCD display.
void PrinterImpl::show_forward_page()
{
    if(forward_page_ == Page::None)
    {
        Log::error() << F("No Forward page defined") << Log::endl();
        return;
    }

    if(!back_pages_.contains(forward_page_))
    {
        show_back_page();
        return;
    }

    while(!back_pages_.is_empty())
    {
        Page page = back_pages_.pop();
        if(page == forward_page_)
        {
            forward_page_ = Page::None;
            show_page(page, false);
            return;
        }
    }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Incoming LCD commands and status update
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Read data from the LCD and act accordingly
void PrinterImpl::task()
{
    read_lcd_serial();
    execute_background_task();
    send_status();
    update_graphs();
}

//! Update the status of the printer on the LCD.
void PrinterImpl::send_status()
{
    auto current_time = millis();
    if(!ELAPSED(current_time, next_update_time_))
        return;
    set_next_update_time();

    WriteRamDataRequest frame{Variable::TargetBed};
    frame << Uint16(thermalManager.target_temperature_bed)
          << Uint16(thermalManager.degBed())
          << Uint16(thermalManager.target_temperature[0])
          << Uint16(thermalManager.degHotend(0))
          << Uint16(scale(fanSpeeds[0], 255, 100))
          << FixedSizeString(LCDImpl::instance().get_message(), 26)
          << FixedSizeString(LCDImpl::instance().get_progress(), 26);
    frame.send(false);
}

void PrinterImpl::send_temperatures_data()
{
    WriteRamDataRequest frame{Variable::TargetBed};
    frame << Uint16(thermalManager.target_temperature_bed)
          << Uint16(thermalManager.degBed())
          << Uint16(thermalManager.target_temperature[0])
          << Uint16(thermalManager.degHotend(0))
          << Uint16(scale(fanSpeeds[0], 255, 100));
    frame.send(false);

    send_graphs_data();
}

//! Read a frame from the LCD and act accordingly.
void PrinterImpl::read_lcd_serial()
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

    Command command; Action action; Uint8 nb_words; Uint16 value;
    frame >> command >> action >> nb_words >> value;
    auto key_value = static_cast<KeyValue>(value.word);

    // TODO: Check that length == 1, that Hi(action) == 0x04
    Log::log() << F(">>> ") << nb_words.byte << F(" words, Action = ") << static_cast<uint16_t>(action) << F(", KeyValue = ") << value.word << Log::endl();

    switch(action)
    {
        case Action::Main:                  main(key_value); break;
        case Action::SdPrintCommand:        sd_print_command(key_value); break;
        case Action::UsbPrintCommand:       usb_print_command(key_value); break;
        case Action::LoadUnload:            load_unload(key_value); break;
        case Action::Preheat:               preheat(key_value); break;
        case Action::Cooldown:              cooldown(); break;
        case Action::Move:                  move(key_value); break;
        case Action::MoveXplus:             move_x_plus(); break;
        case Action::MoveXminus:            move_x_minus(); break;
        case Action::MoveYplus:             move_y_plus(); break;
        case Action::MoveYminus:            move_y_minus(); break;
        case Action::MoveZplus:             move_z_plus(); break;
        case Action::MoveZminus:            move_z_minus(); break;
        case Action::MoveEplus:             move_e_plus(); break;
        case Action::MoveEminus:            move_e_minus(); break;
        case Action::SdCard:                sd_card(key_value); break;
        case Action::SdCardSelectFile:      sd_card_select_file(key_value); break;
        case Action::ShowSettings:          show_settings(key_value); break;
        case Action::SaveSettings:          save_settings(key_value); break;
        case Action::CancelSettings:        cancel_settings(key_value); break;
        case Action::FactoryReset:          factory_reset(key_value); break;
        case Action::Leveling:              leveling(key_value); break;
        case Action::ExtruderCalibration:   extruder_calibration(key_value); break;
        case Action::XYZMotorsCalibration:  xyz_motors_calibration(key_value); break;
        case Action::PidTuning:             pid_tuning(key_value); break;
        case Action::Statistics:            statistics(key_value); break;
        case Action::About:                 about(key_value); break;
        default:                            Log::error() << F("Invalid action ") << static_cast<uint16_t>(action) << Log::endl(); break;
    }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Main
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void PrinterImpl::main(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::MainTemps:           main_temps(); break;
        case KeyValue::MainSD:              main_sd(); break;
        case KeyValue::MainControls:        main_controls(); break;
        case KeyValue::MainCalibrations:    main_calibrations(); break;
        case KeyValue::MainSettings:        main_settings(); break;
        case KeyValue::MainMotors:          main_motors(); break;
        case KeyValue::Back:                back(); break;
        default:                            Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Show one of the temperature graph screens depending of the context: either the SD printing screen,
//! the printing screen or the temperature screen.
void PrinterImpl::main_temps()
{
    set_update_graphs();

    // If there is a SD card print running, display the SD print screen
    if(card.cardOK && card.sdprinting)
    {
        show_page(Page::SdPrint);
        return;
    }

    // If there is an USB print running, display the USB Print page. Otherwise, the temperature graph page
    show_page(print_job_timer.isRunning() ? Page::UsbPrint : Page::Temperature);
}

//! Show one of the SD card screens depending of the context: either the SD screen or the SD printing screen.
//! Fallback to the USB printing if the SD card is not accessible.
void PrinterImpl::main_sd()
{
    // If there is a SD card print running, display the SD print screen
    if(card.cardOK && card.sdprinting)
    {
        show_page(Page::SdPrint);
        set_update_graphs();
        return;
    }

    if(print_job_timer.isRunning())
    {
        show_page(Page::UsbPrint);
        set_update_graphs();
        return;
    }        

    // Try to initialize the SD card
    card.initsd();
    if(!card.cardOK)
    {
        // SD card not accessible so fall back to Temperatures
        show_page(Page::Temperature);
        set_update_graphs();
        return;
    }

    show_sd_files(card.getnrfilenames() - 1);
    show_page(Page::SdCard);
}

void PrinterImpl::main_controls()
{
    show_page(Page::Controls);
}

void PrinterImpl::main_calibrations()
{
    show_page(Page::Calibration);
}

void PrinterImpl::main_settings()
{
    show_page(Page::Settings);
}

void PrinterImpl::main_motors()
{
    show_page(Page::MotorsSettings);
}

void PrinterImpl::back()
{
    show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// SD Card
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Show the list of files on SD.
//! @param last_index   Index of the last file to display
void PrinterImpl::show_sd_files(uint16_t last_index)
{
    WriteRamDataRequest frame{Variable::FileName1};

    last_file_index_ = last_index;
    String name;
    for(uint8_t index = 0; index < nb_visible_sd_files; ++index)
    {
        get_file_name(index, name);
        frame << FixedSizeString(name, 26); // Important to truncate, there is only space for 26 chars
    }
    frame.send(true);
}

//! Get a filename with a given index.
//! @param index    Index of the filename
//! @param name     Copy the filename into this Chars
void PrinterImpl::get_file_name(uint8_t index, String& name)
{
    card.getfilename(last_file_index_ - index);
    name = card.longFilename;
};

//! Select a filename as sent by the LCD screen.
//! @param key_value    The index of the filename to select
void PrinterImpl::sd_card_select_file(KeyValue key_value)
{
    if(!card.cardOK)
        return;

    auto file_index = static_cast<uint16_t>(key_value);
    if(file_index > last_file_index_)
        return;
    card.getfilename(last_file_index_ - file_index);
    FixedSizeString name{card.longFilename, 26};
    if(name.length() <= 0) // If the SD card is not readable
        return;

    LCDImpl::instance().set_progress_name(card.longFilename);

    WriteRamDataRequest frame{Variable::SelectedFileName};
    frame << name;
    frame.send(true);

    card.openFile(card.filename, true);
    card.startFileprint();
    print_job_timer.start();

    stepper.finish_and_disable(); // To circumvent homing problems

    show_page(Page::SdPrint);
    set_update_graphs();
}

//! LCD SD card menu
void PrinterImpl::sd_card(KeyValue key_value)
{
    if(!card.cardOK)
        return;

    uint16_t nb_files = card.getnrfilenames();
    if(nb_files <= nb_visible_sd_files)
        return;

    auto last_file_index = last_file_index_;

    switch(key_value)
    {
        case KeyValue::SdUp:
            if((last_file_index + nb_visible_sd_files) < nb_files)
                last_file_index += nb_visible_sd_files;
            break;

        case KeyValue::SdDown:
            if(last_file_index >= nb_visible_sd_files)
                last_file_index -= nb_visible_sd_files;
            break;

        default:
            break;
    }

    show_sd_files(last_file_index);
};

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// SD card printing commands
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Handle print commands.
//! @param key_value    The sub-action to handle
void PrinterImpl::sd_print_command(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::PrintStop:           sd_print_stop(); break;
        case KeyValue::PrintPause:          sd_print_pause(); break;
        case KeyValue::PrintResume:         sd_print_resume(); break;
        case KeyValue::Back:                sd_print_back(); break;
        default:                            Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Stop SD printing
void PrinterImpl::sd_print_stop()
{
    Log::log() << F("Stop Print") << Log::endl();

    LCDImpl::instance().reset_progress();

    card.stopSDPrint();
    clear_command_queue();
    quickstop_stepper();
    print_job_timer.stop();
    thermalManager.disable_all_heaters();

    show_back_page();
}

//! Pause SD printing
void PrinterImpl::sd_print_pause()
{
    Log::log() << F("Pause Print") << Log::endl();

    LCD::queue_message(F("Pause printing..."));
    card.pauseSDPrint();
    print_job_timer.pause();
#if ENABLED(PARK_HEAD_ON_PAUSE)
    enqueue_and_echo_commands_P(PSTR("M125"));
#endif
}

//! Resume the current SD printing
void PrinterImpl::sd_print_resume()
{
    Log::log() << F("Resume Print") << Log::endl();

    LCD::queue_message(F("Resume printing"));
#if ENABLED(PARK_HEAD_ON_PAUSE)
    enqueue_and_echo_commands_P(PSTR("M24"));
#else
    card.startFileprint();
    print_job_timer.start();
#endif
}

//! Handle the Back button
void PrinterImpl::sd_print_back()
{
    show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// USB printing commands
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Handle print commands.
//! @param key_value    The sub-action to handle
void PrinterImpl::usb_print_command(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::PrintStop:           usb_print_stop(); break;
        case KeyValue::PrintPause:          usb_print_pause(); break;
        case KeyValue::PrintResume:         usb_print_resume(); break;
        case KeyValue::Back:                usb_print_back(); break;
        default:                            Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Stop SD printing
void PrinterImpl::usb_print_stop()
{
    Log::log() << F("Stop Print") << Log::endl();

    LCDImpl::instance().reset_progress();

    clear_command_queue();
    quickstop_stepper();
    print_job_timer.stop();
    thermalManager.disable_all_heaters();

    show_back_page();
}

//! Pause SD printing
void PrinterImpl::usb_print_pause()
{
    Log::log() << F("Pause Print") << Log::endl();

    LCD::queue_message(F("Pause printing"));

    print_job_timer.pause();
#if ENABLED(PARK_HEAD_ON_PAUSE)
    enqueue_and_echo_commands_P(PSTR("M125"));
#endif
}

//! Resume the current SD printing
void PrinterImpl::usb_print_resume()
{
    Log::log() << F("Resume Print") << Log::endl();

    LCD::queue_message(F("Resume printing"));

#if ENABLED(PARK_HEAD_ON_PAUSE)
    enqueue_and_echo_commands_P(PSTR("M24"));
#else
    print_job_timer.start();
#endif
}

//! Handle the Back button
void PrinterImpl::usb_print_back()
{
    show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Target temperature
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Set the target temperature on the LCD screens
//! @param temperature  The temperature to set
void PrinterImpl::set_target_temperature(uint16_t temperature)
{
    WriteRamDataRequest frame{Variable::TargetTemperature};
    frame << Uint16(temperature);
    frame.send();
}

//! Get the target temperature set on the LCD screen
//! @return     The temperature
uint16_t PrinterImpl::PrinterImpl::get_target_temperature()
{
    ReadRamDataRequest frame{Variable::TargetTemperature, 1};
    frame.send();

    ReadRamDataResponse response;
    if(!response.receive(frame))
    {
        Log::error() << F("Receiving Frame (Target Temperature)") << Log::endl();
        return 0;
    }

    Uint16 hotend;
    response >> hotend;
    return hotend.word;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Load and Unload Filament
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Handle Load & Unload actions.
//! @param key_value    The sub-action to handle
void PrinterImpl::load_unload(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::LoadUnload:          load_unload_show(); break;
        case KeyValue::Load:                load_unload_start(true); break;
        case KeyValue::Unload:              load_unload_start(false); break;
        case KeyValue::Back:                load_unload_stop(); break;
        default:                            Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Show the Load & Unload screen on the LCD display.
void PrinterImpl::load_unload_show()
{
    set_target_temperature(200);
    show_page(Page::LoadUnload);
}

//! Start Load or Unload action.
//! @param load    Which action to start (i.e. which screen to display)
void PrinterImpl::load_unload_start(bool load)
{
    auto hotend = get_target_temperature();
    if(hotend <= 0)
        return;

    thermalManager.setTargetHotend(hotend, 0);
    enqueue_and_echo_commands_P(PSTR("G91")); // relative mode

    set_background_task(load ? BackgroundTask::LoadFilament : BackgroundTask::UnloadFilament);
    show_page(load ? Page::Load2 : Page::Unload2);
}

//! Handle back from the Load on Unload LCD screen.
void PrinterImpl::load_unload_stop()
{
    Log::log() << F("Load/Unload Stop");

    clear_background_task();
    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G90")); // absolute mode
    thermalManager.setTargetHotend(0, 0);

    show_back_page();
}

//! Load the filament if the temperature is high enough.
void PrinterImpl::load_filament_task()
{
    if(thermalManager.current_temperature[0] >= thermalManager.target_temperature[0] - 10)
    {
        Log::log() << F("Load Filament") << Log::endl();
        enqueue_and_echo_commands_P(PSTR("G1 E1 F120"));
    }
    set_next_background_task_time();
}

//! Unload the filament if the temperature is high enough.
void PrinterImpl::unload_filament_task()
{
    if(thermalManager.current_temperature[0] >= thermalManager.target_temperature[0] - 10)
    {
        Log::log() << F("Unload Filament") << Log::endl();
        enqueue_and_echo_commands_P(PSTR("G1 E-1 F120"));
    }
    set_next_background_task_time();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Preheat & Cooldown
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Handle Preheat actions.
//! @param key_value    Sub-action to handle
void PrinterImpl::preheat(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::PreheatShow:     preheat_show(); break;
        case KeyValue::Back:            preheat_back(); break;
        default:                        preheat_preset(static_cast<uint16_t>(key_value)); break;
    }
}

//! Show the preheat screen
void PrinterImpl::preheat_show()
{
    Log::log() << F("Preheat page") << Log::endl();
    WriteRamDataRequest frame{Variable::Preset1Bed};
    for(auto& preset : presets_)
        frame << Uint16(preset.hotend) << Uint16(preset.bed);
    frame.send();
    show_page(Page::Preheat);
}

void PrinterImpl::preheat_back()
{
    show_back_page();
}

//! Preheat the nozzle and save the presets.
//! @param key_value    The index (starting from 1) of the preset to use
void PrinterImpl::preheat_preset(uint16_t presetIndex)
{
    Log::log() << F("Preheat Start") << Log::endl();

    presetIndex -= 1;
    if(presetIndex >= NB_PRESETS)
    {
        Log::error() << F("Invalid preset # ") << presetIndex << Log::endl();
        return;
    }

    ReadRamDataRequest frame{Variable::Preset1Bed, 6};
    frame.send();

    ReadRamDataResponse response;
    if(!response.receive(frame))
    {
        Log::error() << F("Receiving Frame (Presets)") << Log::endl();
        return;
    }

    Uint16 hotend, bed;
    for(auto& preset : presets_)
    {
        response >> hotend >> bed;
        preset.hotend = hotend.word;
        preset.bed = bed.word;
    }

    // Save presets
    enqueue_and_echo_commands_P(PSTR("M500"));

    const Preset& preset = presets_[presetIndex];

    String command;

    command = F("M104 S"); command << preset.hotend;
    enqueue_and_echo_command(command.c_str());

    command = F("M140 S"); command << preset.bed;
    enqueue_and_echo_command(command.c_str());

    show_page(Page::Temperature);
    set_update_graphs();
}

//! Cooldown the bed and the nozzle
void PrinterImpl::cooldown()
{
    Log::log() << F("Cooldown") << Log::endl();
    LCD::reset_message();
    thermalManager.disable_all_heaters();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Move & Home
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Execute a move command
void PrinterImpl::move(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::MoveShow:            show_move(); break;
        case KeyValue::HomeX:               home_x(); break;
        case KeyValue::HomeY:               home_y(); break;
        case KeyValue::HomeZ:               home_z(); break;
        case KeyValue::HomeAll:             home_all(); break;
        case KeyValue::DisableMotors:       disable_motors(); break;
        case KeyValue::Back:                move_back(); break;
        default:                            Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void PrinterImpl::show_move()
{
    stepper.finish_and_disable(); // To circumvent homing problems
    show_page(Page::Move);
}

void PrinterImpl::move_back()
{
    show_back_page();
}

//! Move the nozzle.
void PrinterImpl::move_x_plus()
{
    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 X5 F3000"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Move the nozzle.
void PrinterImpl::move_x_minus()
{
    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 X-5 F3000"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Move the nozzle.
void PrinterImpl::move_y_plus()
{
    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 Y5 F3000"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Move the nozzle.
void PrinterImpl::move_y_minus()
{
    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 Y-5 F3000"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Move the nozzle.
void PrinterImpl::move_z_plus()
{
    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0.5 F3000"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Move the nozzle.
void PrinterImpl::move_z_minus()
{
    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 Z-0.5 F3000"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Extrude some filament.
void PrinterImpl::move_e_plus()
{
    if(thermalManager.degHotend(0) < 180)
        return;

    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 E1 F120"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Unextrude.
void PrinterImpl::move_e_minus()
{
    if(thermalManager.degHotend(0) < 180)
        return;

    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 E-1 F120"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Disable the motors.
void PrinterImpl::disable_motors()
{
    enqueue_and_echo_commands_P(PSTR("M84"));
    axis_homed[X_AXIS] = axis_homed[Y_AXIS] = axis_homed[Z_AXIS] = false;
    axis_known_position[X_AXIS] = axis_known_position[Y_AXIS] = axis_known_position[Z_AXIS] = false;
}

//! Go to home on the X axis.
void PrinterImpl::home_x()
{
    enqueue_and_echo_commands_P(PSTR("G28 X0"));
}

//! Go to home on the Y axis.
void PrinterImpl::home_y()
{
    enqueue_and_echo_commands_P(PSTR("G28 Y0"));
}

//! Go to home on the Z axis.
void PrinterImpl::home_z()
{
    enqueue_and_echo_commands_P(PSTR("G28 Z0"));
}

//! Go to home on all axis.
void PrinterImpl::home_all()
{
    enqueue_and_echo_commands_P(PSTR("G28"));
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Settings
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Show a set of settings
//! @param key_value    The set to display
void PrinterImpl::show_settings(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::SettingsPrint:           show_print_settings(); break;
        case KeyValue::SettingsPID:             show_pid_settings(true); break;
        case KeyValue::SettingsSteps:           show_steps_settings(); break;
        case KeyValue::SettingsFeedrate:        show_feedrate_settings(); break;
        case KeyValue::SettingsAcceleration:    show_acceleration_settings(); break;
        case KeyValue::SettingsJerk:            show_jerk_settings(); break;
        default:                                Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Save a set of settings
//! @param key_value    The set to save
void PrinterImpl::save_settings(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::SettingsPrint:           save_print_settings(); break;
        case KeyValue::SettingsPID:             save_pid_settings(); break;
        case KeyValue::SettingsSteps:           save_steps_settings(); break;
        case KeyValue::SettingsFeedrate:        save_feedrate_settings(); break;
        case KeyValue::SettingsAcceleration:    save_acceleration_settings(); break;
        case KeyValue::SettingsJerk:            save_jerk_settings(); break;
        default:                                Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Cancel settings
void PrinterImpl::cancel_settings(KeyValue key_value)
{
    if(key_value == KeyValue::SettingsPID)
        old_pid_.save(); // Restore old values
    show_back_page();
}

//! Display on the LCD screen the printing settings.
void PrinterImpl::show_print_settings()
{
    WriteRamDataRequest frame{Variable::PrintSettingsSpeed};
    frame << Uint16(feedrate_percentage)
          << Uint16(thermalManager.degTargetHotend(0))
          << Uint16(thermalManager.degTargetBed())
          << Uint16(scale(fanSpeeds[0], 255, 100));
    frame.send();

    save_forward_page();
    show_page(Page::PrintSettings);
}

//! Save the printing settings.
void PrinterImpl::save_print_settings()
{
    ReadRamDataRequest frame{Variable::PrintSettingsSpeed, 4};
    frame.send();

    ReadRamDataResponse response;
    if(!response.receive(frame))
    {
        Log::error() << F("Receiving Frame (Print Settings)") << Log::endl();
        return;
    }

    Uint16 speed, hotend, bed, fan;
    response >> speed >> hotend >> bed >> fan;

    feedrate_percentage = speed.word;
    thermalManager.setTargetHotend(hotend.word, 0);
    thermalManager.setTargetBed(bed.word);
    fanSpeeds[0] = scale(fan.word, 100, 255);

    show_forward_page();
}

//! Show the PID settings
void PrinterImpl::show_pid_settings(bool back, bool init)
{
    if(init)
    {
        old_pid_.init();
        save_forward_page();
    }        

    WriteRamDataRequest frame{Variable::PidP};
    frame << Uint16(Temperature::Kp * 100)
          << Uint16(unscalePID_i(Temperature::Ki) * 100)
          << Uint16(unscalePID_d(Temperature::Kd) * 100);
    frame.send();

    show_page(Page::PidSettings, back);
}

//! Save the PID settings
void PrinterImpl::save_pid_settings()
{
    ReadRamDataRequest frame{Variable::PidP, 3};
    frame.send();

    ReadRamDataResponse response;
    if(!response.receive(frame))
    {
        Log::error() << F("Receiving Frame (PID Settings)") << Log::endl();
        return;
    }

    Uint16 p, i, d;
    response >> p >> i >> d;

    Temperature::Kp = static_cast<float>(p.word) / 100;
    Temperature::Ki = scalePID_i(static_cast<float>(i.word) / 100);
    Temperature::Kd = scalePID_d(static_cast<float>(d.word) / 100);

    enqueue_and_echo_commands_P(PSTR("M500"));

    show_forward_page();
}

//! Show the Steps settings
//! @param init     Initialize the settings are use those already set
void PrinterImpl::show_steps_settings(bool init)
{
    if(init)
    {
        steps_.init();
        save_forward_page();
    }        

    WriteRamDataRequest frame{Variable::StepSettingsX};
    frame << Uint16(steps_.axis_steps_per_mm[X_AXIS] * 10)
          << Uint16(steps_.axis_steps_per_mm[Y_AXIS] * 10)
          << Uint16(steps_.axis_steps_per_mm[Z_AXIS] * 10)
          << Uint16(steps_.axis_steps_per_mm[E_AXIS] * 10);
    frame.send();

    show_page(Page::StepsSettings);
}

//! Save the Steps settings
void PrinterImpl::save_steps_settings()
{
    ReadRamDataRequest frame{Variable::StepSettingsX, 4};
    frame.send();

    ReadRamDataResponse response;
    if(!response.receive(frame))
    {
        Log::error() << F("Receiving Frame (Steps Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z, e;
    response >> x >> y >> z >> e;

    steps_.axis_steps_per_mm[X_AXIS] = static_cast<float>(x.word) / 10;
    steps_.axis_steps_per_mm[Y_AXIS] = static_cast<float>(y.word) / 10;
    steps_.axis_steps_per_mm[Z_AXIS] = static_cast<float>(z.word) / 10;
    steps_.axis_steps_per_mm[E_AXIS] = static_cast<float>(e.word) / 10;
    steps_.save();

    show_forward_page();
}

//! Show the Feedrate settings
//! @param init     Initialize the settings are use those already set
void PrinterImpl::show_feedrate_settings(bool init)
{
    if(init)
    {    
        feedrates_.init();
        save_forward_page();
    }        

    WriteRamDataRequest frame{Variable::FeedrateMaxX};
    frame << Uint16(feedrates_.max_feedrate_mm_s[X_AXIS])
          << Uint16(feedrates_.max_feedrate_mm_s[Y_AXIS])
          << Uint16(feedrates_.max_feedrate_mm_s[Z_AXIS])
          << Uint16(feedrates_.max_feedrate_mm_s[E_AXIS])
          << Uint16(feedrates_.min_feedrate_mm_s)
          << Uint16(feedrates_.min_travel_feedrate_mm_s);
    frame.send();

    show_page(Page::FeedrateSettings);
}

//! Save the Feedrate settings
void PrinterImpl::save_feedrate_settings()
{
    ReadRamDataRequest frame{Variable::FeedrateMaxX, 6};
    frame.send();

    ReadRamDataResponse response;
    if(!response.receive(frame))
    {
        Log::error() << F("Receiving Frame (Feedrate Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z, e, min, travel;
    response >> x >> y >> z >> e >> min >> travel;

    feedrates_.max_feedrate_mm_s[X_AXIS] = static_cast<float>(x.word);
    feedrates_.max_feedrate_mm_s[Y_AXIS] = static_cast<float>(y.word);
    feedrates_.max_feedrate_mm_s[Z_AXIS] = static_cast<float>(z.word);
    feedrates_.max_feedrate_mm_s[E_AXIS] = static_cast<float>(e.word);
    feedrates_.min_feedrate_mm_s         = static_cast<float>(min.word);
    feedrates_.min_travel_feedrate_mm_s  = static_cast<float>(travel.word);
    feedrates_.save();

    show_forward_page();
}

//! Show the Acceleration settings
//! @param init     Initialize the settings are use those already set
void PrinterImpl::show_acceleration_settings(bool init)
{
    if(init)
    {
        accelerations_.init();
        save_forward_page();
    }        

    WriteRamDataRequest frame{Variable::AccelerationMaxX};
    frame << Uint16(static_cast<uint16_t>(accelerations_.max_acceleration_mm_per_s2[X_AXIS]))
          << Uint16(static_cast<uint16_t>(accelerations_.max_acceleration_mm_per_s2[Y_AXIS]))
          << Uint16(static_cast<uint16_t>(accelerations_.max_acceleration_mm_per_s2[Z_AXIS]))
          << Uint16(static_cast<uint16_t>(accelerations_.max_acceleration_mm_per_s2[E_AXIS]))
          << Uint16(static_cast<uint16_t>(accelerations_.acceleration))
          << Uint16(static_cast<uint16_t>(accelerations_.retract_acceleration))
          << Uint16(static_cast<uint16_t>(accelerations_.travel_acceleration));
    frame.send();

    show_page(Page::AccelerationSettings);
}

//! Save the Acceleration settings
void PrinterImpl::save_acceleration_settings()
{
    ReadRamDataRequest frame{Variable::AccelerationMaxX, 7};
    frame.send();

    ReadRamDataResponse response;
    if(!response.receive(frame))
    {
        Log::error() << F("Receiving Frame (Acceleration Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z, e, print, retract, travel;
    response >> x >> y >> z >> e >> print >> retract >> travel;

    accelerations_.max_acceleration_mm_per_s2[X_AXIS] = static_cast<uint32_t>(x.word);
    accelerations_.max_acceleration_mm_per_s2[Y_AXIS] = static_cast<uint32_t>(y.word);
    accelerations_.max_acceleration_mm_per_s2[Z_AXIS] = static_cast<uint32_t>(z.word);
    accelerations_.max_acceleration_mm_per_s2[E_AXIS] = static_cast<uint32_t>(e.word);
    accelerations_.acceleration                       = static_cast<float>(print.word);
    accelerations_.retract_acceleration               = static_cast<float>(retract.word);
    accelerations_.travel_acceleration                = static_cast<float>(travel.word);
    accelerations_.save();

    show_forward_page();
}

//! Show the Jerk settings
//! @param init     Initialize the settings are use those already set
void PrinterImpl::show_jerk_settings(bool init)
{
    if(init)
    {
        jerks_.init();
        save_forward_page();
    }        

    WriteRamDataRequest frame{Variable::JerkX};
    frame << Uint16(jerks_.max_jerk[X_AXIS] * 10)
          << Uint16(jerks_.max_jerk[Y_AXIS] * 10)
          << Uint16(jerks_.max_jerk[Z_AXIS] * 10)
          << Uint16(jerks_.max_jerk[E_AXIS] * 10);
    frame.send();

    show_page(Page::JerkSettings);
}

//! Save the Jerk settings
void PrinterImpl::save_jerk_settings()
{
    ReadRamDataRequest frame{Variable::JerkX, 4};
    frame.send();

    ReadRamDataResponse response;
    if(!response.receive(frame))
    {
        Log::error() << F("Receiving Frame (Acceleration Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z, e;
    response >> x >> y >> z >> e;

    jerks_.max_jerk[X_AXIS] = static_cast<uint32_t>(x.word) / 10;
    jerks_.max_jerk[Y_AXIS] = static_cast<uint32_t>(y.word) / 10;
    jerks_.max_jerk[Z_AXIS] = static_cast<uint32_t>(z.word) / 10;
    jerks_.max_jerk[E_AXIS] = static_cast<uint32_t>(e.word) / 10;
    jerks_.save();

    show_forward_page();
}

//! Reset all settings of the printer to factory ones.
void PrinterImpl::factory_reset(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::ResetShow:               show_factory_reset_warning(); break;
        case KeyValue::ResetConfirm:            do_factory_reset(); break;
        case KeyValue::Cancel:                  cancel_factory_reset(); break;
        default:                                Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void PrinterImpl::show_factory_reset_warning()
{
    show_page(Page::FactoryReset);
}

void PrinterImpl::do_factory_reset()
{
    enqueue_and_echo_commands_P(PSTR("M502"));
    enqueue_and_echo_commands_P(PSTR("M500"));
    show_back_page();
}

void PrinterImpl::cancel_factory_reset()
{
    show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Statistics
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Display statistics on the LCD screen.
void PrinterImpl::statistics(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::StatisticsShow:          show_stats(); break;
        case KeyValue::Back:                    stats_back(); break;
        default:                                Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void PrinterImpl::show_stats()
{
    send_stats();
    show_page(Page::Statistics);
}

void PrinterImpl::stats_back()
{
    show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// About & Versions
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Get the current LCD firmware version.
//! @return     The version as a string.
String PrinterImpl::get_lcd_firmware_version()
{
    ReadRegisterDataRequest frame{Register::Version, 1};
    frame.send();

    ReadRegisterDataResponse response;
    if(!response.receive(frame))
    {
        Log::error() << F("Receiving Frame (Version)") << Log::endl();
        return F("Unknown");
    }

    Uint8 version; response >> version;
    String lcd_version; lcd_version << (version.byte / 0x10) << "." << (version.byte % 0x10);
    Log::log() << F("LCD Firmware raw version = ") << version.byte << Log::endl();
    return lcd_version;
}

//! Convert a version from its hexadecimal representation.
//! @param hex_version  Hexadecimal representation of the version
//! @return             Version as a string
String convert_version(uint16_t hex_version)
{
    String version;
    version << hex_version / 0x0100 << "." << (hex_version % 0x100) / 0x10 << "." << hex_version % 0x10;
    return version;
}

//! Send the different versions to the LCD screen.
void PrinterImpl::send_versions()
{
    String marlin_version{SHORT_BUILD_VERSION};
    String motherboard_version = convert_version(advi3_pp_version);
    String advi3pp_lcd_version = convert_version(adv_i3_pp_lcd_version_);
    String lcd_firmware_version = get_lcd_firmware_version();

    WriteRamDataRequest frame{Variable::MarlinVersion};
    frame << FixedSizeString(marlin_version, 16)
          << FixedSizeString(motherboard_version, 16)
          << FixedSizeString(advi3pp_lcd_version, 16)
          << FixedSizeString(lcd_firmware_version, 16);
    frame.send();
}

bool PrinterImpl::is_version_valid() const
{
    return adv_i3_pp_lcd_version_ >= advi3_pp_oldest_lcd_compatible_version  && adv_i3_pp_lcd_version_ <= advi3_pp_newest_lcd_compatible_version;
}

//! Display the About screen,
void PrinterImpl::about(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::AboutForward:            about_forward(); break;
        case KeyValue::Back:                    about_back(); break;
        default:                                show_about(static_cast<uint16_t>(key_value)); break;
    }
}

void PrinterImpl::about_forward()
{
    show_page(Page::About, false);
}

void PrinterImpl::about_back()
{
    show_back_page();
}

void PrinterImpl::show_about(uint16_t version)
{
    adv_i3_pp_lcd_version_ = version;
    send_versions();
    show_page(is_version_valid() ? Page::About : Page::Mismatch);    
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// PID Tuning
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Handle PID tuning.
//! @param key_value    The step of the PID tuning
void PrinterImpl::pid_tuning(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::PidTuningStep1:   pid_tuning_step1(); break;
        case KeyValue::PidTuningStep2:   pid_tuning_step2(); break;
        case KeyValue::Cancel:           pid_tuning_cancel(); break;
        default:                         Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Show step #1 of PID tuning
void PrinterImpl::pid_tuning_step1()
{
    old_pid_.init();
    set_target_temperature(200);
    save_forward_page();
    show_page(Page::PidTuning1);
}

//! Show step #2 of PID tuning
void PrinterImpl::pid_tuning_step2()
{
    auto hotend = get_target_temperature();
    if(hotend <= 0)
        return;

    enqueue_and_echo_commands_P(PSTR("M106 S255")); // Turn on fan
    String auto_pid_command; auto_pid_command << F("M303 S") << hotend << F("E0 C8 U1");
    enqueue_and_echo_command(auto_pid_command.c_str());

    show_page(Page::PidTuning2);
};

//! PID automatic tuning is finished.
void PrinterImpl::auto_pid_finished()
{
    Log::log() << F("Auto PID finished") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("M106 S0"));
    show_pid_settings(false, false);
}

void PrinterImpl::pid_tuning_cancel()
{
    show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Leveling
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void PrinterImpl::leveling(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::LevelingHome:    leveling_home(); break;
        case KeyValue::LevelingPoint1:  leveling_point1(); break;
        case KeyValue::LevelingPoint2:  leveling_point2(); break;
        case KeyValue::LevelingPoint3:  leveling_point3(); break;
        case KeyValue::LevelingPoint4:  leveling_point4(); break;
        case KeyValue::LevelingPoint5:  leveling_point5(); break;
        case KeyValue::Back:            leveling_finish(); break;
        default:                        Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Home the printer for bed leveling.
void PrinterImpl::leveling_home()
{
    show_page(Page::Leveling1);
    axis_homed[X_AXIS] = axis_homed[Y_AXIS] = axis_homed[Z_AXIS] = false;
    axis_known_position[X_AXIS] = axis_known_position[Y_AXIS] = axis_known_position[Z_AXIS] = false;
    enqueue_and_echo_commands_P(PSTR("G90")); // absolute mode
    enqueue_and_echo_commands_P((PSTR("G28"))); // homing
    set_background_task(BackgroundTask::Leveling);
}

//! Leveling Background task.
void PrinterImpl::leveling_task()
{
    if(axis_homed[X_AXIS] && axis_homed[Y_AXIS] && axis_homed[Z_AXIS])
    {
        Log::log() << F("Leveling Homed, start process") << Log::endl();
        clear_background_task();
        show_page(Page::Leveling2, false);
    }
    else
        set_next_background_task_time(200);
}

//! Handle leveling point #1.
void PrinterImpl::leveling_point1()
{
    Log::log() << F("Level step 1") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X30 Y30 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #2.
void PrinterImpl::leveling_point2()
{
    Log::log() << F("Level step 2") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X170 Y170 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #3.
void PrinterImpl::leveling_point3()
{
    Log::log() << F("Level step 3") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X170 Y30 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #4.
void PrinterImpl::leveling_point4()
{
    Log::log() << F("Level step 4") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X30 Y170 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #5.
void PrinterImpl::leveling_point5()
{
    Log::log() << F("Level step 5") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X100 Y100 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #4.
void PrinterImpl::leveling_finish()
{
    enqueue_and_echo_commands_P(PSTR("G1 Z30 F2000"));
    show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Extruder calibration
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void PrinterImpl::extruder_calibration(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::CalibrationShow:         show_extruder_calibration(); break;
        case KeyValue::CalibrationStart:        start_extruder_calibration(); break;
        case KeyValue::CalibrationSettings:     extruder_calibrartion_settings(); break;
        case KeyValue::Cancel:                  cancel_extruder_calibration(); break;
        default:                                Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Show the extruder calibration screen.
void PrinterImpl::show_extruder_calibration()
{
    set_target_temperature(200);

    WriteRamDataRequest frame{Variable::Measure1};
    frame << 200_u16;
    frame.send();

    save_forward_page();
    show_page(Page::ExtruderCalibration1);
}

//! Start extruder calibration.
void PrinterImpl::start_extruder_calibration()
{
    auto hotend = get_target_temperature();
    if(hotend <= 0)
        return;

    thermalManager.setTargetHotend(hotend, 0);
    set_background_task(BackgroundTask::ExtruderCalibration);
    enqueue_and_echo_commands_P(PSTR("M83"));       // relative E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));    // reset E axis

    show_page(Page::ExtruderCalibration2, false);
}

//! Extruder calibration background task.
void PrinterImpl::extruder_calibration_task()
{
    if(current_position[E_AXIS] >= calibration_extruder_filament)
    {
        extruder_calibration_finished();
        return;
    }

    if(thermalManager.current_temperature[0] < thermalManager.target_temperature[0] - 10)
    {
        set_next_background_task_time();
        return;
    }

    enqueue_and_echo_commands_P(PSTR("G1 E1 F100")); // Extrude 1mm slowly
    set_next_background_task_time();
}

//! Record the amount of filament extruded.
void PrinterImpl::extruder_calibration_finished()
{
    extruded_ = current_position[E_AXIS];
    Log::log() << F("Filament extruded ") << extruded_ << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G82"));       // absolute E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));    // reset E axis

    clear_background_task();
    show_page(Page::ExtruderCalibration3);
}

//! Compute the extruder (E axis) new value and show the steps settings.
void PrinterImpl::extruder_calibrartion_settings()
{
    ReadRamDataRequest frame{Variable::Measure1, 1};
    frame.send();

    ReadRamDataResponse response;
    if(!response.receive(frame))
    {
        Log::error() << F("Receiving Frame (Measures)") << Log::endl();
        return;
    }

    Uint16 e; response >> e;
    e.word /= 10;

    // Fill all values because all 4 axis are displayed by  show_steps_settings
    steps_.axis_steps_per_mm[X_AXIS] = planner.axis_steps_per_mm[X_AXIS];
    steps_.axis_steps_per_mm[Y_AXIS] = planner.axis_steps_per_mm[Y_AXIS];
    steps_.axis_steps_per_mm[Z_AXIS] = planner.axis_steps_per_mm[Z_AXIS];
	steps_.axis_steps_per_mm[E_AXIS] = planner.axis_steps_per_mm[E_AXIS] * extruded_ / (extruded_ + calibration_extruder_delta - e.word);
    Log::log() << F("Adjust: old = ") << planner.axis_steps_per_mm[E_AXIS] << F(", expected = ") << extruded_ << F(", measured = ") << (extruded_ + calibration_extruder_delta - e.word) << F(", new = ") << steps_.axis_steps_per_mm[E_AXIS] << Log::endl();

    show_steps_settings(false);
}

//! Cancel the extruder calibration.
void PrinterImpl::cancel_extruder_calibration()
{
    clear_background_task();

    thermalManager.setTargetHotend(0, 0);
    thermalManager.setTargetBed(0);

    enqueue_and_echo_commands_P(PSTR("G82"));       // absolute E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));    // reset E axis

    show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// XYZ Motors calibration
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void PrinterImpl::xyz_motors_calibration(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::CalibrationShow:         show_xyz_motors_calibration(); break;
        case KeyValue::CalibrationSettings:     xyz_motors_calibration_settings(); break;
        case KeyValue::Cancel:                  cancel_xyz_motors_calibration(); break;
        default:                                Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void PrinterImpl::show_xyz_motors_calibration()
{
    WriteRamDataRequest frame{Variable::Measure1};
    frame << 200_u16 << 200_u16 << 200_u16;
    frame.send();
    save_forward_page();
    show_page(Page::XYZMotorsCalibration);
}

float adjust_value(float old, double expected, double measured)
{
	auto new_value = old * expected / measured;
    Log::log() << F("Adjust: old = ") << old << F(", expected = ") << expected << F(", measured = ") << measured << F(", new = ") << new_value << Log::endl();
    return new_value;
};

void PrinterImpl::xyz_motors_calibration_settings()
{
    ReadRamDataRequest frame{Variable::Measure1, 3};
    frame.send();

    ReadRamDataResponse response;
    if(!response.receive(frame))
    {
        Log::error() << F("Receiving Frame (Measures)") << Log::endl();
        return;
    }

    Uint16 x, y, z;
    response >> x >> y >> z;

    steps_.axis_steps_per_mm[X_AXIS] = adjust_value(planner.axis_steps_per_mm[X_AXIS], calibration_cube_size * 10, x.word);
    steps_.axis_steps_per_mm[Y_AXIS] = adjust_value(planner.axis_steps_per_mm[Y_AXIS], calibration_cube_size * 10, y.word);
    steps_.axis_steps_per_mm[Z_AXIS] = adjust_value(planner.axis_steps_per_mm[Z_AXIS], calibration_cube_size * 10, z.word);
    // Fill all values because all 4 axis are displayed by  show_steps_settings
    steps_.axis_steps_per_mm[E_AXIS] = planner.axis_steps_per_mm[E_AXIS];

    show_steps_settings(false);
}

//! Cancel the extruder calibration.
void PrinterImpl::cancel_xyz_motors_calibration()
{
    show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Status
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Send statistics to the LCD screen.
void PrinterImpl::send_stats()
{
    printStatistics stats = print_job_timer.getStats();

    WriteRamDataRequest frame{Variable::TotalPrints};
    frame << Uint16(stats.totalPrints) << Uint16(stats.finishedPrints);
    frame.send();

    duration_t duration = stats.printTime;
    frame.reset(Variable::TotalPrintTime);
    frame << FixedSizeString{duration, 16};
    frame.send();

    duration = stats.longestPrint;
    frame.reset(Variable::LongestPrintTime);
    frame << FixedSizeString{duration, 16};
    frame.send();

    String filament_used;
    filament_used << static_cast<unsigned int>(stats.filamentUsed / 1000)
                  << "."
                  << static_cast<unsigned int>(stats.filamentUsed / 100) % 10;
    frame.reset(Variable::TotalFilament);
    frame << FixedSizeString{filament_used, 16};
    frame.send();
}

void PrinterImpl::set_update_graphs()
{
    update_graphs_ = true;
    next_update_graph_time_ = millis() + 2000;
}

void PrinterImpl::update_graphs()
{
    if(!update_graphs_ || !ELAPSED(millis(), next_update_graph_time_))
        return;

    send_graphs_data();
    next_update_graph_time_ = millis() + 500;
}
    
//! Update the graphics (two channels: the bed and the hotend).
void PrinterImpl::send_graphs_data()
{
    WriteCurveDataRequest frame{0b00001111};
    frame << Uint16{thermalManager.degHotend(0)}
          << Uint16{thermalManager.degHotend(0)}
          << Uint16{thermalManager.degBed()}
          << Uint16{thermalManager.degBed()};
    frame.send(false);
}

void PrinterImpl::clear_graphs()
{
    WriteRegisterDataRequest request{Register::TrendlineClear};
    request << 0x55_u8;
    request.send();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Errors
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Display the Thermal Runaway Error screen.
void PrinterImpl::temperature_error()
{
    show_page(advi3pp::Page::ThermalRunawayError);
}

// --------------------------------------------------------------------
// PidSettings
// --------------------------------------------------------------------

//! Initialize temporary Step settings.
void PidSettings::init()
{
    Kp = Temperature::Kp;
    Ki = Temperature::Ki;
    Kd = Temperature::Kd;
}

//! Save temporary Step settings.
void PidSettings::save()
{
    Temperature::Kp = Kp;
    Temperature::Ki = Ki;
    Temperature::Kd = Kd;

    enqueue_and_echo_commands_P(PSTR("M500"));
}

// --------------------------------------------------------------------
// StepSettings
// --------------------------------------------------------------------

//! Initialize temporary Step settings.
void StepSettings::init()
{
    axis_steps_per_mm[X_AXIS] = planner.axis_steps_per_mm[X_AXIS];
    axis_steps_per_mm[Y_AXIS] = planner.axis_steps_per_mm[Y_AXIS];
    axis_steps_per_mm[Z_AXIS] = planner.axis_steps_per_mm[Z_AXIS];
    axis_steps_per_mm[E_AXIS] = planner.axis_steps_per_mm[E_AXIS];
}

//! Save temporary Step settings.
void StepSettings::save()
{
    planner.axis_steps_per_mm[X_AXIS] = axis_steps_per_mm[X_AXIS];
    planner.axis_steps_per_mm[Y_AXIS] = axis_steps_per_mm[Y_AXIS];
    planner.axis_steps_per_mm[Z_AXIS] = axis_steps_per_mm[Z_AXIS];
    planner.axis_steps_per_mm[E_AXIS] = axis_steps_per_mm[E_AXIS];

    enqueue_and_echo_commands_P(PSTR("M500"));
}

// --------------------------------------------------------------------
// FeedrateSettings
// --------------------------------------------------------------------

//! Initialize temporary Feedrate settings.
void FeedrateSettings::init()
{
    max_feedrate_mm_s[X_AXIS] = planner.max_feedrate_mm_s[X_AXIS];
    max_feedrate_mm_s[Y_AXIS] = planner.max_feedrate_mm_s[Y_AXIS];
    max_feedrate_mm_s[Z_AXIS] = planner.max_feedrate_mm_s[Z_AXIS];
    max_feedrate_mm_s[E_AXIS] = planner.max_feedrate_mm_s[E_AXIS];
    min_feedrate_mm_s = planner.min_feedrate_mm_s;
    min_travel_feedrate_mm_s = planner.min_travel_feedrate_mm_s;
}

//! Save temporary Feedrate settings.
void FeedrateSettings::save()
{
    planner.max_feedrate_mm_s[X_AXIS] = max_feedrate_mm_s[X_AXIS];
    planner.max_feedrate_mm_s[Y_AXIS] = max_feedrate_mm_s[Y_AXIS];
    planner.max_feedrate_mm_s[Z_AXIS] = max_feedrate_mm_s[Z_AXIS];
    planner.max_feedrate_mm_s[E_AXIS] = max_feedrate_mm_s[E_AXIS];
    planner.min_feedrate_mm_s = min_feedrate_mm_s;
    planner.min_travel_feedrate_mm_s = min_travel_feedrate_mm_s;

    enqueue_and_echo_commands_P(PSTR("M500"));
}

// --------------------------------------------------------------------
// AccelerationSettings
// --------------------------------------------------------------------

//! Initialize temporary Acceleration settings.
void AccelerationSettings::init()
{
    max_acceleration_mm_per_s2[X_AXIS] = planner.max_acceleration_mm_per_s2[X_AXIS];
    max_acceleration_mm_per_s2[Y_AXIS] = planner.max_acceleration_mm_per_s2[Y_AXIS];
    max_acceleration_mm_per_s2[Z_AXIS] = planner.max_acceleration_mm_per_s2[Z_AXIS];
    max_acceleration_mm_per_s2[E_AXIS] = planner.max_acceleration_mm_per_s2[E_AXIS];
    acceleration = planner.acceleration;
    retract_acceleration = planner.retract_acceleration;
    travel_acceleration = planner.travel_acceleration;
}

//! Save temporary Acceleration settings.
void AccelerationSettings::save()
{
    planner.max_acceleration_mm_per_s2[X_AXIS] = max_acceleration_mm_per_s2[X_AXIS];
    planner.max_acceleration_mm_per_s2[Y_AXIS] = max_acceleration_mm_per_s2[Y_AXIS];
    planner.max_acceleration_mm_per_s2[Z_AXIS] = max_acceleration_mm_per_s2[Z_AXIS];
    planner.max_acceleration_mm_per_s2[E_AXIS] = max_acceleration_mm_per_s2[E_AXIS];
    planner.acceleration = acceleration;
    planner.retract_acceleration = retract_acceleration;
    planner.travel_acceleration =  travel_acceleration;

    enqueue_and_echo_commands_P(PSTR("M500"));
}

// --------------------------------------------------------------------
// JerkSettings
// --------------------------------------------------------------------

//! Initialize temporary Jerk settings.
void JerkSettings::init()
{
    max_jerk[X_AXIS] = planner.max_jerk[X_AXIS];
    max_jerk[Y_AXIS] = planner.max_jerk[Y_AXIS];
    max_jerk[Z_AXIS] = planner.max_jerk[Z_AXIS];
    max_jerk[E_AXIS] = planner.max_jerk[E_AXIS];
}

//! Save temporary Jerk settings.
void JerkSettings::save()
{
    planner.max_jerk[X_AXIS] = max_jerk[X_AXIS];
    planner.max_jerk[Y_AXIS] = max_jerk[Y_AXIS];
    planner.max_jerk[Z_AXIS] = max_jerk[Z_AXIS];
    planner.max_jerk[E_AXIS] = max_jerk[E_AXIS];

    enqueue_and_echo_commands_P(PSTR("M500"));
}

}
