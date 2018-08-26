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
#include "planner.h"
#include "parser.h"
#include "printcounter.h"
#include "duration_t.h"

#include "advi3pp.h"
#include "advi3pp_utils.h"
#include "advi3pp_.h"

#ifdef DEBUG
#pragma message "This is a DEBUG build"
#endif

#ifdef ADVi3PP_BLTOUCH
#pragma message "This is a BLTouch build"
#endif

#include <HardwareSerial.h>
extern HardwareSerial Serial2;

// Used directly by Marlin
uint8_t progress_bar_percent;
int16_t lcd_contrast;

namespace
{
    const uint16_t advi3_pp_version = 0x400;
    const uint16_t advi3_pp_oldest_lcd_compatible_version = 0x400;
    const uint16_t advi3_pp_newest_lcd_compatible_version = 0x400;

    const unsigned long advi3_pp_baudrate = 115200; // Between the LCD panel and the mainboard
    const uint16_t nb_visible_sd_files = 5;
	const uint8_t  nb_visible_sd_file_chars = 48;
    const uint16_t tuning_extruder_filament = 100; // 10 cm
	const uint16_t tuning_extruder_delta = 20; // 2 cm

    const advi3pp::Feature DEFAULT_FEATURES =
        advi3pp::Feature::ThermalProtection |
        advi3pp::Feature::HeadParking |
        advi3pp::Feature::Dimming |
        advi3pp::Feature::Buzzer;

    const uint32_t DEFAULT_USB_BAUDRATE = BAUDRATE;

    const uint32_t usb_baudrates[] = {9600, 19200, 38400, 57600, 115200, 230400, 250000};

    const int8_t BRIGHTNESS_MIN = 0x01;
    const int8_t BRIGHTNESS_MAX = 0x40;
    const uint8_t DIMMING_RATIO = 25; // in percent
    const uint16_t DIMMING_DELAY = 1 * 60;

    const uint16_t DEFAULT_PREHEAT_PRESET[5][3] = {
        {180, 50, 0},
        {200, 60, 0},
        {220, 70, 0},
        {180, 00, 0},
        {200, 00, 0}
    };

    //! Transform a value from a scale to another one.
    //! @param value        Value to be transformed
    //! @param valueScale   Current scale of the value (maximal)
    //! @param targetScale  Target scale
    //! @return             The scaled value
    int16_t scale(int16_t value, int16_t valueScale, int16_t targetScale) { return value * targetScale / valueScale; }
}

#ifdef ADVi3PP_BLTOUCH
bool set_probe_deployed(bool);
float run_z_probe();
extern float zprobe_zoffset;
#endif


namespace advi3pp {

namespace { Printer_ printer; };

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

void Printer::g29_leveling_finished(bool success)
{
    printer.g29_leveling_finished(success);
}

//! Store presets in permanent memory.
//! @param write Function to use for the actual writing
//! @param eeprom_index
//! @param working_crc
void Printer::store_eeprom_data(eeprom_write write, int& eeprom_index, uint16_t& working_crc)
{
    printer.store_eeprom_data(write, eeprom_index, working_crc);
}

//! Restore presets from permanent memory.
//! @param read Function to use for the actual reading
//! @param eeprom_index
//! @param working_crc
void Printer::restore_eeprom_data(eeprom_read read, int& eeprom_index, uint16_t& working_crc)
{
    printer.restore_eeprom_data(read, eeprom_index, working_crc);
}

//! Reset presets.
void Printer::reset_eeprom_data()
{
    printer.reset_eeprom_data();
}

//! Return the size of data specific to ADVi3++
uint16_t Printer::size_of_eeprom_data()
{
    return printer.size_of_eeprom_data();
}

//! Inform the user that the EEPROM data are not compatible and have been reset
void Printer::eeprom_settings_mismatch()
{
    printer.eeprom_settings_mismatch();
}

void Printer::save_settings()
{
    printer.save_settings();
}

//! Called when a temperature error occurred and display the error on the LCD.
void Printer::temperature_error(const __FlashStringHelper* message)
{
    printer.temperature_error(message);
}

void Printer::update()
{
    printer.task();
}

bool Printer::is_thermal_protection_enabled()
{
    return printer.is_thermal_protection_enabled();
}

void Printer::process_command(const GCodeParser& parser)
{
    printer.process_command(parser);
}

void Printer::set_brightness(int16_t britghness)
{
    printer.set_brightness(britghness);
}

LCD_& LCD_::instance()
{
    return printer.lcd_;
}

// --------------------------------------------------------------------
// Pages management
// --------------------------------------------------------------------

PagesManager::PagesManager(Printer_& printer)
        : printer_{printer}
{
}

//! Show the given page on the LCD screen
//! @param [in] page The page to be displayed on the LCD screen
void PagesManager::show_page(Page page, bool save_back)
{
    Log::log() << F("Show page ") << static_cast<uint8_t>(page) << Log::endl();

    if(save_back)
        back_pages_.push(get_current_page());

    WriteRegisterDataRequest frame{Register::PictureID};
    frame << 00_u8 << page;
    frame.send(true);
}

void PagesManager::show_wait_page(const __FlashStringHelper* message, bool save_back)
{
    LCD::set_status_PGM(reinterpret_cast<const char*>(message));
    show_page(Page::Waiting, save_back);
}

void PagesManager::show_wait_back_page(const __FlashStringHelper* message, WaitCalllback back, bool save_back)
{
    LCD::set_status_PGM(reinterpret_cast<const char*>(message));
    back_ = back;
    show_page(Page::WaitBack, save_back);
}

void PagesManager::show_wait_back_continue_page(const __FlashStringHelper* message, WaitCalllback back, WaitCalllback cont, bool save_back)
{
    LCD::set_status_PGM(reinterpret_cast<const char*>(message));
    back_ = back;
    continue_ = cont;
    show_page(Page::WaitBackContinue, save_back);
}

void PagesManager::show_wait_continue_page(const __FlashStringHelper* message, WaitCalllback cont, bool save_back)
{
    LCD::set_status_PGM(reinterpret_cast<const char*>(message));
    back_ = nullptr;
    continue_ = cont;
    show_page(Page::WaitContinue, save_back);
}

void PagesManager::handle_wait(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Back:        handle_lcd_back(); break;
        case KeyValue::Save:        handle_lcd_continue(); break;
        default:                    Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void PagesManager::handle_lcd_back()
{
    if(!back_)
    {
        Log::error() << F("No Back action defined") << Log::endl();
        return;
    }

    back_();
    back_ = nullptr;
}

void PagesManager::handle_lcd_continue()
{
    if(!continue_)
    {
        Log::error() << F("No Continue action defined") << Log::endl();
        return;
    }

    continue_();
    continue_ = nullptr;
}

//! Retrieve the current page on the LCD screen
Page PagesManager::get_current_page()
{
    ReadRegister frame{Register::PictureID, 2};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Reading PictureID") << Log::endl();
        return Page::None;
    }

    Uint16 page; frame >> page;
    Log::log() << F("Current page index = ") << page.word << Log::endl();
    return static_cast<Page>(page.word);
}

//! Set page to display after the completion of an operation.
void PagesManager::save_forward_page()
{
    forward_page_ = get_current_page();
}

//! Show the "Back" page on the LCD display.
void PagesManager::show_back_page()
{
    if(back_pages_.is_empty())
    {
        Log::error() << F("No Back page defined" )<< Log::endl();
        return;
    }

    show_page(back_pages_.pop(), false);
}

//! Show the "Next" page on the LCD display.
void PagesManager::show_forward_page()
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

// --------------------------------------------------------------------
// Printer implementation
// --------------------------------------------------------------------

Printer_::Printer_()
: pages_{*this},
  lcd_{pages_},
  task_{*this, pages_},
  features_{DEFAULT_FEATURES},
  usb_baudrate_{DEFAULT_USB_BAUDRATE},
  usb_old_baudrate_{DEFAULT_USB_BAUDRATE},
  load_unload_{pages_},
  preheat_{pages_},
  move_{pages_},
  sd_card_{pages_},
  factory_reset_{pages_},
  manual_leveling_{pages_},
  extruder_tuning_{pages_},
  pid_tuning_{pages_},
  sensor_settings_{pages_},
  firmware_settings_{pages_},
  no_sensor_{pages_},
  lcd_settings_{pages_},
  statistics_{pages_},
  versions_{pages_},
  print_settings_{pages_},
  pid_settings_{pages_},
  steps_settings_{pages_},
  feedrates_settings_{pages_},
  accelerations_settings_{pages_},
  jerks_settings_{pages_},
  copyrights_{pages_},
  sensor_tuning_{pages_},
  sensor_grid_{pages_},
  sensor_z_height_{pages_},
  change_filament_{pages_},
  eeprom_mismatch_{pages_},
  sponsors_{pages_},
  linear_advance_tuning_{pages_},
  linear_advance_settings_{pages_},
  diagnosis_{pages_},
  sd_print_{pages_},
  usb_print_{pages_},
  pause_{pages_}
{
}

//! Initialize the printer and its LCD
void Printer_::setup()
{
    init_ = true;

    if(usb_baudrate_ != BAUDRATE)
        change_usb_baudrate();

    Serial2.begin(advi3_pp_baudrate);
}

void Printer_::show_boot_page()
{
    if(eeprom_mismatch_.does_mismatch())
    {
        pages_.show_page(Page::EEPROMMismatch);
        return;
    }

    if(!is_lcd_version_valid())
    {
        pages_.show_page(Page::VersionsMismatch, false);
        return;
    }

    Page page = pages_.get_current_page();
    if(page > Page::Main)
        pages_.show_page(Page::Main, false);
}

//! Note to forks author:
//! Under GPLv3 provision 7(b), you are not authorized to remove or alter this notice.
void Printer_::send_gplv3_7b_notice()
{
    Log::log() << F("Based on ADVi3++, Copyright (C) 2017 Sebastien Andrivet") << Log::endl();
}

void Printer_::send_sponsors()
{
    Log::log() << F("Sponsored by Johnathan Chamberlain, Timothy D Hoogland, Gavin Smith, Sawtoothsquid") << Log::endl();
}

//! Process command specific to this printer (I)
void Printer_::process_command(const GCodeParser& parser)
{
    switch(parser.codenum)
    {
        case 0: icode_0(parser); break;
        default: Log::error() << F("Invalid I-code number ") << static_cast<uint16_t>(parser.codenum) << Log::endl(); break;
    }
}

//! Store presets in permanent memory.
//! @param write Function to use for the actual writing
//! @param eeprom_index
//! @param working_crc
void Printer_::store_eeprom_data(eeprom_write write, int& eeprom_index, uint16_t& working_crc)
{
    EepromWrite eeprom{write, eeprom_index, working_crc};

    preheat_.store_eeprom_data(eeprom);
    pid_settings_.store_eeprom_data(eeprom);
    eeprom.write(features_);
    eeprom.write(usb_baudrate_);
}

//! Restore presets from permanent memory.
//! @param read Function to use for the actual reading
//! @param eeprom_index
//! @param working_crc
void Printer_::restore_eeprom_data(eeprom_read read, int& eeprom_index, uint16_t& working_crc)
{
    EepromRead eeprom{read, eeprom_index, working_crc};

    preheat_.restore_eeprom_data(eeprom);
    pid_settings_.restore_eeprom_data(eeprom);
    eeprom.read(features_);
    eeprom.read(usb_baudrate_);

    dimming_.enable(test_one_bit(features_, Feature::Dimming));
    LCD::enable_buzzer(test_one_bit(features_, Feature::Buzzer));
    LCD::enable_buzz_on_press(test_one_bit(features_, Feature::BuzzOnPress));

    check_and_fix();
}

//! Reset presets.
void Printer_::reset_eeprom_data()
{
    preheat_.reset_eeprom_data();
    pid_settings_.reset_eeprom_data();
    features_ = DEFAULT_FEATURES;
    usb_baudrate_ = DEFAULT_USB_BAUDRATE;
}

//! Return the size of data specific to ADVi3++
uint16_t Printer_::size_of_eeprom_data() const
{
    return
        preheat_.size_of_eeprom_data() +
        pid_settings_.size_of_eeprom_data() +
        sizeof(features_) +
        sizeof(usb_baudrate_);
}


//! Inform the user that the EEPROM data are not compatible and have been reset
void Printer_::eeprom_settings_mismatch()
{
    // It is not possible to show the Mismatch page now since nothing is yet initialized.
    // It will be done in the setup method.
    eeprom_mismatch_.set_mismatch();
}

void Printer_::save_settings()
{
    eeprom_mismatch_.reset_mismatch();
    enqueue_and_echo_commands_P(PSTR("M500"));
}

bool Printer_::is_thermal_protection_enabled() const
{
    return test_one_bit(features_, Feature::ThermalProtection);
}


// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Incoming LCD commands and status update
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Do not do soo many things in setup so do things here
void Printer_::init()
{
    init_ = false;

#ifdef DEBUG
    Log::log() << F("This is a DEBUG build") << Log::endl();
#endif

#ifdef ADVi3PP_BLTOUCH
    Log::log() << F("This is a BLTouch build") << Log::endl();
#endif

    send_gplv3_7b_notice(); // You are not authorized to remove or alter this notice
    send_sponsors();

    get_advi3pp_lcd_version();
    send_versions();
    graphs_.clear();
    dimming_.reset();

    show_boot_page();
}

//! Background tasks
void Printer_::task()
{
    if(init_)
        init();

    dimming_.check();
    read_lcd_serial();
    task_.execute_background_task();
    update_progress();
    send_status_data();
    graphs_.update();
}

bool Printer_::is_busy()
{
    return busy_state != NOT_BUSY || Planner::has_blocks_queued();
}

void Printer_::update_progress()
{
    // Progress bar % comes from SD when actively printing
    if(card.sdprinting)
        progress_bar_percent = card.percentDone();
}

//! Update the status of the printer on the LCD.
void Printer_::send_status_data(bool force_update)
{
    if(!force_update && !task_.is_update_time())
        return;

    int16_t progress_bar_low = progress_bar_percent >= 50 ? 5 : progress_bar_percent / 10;
    int16_t progress_var_high  = progress_bar_percent < 50 ? 0 : (progress_bar_percent / 10) - 5;

    WriteRamDataRequest frame{Variable::TargetBed};
    frame << Uint16(Temperature::target_temperature_bed)
          << Uint16(Temperature::degBed())
          << Uint16(Temperature::target_temperature[0])
          << Uint16(Temperature::degHotend(0))
          << Uint16(scale(fanSpeeds[0], 255, 100))
          << Uint16(lround(LOGICAL_Z_POSITION(current_position[Z_AXIS]) * 100.0))
          << FixedSizeString(LCD_::instance().get_message(), 40)
          << FixedSizeString(LCD_::instance().get_progress(), 40)
          << FixedSizeString(LCD_::instance().get_message(), 44, true)
          << Uint16(progress_bar_low)
          << Uint16(progress_var_high);
    frame.send(false);
}

//! Read a frame from the LCD and act accordingly.
void Printer_::read_lcd_serial()
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

    LCD::buzz_on_press();

    Command command; Action action; Uint8 nb_words; Uint16 value;
    frame >> command >> action >> nb_words >> value;
    auto key_value = static_cast<KeyValue>(value.word);

    Log::log() << F(">>> ") << nb_words.byte << F(" words, Action = ") << static_cast<uint16_t>(action) << F(", KeyValue = ") << value.word << Log::endl();

    switch(action)
    {
        case Action::Screen:                screen(key_value); break;
        case Action::PrintCommand:          print_command(key_value); break;
        case Action::Wait:                  pages_.handle_wait(key_value); break;
        case Action::LoadUnload:            load_unload_.handle(key_value); break;
        case Action::Preheat:               preheat_.handle(key_value); break;
        case Action::Move:                  move_.handle(key_value); break;
        case Action::SdCard:                sd_card_.handle(key_value); break;
        case Action::FactoryReset:          factory_reset_.handle(key_value); break;
        case Action::ManualLeveling:        manual_leveling_.handle(key_value); break;
        case Action::ExtruderTuning:        extruder_tuning_.handle(key_value); break;
        case Action::PidTuning:             pid_tuning_.handle(key_value); break;
        case Action::SensorSettings:        sensor_settings_.handle(key_value); break;
        case Action::Firmware:              firmware_settings_.handle(key_value); break;
        case Action::NoSensor:              no_sensor_.handle(key_value); break;
        case Action::LCD:                   lcd_settings_.handle(key_value); break;
        case Action::Statistics:            statistics_.handle(key_value); break;
        case Action::Versions:              versions_.handle(key_value); break;
        case Action::PrintSettings:         print_settings_.handle(key_value); break;
        case Action::PIDSettings:           pid_settings_.handle(key_value); break;
        case Action::StepsSettings:         steps_settings_.handle(key_value); break;
        case Action::FeedrateSettings:      feedrates_settings_.handle(key_value); break;
        case Action::AccelerationSettings:  accelerations_settings_.handle(key_value); break;
        case Action::JerkSettings:          jerks_settings_.handle(key_value); break;
        case Action::Copyrights:            copyrights_.handle(key_value); break;
        case Action::SensorTuning:          sensor_tuning_.handle(key_value); break;
        case Action::SensorGrid:            sensor_grid_.handle(key_value); break;
        case Action::SensorZHeight:         sensor_z_height_.handle(key_value); break;
        case Action::ChangeFilament:        change_filament_.handle(key_value); break;
        case Action::EEPROMMismatch:        eeprom_mismatch_.handle(key_value); break;
        case Action::Sponsors:              sponsors_.handle(key_value); break;
        case Action::LinearAdvanceTuning:   linear_advance_tuning_.handle(key_value); break;
        case Action::LinearAdvanceSettings: linear_advance_settings_.handle(key_value); break;
        case Action::Diagnosis:             diagnosis_.handle(key_value); break;

        case Action::MoveXPlus:             move_.x_plus(); break;
        case Action::MoveXMinus:            move_.x_minus(); break;
        case Action::MoveYPlus:             move_.y_plus(); break;
        case Action::MoveYMinus:            move_.y_minus(); break;
        case Action::MoveZPlus:             move_.z_plus(); break;
        case Action::MoveZMinus:            move_.z_minus(); break;
        case Action::MoveEPlus:             move_.e_plus(); break;
        case Action::MoveEMinus:            move_.e_minus(); break;
        case Action::LCDBrightness:         dimming_.change_brightness(static_cast<int16_t>(key_value)); break;
        case Action::BabyMinus:             print_settings_.baby_minus(); break;
        case Action::BabyPlus:              print_settings_.baby_plus(); break;

        default:                            Log::error() << F("Invalid action ") << static_cast<uint16_t>(action) << Log::endl(); break;
    }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Main
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::screen(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Temps:           show_temps(); break;
        case KeyValue::Print:           show_print(); break;
        case KeyValue::Controls:        pages_.show_page(Page::Controls); break;
        case KeyValue::Tuning:          pages_.show_page(Page::Tuning); break;
        case KeyValue::Settings:        pages_.show_page(Page::Settings); break;
        case KeyValue::Infos:           pages_.show_page(Page::Infos); break;
        case KeyValue::Motors:          pages_.show_page(Page::MotorsSettings); break;
        case KeyValue::Leveling:        pages_.show_page(Page::Leveling); break;
        case KeyValue::Back:            back(); break;
        default:                        Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Show one of the temperature graph screens depending of the context: either the SD printing screen,
//! the printing screen or the temperature screen.
void Printer_::show_temps()
{
    if(!PrintCounter::isRunning() && !PrintCounter::isPaused())
    {
        pages_.show_page(Page::Temperature);
        return;
    }

    // If there is a print running (or paused), display the print screen.
    pages_.show_page(Page::Print);
}

//! Show one of the Printing screens depending of the context:
//! - If a print is running, display the Print screen
//! - Otherwise, try to access the SD card. Depending of the result, display the SD card Page or the Temperatures page
void Printer_::show_print()
{
    // If there is a print running (or paused), display the SD or USB print screen
    if(PrintCounter::isRunning() || PrintCounter::isPaused())
    {
        pages_.show_page(Page::Print);
        return;
    }

    pages_.show_wait_page(F("Try to access the SD card..."));
    task_.set_background_task(BackgroundTask(this, &Printer_::show_sd_or_temp_page));
}

void Printer_::show_sd_or_temp_page()
{
    task_.clear_background_task();

    card.initsd(); // Can take some time
    LCD::reset_message();
    if(!card.cardOK)
    {
        // SD card not accessible so fall back to Temperatures
        pages_.show_page(Page::Temperature, false);
        return;
    }

    pages_.show_page(Page::SdCard, false);
    sd_card_.show_first_page();
}

void Printer_::back()
{
    pages_.show_back_page();
}

//! Handle print commands.
//! @param key_value    The sub-action to handle
void Printer_::print_command(KeyValue key_value)
{
    if(card.isFileOpen())
        sd_print_.handle(key_value);
    else
        usb_print_.handle(key_value);
}

// --------------------------------------------------------------------
// SD Card
// --------------------------------------------------------------------

SdCard::SdCard(PagesManager& pages, Task& task)
: Handler{pages, task}
{
}

bool SdCard::dispatch(KeyValue key_value)
{
    if(Handler::dispatch(key_value))
        return true;

	switch(key_value)
	{
		case KeyValue::SDUp:	up(); break;
		case KeyValue::SDDown:	down(); break;
		case KeyValue::SDLine1:
		case KeyValue::SDLine2:
		case KeyValue::SDLine3:
		case KeyValue::SDLine4:
		case KeyValue::SDLine5:	select_file(static_cast<uint16_t>(key_value) - 1); break;
		default:                return false;
	}

	return true;
}

void SdCard::show_first_page()
{
	if(!card.cardOK)
		return;

	nb_files_ = card.getnrfilenames();
	last_file_index_ = nb_files_ - 1;

    show_current_page();
}

void SdCard::down()
{
	if(!card.cardOK)
		return;

	if(last_file_index_ >= nb_visible_sd_files)
		last_file_index_ -= nb_visible_sd_files;

    show_current_page();
}

void SdCard::up()
{
	if(!card.cardOK)
		return;

	if(last_file_index_ + nb_visible_sd_files < nb_files_)
		last_file_index_ += nb_visible_sd_files;

    show_current_page();
}

//! Show the list of files on SD.
void SdCard::show_current_page()
{
    WriteRamDataRequest frame{Variable::FileName1};

    String name;
    for(uint8_t index = 0; index < nb_visible_sd_files; ++index)
    {
        get_file_name(index, name);
        frame << FixedSizeString(name, nb_visible_sd_file_chars); // Important to truncate, there is only limited space
    }
    frame.send(true);
}

//! Get a filename with a given index.
//! @param index    Index of the filename
//! @param name     Copy the filename into this Chars
void SdCard::get_file_name(uint8_t index_in_page, String& name)
{
    name = "";
	if(last_file_index_ >= index_in_page)
	{
		card.getfilename(last_file_index_ - index_in_page);
        if(card.filenameIsDir) name = "[";
		name += (card.longFilename[0] == 0) ? card.filename : card.longFilename;
		if(card.filenameIsDir) name += "]";
	}
};

//! Select a filename as sent by the LCD screen.
//! @param file_index    The index of the filename to select
void SdCard::select_file(uint16_t file_index)
{
    if(!card.cardOK)
        return;

    if(file_index > last_file_index_)
        return;

    card.getfilename(last_file_index_ - file_index);
    if(card.filenameIsDir)
        return;

    String longName{(card.longFilename[0] == 0) ? card.filename : card.longFilename};
    if(longName.length() <= 0) // If the SD card is not readable
        return;

    LCD_::instance().set_progress_name(longName);

    WriteRamDataRequest frame{Variable::CurrentFileName};
    frame << FixedSizeString{longName, 26};
    frame.send(true);

    card.openFile(card.filename, true); // use always short filename so it will work even if the filename is long
    card.startFileprint();
    PrintCounter::start();

    pages().show_page(Page::Print);
}

// --------------------------------------------------------------------
// Printing
// --------------------------------------------------------------------

//! Handle print commands.
//! @param key_value    The sub-action to handle
bool Print::dispatch(KeyValue key_value)
{
    if(Handler::dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::PrintStop:           stop(); break;
        case KeyValue::PrintPauseResume:    pause_resume(); break;
        case KeyValue::PrintAdvancedPause:  advanced_pause(); break;
        default:                            return false;
    }

    return true;
}

//! Stop printing
void Print::stop()
{
    Log::log() << F("Stop Print") << Log::endl();

    do_stop();
    clear_command_queue();
    quickstop_stepper();
    PrintCounter::stop();
    Temperature::disable_all_heaters();
    fanSpeeds[0] = 0;

    pages().show_back_page();
    task().set_background_task(BackgroundTask(this, &Print::reset_messages_task), 500);
}

void Print::reset_messages_task()
{
    task().clear_background_task();
    lcd().reset_progress();
    lcd().reset_message();
}

//! Pause printing
void Print::pause_resume()
{
    // FIX
    Log::log() << F("Pause or Resume Print") << Log::endl();

    if(is_printing())
    {
        LCD::queue_message(F("Pause printing..."));
        do_pause();
        PrintCounter::pause();
#if ENABLED(PARK_HEAD_ON_PAUSE)
        enqueue_and_echo_commands_P(PSTR("M125"));
#endif
    }
    else
    {
        Log::log() << F("Resume Print") << Log::endl();

        LCD::queue_message(F("Resume printing"));
#if ENABLED(PARK_HEAD_ON_PAUSE)
        enqueue_and_echo_commands_P(PSTR("M24"));
#endif
        do_resume();
        PrintCounter::start(); // TODO: Check this is right
    }
}

//! Resume the current SD printing
void Print::advanced_pause()
{
    enqueue_and_echo_commands_P(PSTR("M600"));
}

// --------------------------------------------------------------------
// SD Printing
// --------------------------------------------------------------------

void SdPrint::do_stop()
{
    card.stopSDPrint();
}

void SdPrint::do_pause()
{
    card.pauseSDPrint();
}

void SdPrint::do_resume()
{
    card.startFileprint();
}

bool SdPrint::is_printing() const
{
    return card.sdprinting;
}

// --------------------------------------------------------------------
// USB Printing
// --------------------------------------------------------------------

void UsbPrint::do_stop()
{
    SERIAL_ECHOLNPGM("//action:cancel");
}

void UsbPrint::do_pause()
{
    SERIAL_ECHOLNPGM("//action:pause");
}

void UsbPrint::do_resume()
{
    SERIAL_ECHOLNPGM("//action:resume");
}

bool UsbPrint::is_printing() const
{
    return PrintCounter::isRunning();
}

// --------------------------------------------------------------------
// Load and Unload Filament
// --------------------------------------------------------------------

//! Handle Load & Unload actions.
//! @param key_value    The sub-action to handle
bool LoadUnload::dispatch(KeyValue key_value)
{
    if(Handler::dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::Load:    load(); break;
        case KeyValue::Unload:  unload(); break;
        default:                return false;
    }

    return true;
}

//! Show the Load & Unload screen on the LCD display.
Page LoadUnload::do_show()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(printer().last_used_hotend_temperature());
    frame.send();

    return Page::LoadUnload;
}

void LoadUnload::prepare()
{
    ReadRamData frame{Variable::Value0, 1};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Target Temperature)") << Log::endl();
        return;
    }

    Uint16 hotend; frame >> hotend;

    Temperature::setTargetHotend(hotend.word, 0);
    enqueue_and_echo_commands_P(PSTR("M83"));       // relative E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));    // reset E axis
}

//! Start Load action.
void LoadUnload::load()
{
    prepare();

    task().set_background_task(BackgroundTask(this, &LoadUnload::load_start_task));
    pages().show_wait_back_page(F("Wait until the target temp is reached..."),
        WaitCalllback(this, &LoadUnload::stop));
}

//! Start Load action.
void LoadUnload::unload()
{
    prepare();

    task().set_background_task(BackgroundTask(this, &LoadUnload::unload_start_task));
    pages().show_wait_back_page(F("Wait until the target temp is reached..."),
        WaitCalllback(this, &LoadUnload::stop));
}


//! Handle back from the Load on Unload LCD screen.
void LoadUnload::stop()
{
    Log::log() << F("Load/Unload Stop");

    LCD::reset_message();
    task().set_background_task(BackgroundTask(this, &LoadUnload::stop_task));
    clear_command_queue();
    Temperature::setTargetHotend(0, 0);

    pages().show_back_page();
}

void LoadUnload::stop_task()
{
    if(printer().is_busy() || !task().has_background_task())
        return;

    task().clear_background_task();
    LCD::reset_message();
    // Do this asynchronously to avoid race conditions
    enqueue_and_echo_commands_P(PSTR("M82"));       // absolute E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));    // reset E axis

}

//! Load the filament if the temperature is high enough.
void LoadUnload::load_start_task()
{
    if(Temperature::current_temperature[0] >= Temperature::target_temperature[0] - 10)
    {
        Log::log() << F("Load Filament") << Log::endl();
        LCD::buzz(100); // Inform the user that the extrusion starts
        enqueue_and_echo_commands_P(PSTR("G1 E1 F120"));
        task().set_background_task(BackgroundTask(this, &LoadUnload::load_task));
        LCD::set_status(F("Wait until the filament comes out..."));
    }
}

//! Load the filament if the temperature is high enough.
void LoadUnload::load_task()
{
    if(Temperature::current_temperature[0] >= Temperature::target_temperature[0] - 10)
        enqueue_and_echo_commands_P(PSTR("G1 E1 F120"));
}


//! Unload the filament if the temperature is high enough.
void LoadUnload::unload_start_task()
{
    if(Temperature::current_temperature[0] >= Temperature::target_temperature[0] - 10)
    {
        Log::log() << F("Unload Filament") << Log::endl();
        LCD::buzz(100); // Inform the user that the un-extrusion starts
        enqueue_and_echo_commands_P(PSTR("G1 E-1 F120"));
        task().set_background_task(BackgroundTask(this, &LoadUnload::unload_task));
        LCD::set_status(F("Wait until the filament comes out..."));
    }
}

//! Unload the filament if the temperature is high enough.
void LoadUnload::unload_task()
{
    if(Temperature::current_temperature[0] >= Temperature::target_temperature[0] - 10)
        enqueue_and_echo_commands_P(PSTR("G1 E-1 F120"));
}

// --------------------------------------------------------------------
// Preheat & Cooldown
// --------------------------------------------------------------------

//! Handle Preheat actions.
//! @param key_value    Sub-action to handle
bool Preheat::dispatch(KeyValue key_value)
{
    if(Handler::dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::PresetPrevious:  previous(); break;
        case KeyValue::PresetNext:      next(); break;
        case KeyValue::Cooldown:        cooldown(); break;
        default:                        return false;
    }

    return true;
}

//! Store presets in permanent memory.
void Preheat::store_eeprom_data(EepromWrite& eeprom)
{
    for(auto& preset: presets_)
    {
        eeprom.write(preset.hotend);
        eeprom.write(preset.bed);
    }
}

//! Restore presets from permanent memory.
//! @param read Function to use for the actual reading
//! @param eeprom_index
//! @param working_crc
void Preheat::restore_eeprom_data(EepromRead& eeprom)
{
    for(auto& preset: presets_)
    {
        eeprom.read(preset.hotend);
        eeprom.read(preset.bed);
    }
}

//! Reset presets.
void Preheat::reset_eeprom_data()
{
    for(size_t i = 0; i < NB_PRESETS; ++i)
    {
        presets_[i].hotend  = DEFAULT_PREHEAT_PRESET[i][0];
        presets_[i].bed     = DEFAULT_PREHEAT_PRESET[i][1];
        presets_[i].fan     = DEFAULT_PREHEAT_PRESET[i][2];
    }
}

uint16_t Preheat::size_of_eeprom_data() const
{
    return NB_PRESETS * (sizeof(Preset::hotend) + sizeof(Preset::bed));
}

void Preheat::send_preset()
{
    Log::log() << F("Preheat page") << Log::endl();
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(presets_[index_].bed)
          << Uint16(presets_[index_].hotend)
          << Uint16(presets_[index_].fan);
    frame.send();
}

//! Show the preheat screen
Page Preheat::do_show()
{
    send_presets();
    return Page::Preheat;
}

void Preheat::previous()
{
    if(index_ <= 0)
        return;
    --index_;
    send_preset();
}

void Preheat::next()
{
    if(index_ >= NB_PRESETS - 1)
        return;
    ++index_;
    send_preset();
}

//! Cooldown the bed and the nozzle
void Preheat::cooldown()
{
    Log::log() << F("Cooldown") << Log::endl();
    LCD::reset_message();
    Temperature::disable_all_heaters();
}

// --------------------------------------------------------------------
// Move & Home
// --------------------------------------------------------------------

//! Execute a move command
bool Move::dispatch(KeyValue key_value)
{
    if(Handler::dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::MoveXHome:           x_home(); break;
        case KeyValue::MoveYHome:           y_home(); break;
        case KeyValue::MoveZHome:           z_home(); break;
        case KeyValue::MoveAllHome:         all_home(); break;
        case KeyValue::DisableMotors:       disable_motors(); break;
        default:                            return false;
    }

    return true;
}

Page Move::do_show()
{
    Planner::finish_and_disable(); // To circumvent homing problems
    return Page::Move;
}

//! Move the nozzle.
void Move::move(const char* command, millis_t delay)
{
    if(!ELAPSED(millis(), last_move_time_ + delay))
        return;
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(command);
    enqueue_and_echo_commands_P(PSTR("G90"));
    last_move_time_ = millis();
}


//! Move the nozzle.
void Move::x_plus()
{
    move(PSTR("G1 X4 F1000"), 150);
}

//! Move the nozzle.
void Move::x_minus()
{
    move(PSTR("G1 X-4 F1000"), 150);
}

//! Move the nozzle.
void Move::y_plus()
{
    move(PSTR("G1 Y4 F1000"), 150);
}

//! Move the nozzle.
void Move::y_minus()
{
    move(PSTR("G1 Y-4 F1000"), 150);
}

//! Move the nozzle.
void Move::z_plus()
{
    move(PSTR("G1 Z0.5 F1000"), 10);
}

//! Move the nozzle.
void Move::z_minus()
{
    move(PSTR("G1 Z-0.5 F1000"), 10);
}

//! Extrude some filament.
void Move::e_plus()
{
    if(Temperature::degHotend(0) < 180)
        return;

    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 E1 F120"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Unextrude.
void Move::e_minus()
{
    if(Temperature::degHotend(0) < 180)
        return;

    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 E-1 F120"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Disable the motors.
void Move::disable_motors()
{
    enqueue_and_echo_commands_P(PSTR("M84"));
    axis_homed = 0;
    axis_known_position = 0;
}

//! Go to home on the X axis.
void Move::x_home()
{
    enqueue_and_echo_commands_P(PSTR("G28 X0"));
}

//! Go to home on the Y axis.
void Move::y_home()
{
    enqueue_and_echo_commands_P(PSTR("G28 Y0"));
}

//! Go to home on the Z axis.
void Move::z_home()
{
    enqueue_and_echo_commands_P(PSTR("G28 Z0"));
}

//! Go to home on all axis.
void Move::all_home()
{
    enqueue_and_echo_commands_P(PSTR("G28"));
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Factory Reset
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Reset all settings of the printer to factory ones.
void Printer_::factory_reset(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:               show_factory_reset_warning(); break;
        case KeyValue::ResetConfirm:       do_factory_reset(); break;
        case KeyValue::Back:               cancel_factory_reset(); break;
        default:                           Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::show_factory_reset_warning()
{
    pages_.show_page(Page::FactoryReset);
}

void Printer_::do_factory_reset()
{
    enqueue_and_echo_commands_P(PSTR("M502"));
    save_settings();
    pages_.show_back_page();
}

void Printer_::cancel_factory_reset()
{
    pages_.show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Statistics
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Display statistics on the LCD screen.
void Printer_::statistics(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:    show_stats(); break;
        case KeyValue::Back:    stats_back(); break;
        default:                Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::show_stats()
{
    send_stats();
    pages_.show_page(Page::Statistics);
}

void Printer_::stats_back()
{
    pages_.show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Versions
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::get_advi3pp_lcd_version()
{
    ReadRamData response{Variable::ADVi3ppLCDversion, 1};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Measures)") << Log::endl();
        return;
    }

    Uint16 version; response >> version;
    Log::log() << F("ADVi3++ LCD version = ") <<  version.word << Log::endl();
    lcd_version_ = version.word;
}

//! Get the current LCD firmware version.
//! @return     The version as a string.
String Printer_::get_lcd_firmware_version()
{
    ReadRegister response{Register::Version, 1};
    if(!response.send_and_receive())
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
void Printer_::send_versions()
{
    String marlin_version{SHORT_BUILD_VERSION};
    String motherboard_version = convert_version(advi3_pp_version);
    String advi3pp_lcd_version = convert_version(lcd_version_);
    String lcd_firmware_version = get_lcd_firmware_version();

    WriteRamDataRequest frame{Variable::MotherboardVersion};
    frame << FixedSizeString(motherboard_version, 16)
          << FixedSizeString(advi3pp_lcd_version, 16)
          << FixedSizeString(lcd_firmware_version, 16)
		  << FixedSizeString(marlin_version, 16);
    frame.send();
}

bool Printer_::is_lcd_version_valid() const
{
    return lcd_version_ >= advi3_pp_oldest_lcd_compatible_version && lcd_version_ <= advi3_pp_newest_lcd_compatible_version;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Versions
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Display the Versions screen,
void Printer_::versions(KeyValue key_value)
{
    switch(key_value)
    {
		case KeyValue::Show:					versions_show(); break;
        case KeyValue::MismatchForward:         versions_mismatch_forward(); break;
        case KeyValue::Back:                    versions_back(); break;
        default:								Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::versions_mismatch_forward()
{
    pages_.show_page(Page::Main, false);
}

void Printer_::versions_back()
{
    pages_.show_back_page();
}

void Printer_::versions_show()
{
    pages_.show_page(Page::Versions);
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Sponsors
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Display the Sponsors screen,
void Printer_::sponsors(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:					sponsors_show(); break;
        case KeyValue::Back:                    sponsors_back(); break;
        default:								Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::sponsors_back()
{
    pages_.show_back_page();
}

void Printer_::sponsors_show()
{
    pages_.show_page(Page::Sponsors);
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Copyrights
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Display the Copyrights screen,
void Printer_::copyrights(advi3pp::KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:					copyrights_show(); break;
        case KeyValue::Back:                    copyrights_back(); break;
        default:								Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::copyrights_show()
{
    pages_.show_page(Page::Copyrights);
}

void Printer_::copyrights_back()
{
    pages_.show_back_page();
}


// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// PID Tuning
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Handle PID tuning.
//! @param key_value    The step of the PID tuning
void Printer_::pid_tuning(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::PidTuningStep1:   pid_tuning_step1(); break;
        case KeyValue::PidTuningStep2:   pid_tuning_step2(); break;
        case KeyValue::Back:             pid_tuning_cancel(); break;
        default:                         Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Show step #1 of PID tuning
void Printer_::pid_tuning_step1()
{
    pid_.init();
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(last_used_temperature);
    frame.send();

    pages_.save_forward_page();
    pages_.show_page(Page::PidTuning1);
}

//! Show step #2 of PID tuning
void Printer_::pid_tuning_step2()
{
    ReadRamData frame{Variable::Value0, 1};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Target Temperature)") << Log::endl();
        return;
    }

    Uint16 hotend; frame >> hotend;

    enqueue_and_echo_commands_P(PSTR("M106 S255")); // Turn on fan
    String auto_pid_command; auto_pid_command << F("M303 S") << hotend.word << F("E0 C8 U1");
    enqueue_and_echo_command(auto_pid_command.c_str());

    pages_.show_page(Page::PidTuning2);
};

//! PID automatic tuning is finished.
void Printer_::auto_pid_finished()
{
    Log::log() << F("Auto PID finished") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("M106 S0"));
    pid_settings_show(false, false);
}

void Printer_::pid_tuning_cancel()
{
    pages_.show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Leveling
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::manual_leveling(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:            leveling_home(); break;
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
void Printer_::leveling_home()
{
    pages_.show_wait_page(F("Homing..."));
    axis_homed = 0;
    axis_known_position = 0;
    enqueue_and_echo_commands_P(PSTR("G90")); // absolute mode
    enqueue_and_echo_commands_P((PSTR("G28"))); // homing
    task_.set_background_task(BackgroundTask(this, &Printer_::manual_leveling_task), 200);
}

//! Leveling Background task.
void Printer_::manual_leveling_task()
{
    if(!TEST(axis_homed, X_AXIS) || !TEST(axis_homed, Y_AXIS) || !TEST(axis_homed, Z_AXIS))
        return;

    Log::log() << F("Leveling Homed, start process") << Log::endl();
    LCD::reset_message();
    task_.clear_background_task();
    pages_.show_page(Page::ManualLeveling, false);
}


//! Handle leveling point #1.
void Printer_::leveling_point1()
{
    Log::log() << F("Level step 1") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X30 Y30 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #2.
void Printer_::leveling_point2()
{
    Log::log() << F("Level step 2") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X170 Y170 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #3.
void Printer_::leveling_point3()
{
    Log::log() << F("Level step 3") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X170 Y30 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #4.
void Printer_::leveling_point4()
{
    Log::log() << F("Level step 4") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X30 Y170 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #5.
void Printer_::leveling_point5()
{
    Log::log() << F("Level step 5") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F2000"));
    enqueue_and_echo_commands_P(PSTR("G1 X100 Y100 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1000"));
}

//! Handle leveling point #4.
void Printer_::leveling_finish()
{
    enqueue_and_echo_commands_P(PSTR("G1 Z30 F2000"));
    pages_.show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Extruder tuning
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::extruder_tuning(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:            show_extruder_tuning(); break;
        case KeyValue::TuningStart:     start_extruder_tuning(); break;
        case KeyValue::TuningSettings: extruder_calibration_settings(); break;
        case KeyValue::Back:            cancel_extruder_tuning(); break;
        default:                        Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

//! Show the extruder tuning screen.
void Printer_::show_extruder_tuning()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << 200_u16;
    frame.send();

    pages_.save_forward_page();
    pages_.show_page(Page::ExtruderTuningTemp);
}

//! Start extruder tuning.
void Printer_::start_extruder_tuning()
{
    ReadRamData frame{Variable::Value0, 1};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Target Temperature)") << Log::endl();
        return;
    }

    Uint16 hotend; frame >> hotend;

    pages_.show_wait_page(F("Heating the extruder..."));
    Temperature::setTargetHotend(hotend.word, 0);

    task_.set_background_task(BackgroundTask(this, &Printer_::extruder_tuning_heating_task));
}

//! Extruder tuning background task.
void Printer_::extruder_tuning_heating_task()
{
    if(Temperature::current_temperature[0] < Temperature::target_temperature[0] - 10)
        return;
    task_.clear_background_task();

    LCD::set_status(F("Wait until the extrusion is finished..."));
    enqueue_and_echo_commands_P(PSTR("G1 Z20 F240"));   // raise head
    enqueue_and_echo_commands_P(PSTR("M83"));           // relative E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));        // reset E axis

    String command; command << F("G1 E") << tuning_extruder_filament << " F50"; // Extrude slowly
    enqueue_and_echo_command(command.c_str());

    task_.set_background_task(BackgroundTask(this, &Printer_::extruder_tuning_extruding_task));
}

//! Extruder tuning background task.
void Printer_::extruder_tuning_extruding_task()
{
    if(current_position[E_AXIS] < tuning_extruder_filament || is_busy())
        return;
    task_.clear_background_task();

    extruded_ = current_position[E_AXIS];

    Temperature::setTargetHotend(0, 0);
    task_.clear_background_task();
    LCD::reset_message();
    extruder_tuning_finished();
}

//! Record the amount of filament extruded.
void Printer_::extruder_tuning_finished()
{
    Log::log() << F("Filament extruded ") << extruded_ << Log::endl();
    enqueue_and_echo_commands_P(PSTR("M82"));       // absolute E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));    // reset E axis

    task_.clear_background_task();
    pages_.show_page(Page::ExtruderTuningMeasure, false);
}

//! Cancel the extruder tuning.
void Printer_::cancel_extruder_tuning()
{
    task_.clear_background_task();

    Temperature::setTargetHotend(0, 0);

    enqueue_and_echo_commands_P(PSTR("M82"));       // absolute E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));    // reset E axis

    pages_.show_back_page();
}


//! Compute the extruder (E axis) new value and show the steps settings.
void Printer_::extruder_calibration_settings()
{
    ReadRamData response{Variable::Value0, 1};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Measures)") << Log::endl();
        return;
    }

    Uint16 e; response >> e;
    e.word /= 10;

    // Fill all values because all 4 axis are displayed by  show_steps_settings
    steps_.axis_steps_per_mm[X_AXIS] = Planner::axis_steps_per_mm[X_AXIS];
    steps_.axis_steps_per_mm[Y_AXIS] = Planner::axis_steps_per_mm[Y_AXIS];
    steps_.axis_steps_per_mm[Z_AXIS] = Planner::axis_steps_per_mm[Z_AXIS];
	steps_.axis_steps_per_mm[E_AXIS] = Planner::axis_steps_per_mm[E_AXIS]
                                       * extruded_ / (extruded_ + tuning_extruder_delta - e.word);

	Log::log() << F("Adjust: old = ")
               << Planner::axis_steps_per_mm[E_AXIS]
               << F(", expected = ") << extruded_
               << F(", measured = ") << (extruded_ + tuning_extruder_delta - e.word)
               << F(", new = ") << steps_.axis_steps_per_mm[E_AXIS] << Log::endl();

    steps_settings_show(false);
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Sensor Settings
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::sensor_settings(advi3pp::KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:            sensor_settings_show(); break;
        case KeyValue::Save:            sensor_settings_save(); break;
        case KeyValue::Back:            sensor_settings_cancel(); break;
        default:                        Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::sensor_settings_show()
{
#ifdef ADVi3PP_BLTOUCH
    sensor_.send_z_height_to_lcd(zprobe_zoffset);
    pages_.save_forward_page();
    pages_.show_page(Page::SensorSettings);
#else
    pages_.show_page(Page::NoSensor);
#endif
}

void Printer_::sensor_settings_save()
{
#ifdef ADVi3PP_BLTOUCH
    sensor_.save_lcd_z_height();
    pages_.show_forward_page();
#endif
}

void Printer_::sensor_settings_cancel()
{
    pages_.show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Sensor Tuning
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::sensor_tuning(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:            sensor_tuning_show(); break;
#ifdef ADVi3PP_BLTOUCH
        case KeyValue::SensorSelfTest:  sensor_.self_test(); break;
        case KeyValue::SensorReset:     sensor_.reset(); break;
        case KeyValue::SensorDeploy:    sensor_.deploy(); break;
        case KeyValue::SensorStow:      sensor_.stow(); break;
        case KeyValue::SensorZHeight:   sensor_z_height(); break;
#endif
        case KeyValue::Back:            sensor_tuning_back(); break;
        default:                        Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::sensor_tuning_show()
{
#ifdef ADVi3PP_BLTOUCH
    pages_.show_page(Page::SensorTuning);
#else
    pages_.show_page(Page::NoSensor);
#endif
}

void Printer_::sensor_tuning_back()
{
    pages_.show_back_page();
}

void Printer_::sensor_leveling()
{
#ifdef ADVi3PP_BLTOUCH
    sensor_interactive_leveling_ = true;
    pages_.save_forward_page();
    pages_.show_wait_page(F("Homing..."));
    enqueue_and_echo_commands_P(PSTR("G28"));                   // homing
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F240"));           // raise head
    enqueue_and_echo_commands_P(PSTR("G29 E"));                 // leveling
    enqueue_and_echo_commands_P(PSTR("G1 X0 Y0 F3000"));        // go back to corner
#else
    pages_.show_page(Page::NoSensor);
#endif
}

void Printer_::g29_leveling_finished(bool success)
{
    if(!success)
    {
        if(!sensor_interactive_leveling_ && !IS_SD_FILE_OPEN) // i.e. USB print
            SERIAL_ECHOLNPGM("//action:disconnect"); // "disconnect" is the only standard command to stop an USB print

        if(sensor_interactive_leveling_)
		    pages_.show_wait_back_page(F("Leveling failed"), WaitCalllback(this, &Printer_::g29_leveling_failed), false);
        else
            LCD::set_status(F("Leveling failed"));

        sensor_interactive_leveling_ = false;
        return;
    }

    LCD::reset_message();

    if(sensor_interactive_leveling_)
	{
		sensor_interactive_leveling_ = false;	
        sensor_grid_show();
	}
    else
    {
        enqueue_and_echo_commands_P(PSTR("M500"));      // Save settings (including mash)
        enqueue_and_echo_commands_P(PSTR("M420 S1"));   // Set bed leveling state (enable)
    }
}

void Printer_::g29_leveling_failed()
{
    pages_.show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Sensor grid
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::sensor_grid(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:            sensor_grid_show(); break;
        case KeyValue::Cancel:          sensor_grid_cancel(); break;
        case KeyValue::Save:            sensor_grid_save(); break;
        default:                        Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::sensor_grid_show()
{
#ifdef ADVi3PP_BLTOUCH
    WriteRamDataRequest frame{Variable::Value0};
    for(auto y = 0; y < GRID_MAX_POINTS_Y; y++)
        for(auto x = 0; x < GRID_MAX_POINTS_X; x++)
            frame << Uint16(static_cast<int16_t>(z_values[x][y] * 1000));
    frame.send();

    pages_.show_page(Page::SensorGrid, false);
#endif
}

void Printer_::sensor_grid_cancel()
{
    pages_.show_back_page();
}

void Printer_::sensor_grid_save()
{
    enqueue_and_echo_commands_P(PSTR("M500"));      // Save settings (including mash)
    enqueue_and_echo_commands_P(PSTR("M420 S1"));   // Set bed leveling state (enable)
    pages_.show_forward_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Sensor Z-Height
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::sensor_z_height()
{
#ifdef ADVi3PP_BLTOUCH
    pages_.save_forward_page();
    pages_.show_wait_page(F("Homing..."));
    enqueue_and_echo_commands_P((PSTR("G28")));  // homing
    task_.set_background_task(BackgroundTask(this, &Printer_::z_height_tuning_home_task), 200);
#else
    pages_.show_page(Page::NoSensor);
#endif
}

void Printer_::z_height_tuning_home_task()
{
    if(!TEST(axis_homed, X_AXIS) || !TEST(axis_homed, Y_AXIS) || !TEST(axis_homed, Z_AXIS))
        return;
    if(is_busy())
        return;

    LCD::set_status(F("Going to the middle of the bed..."));
    enqueue_and_echo_commands_P(PSTR("G1 Z10 F240"));           // raise head
    enqueue_and_echo_commands_P(PSTR("G1 X100 Y100 F3000"));    // center of the bed
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F240"));            // lower head

    task_.set_background_task(BackgroundTask(this, &Printer_::z_height_tuning_center_task), 200);
}

void Printer_::z_height_tuning_center_task()
{
    if(current_position[X_AXIS] != 100 || current_position[Y_AXIS] != 100 || current_position[Z_AXIS] != 0)
        return;
    if(is_busy())
        return;
    task_.clear_background_task();

    LCD::reset_message();
    pages_.show_page(Page::ZHeightTuning, false);
}

void Printer_::sensor_z_height(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Cancel:          sensor_z_height_cancel(); break;
        case KeyValue::Continue:        sensor_z_height_continue(); break;
        default:                        Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::sensor_z_height_cancel()
{
    enqueue_and_echo_commands_P((PSTR("G28 X0 Y0"))); // homing
    pages_.show_back_page();
}

void Printer_::sensor_z_height_continue()
{
    pages_.show_wait_page(F("Measure Z-height"));
    enqueue_and_echo_commands_P(PSTR("I0")); // measure z-height
}

//! I-code 0: measure z-height
void Printer_::icode_0(const GCodeParser& parser)
{
#ifdef ADVi3PP_BLTOUCH

    if(axis_unhomed_error())
    {
        pages_.show_back_page();
        return;
    }

    const float old_feedrate_mm_s = feedrate_mm_s;
    feedrate_mm_s = MMM_TO_MMS(XY_PROBE_SPEED);

    do_blocking_move_to(X_BED_SIZE / 2 - X_PROBE_OFFSET_FROM_EXTRUDER,
                        Y_BED_SIZE / 2 - Y_PROBE_OFFSET_FROM_EXTRUDER,
                        Z_CLEARANCE_DEPLOY_PROBE);

    LCD::set_status(F("Measuring Z-height..."));
    DEPLOY_PROBE();
    auto zHeight = run_z_probe();
    do_blocking_move_to_z(current_position[Z_AXIS] + Z_CLEARANCE_BETWEEN_PROBES, MMM_TO_MMS(Z_PROBE_SPEED_FAST));
    LCD::reset_message();

    feedrate_mm_s = old_feedrate_mm_s;

    sensor_.send_z_height_to_lcd(-zHeight);
    pages_.show_page(Page::SensorSettings, false);
#endif
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// No Sensor
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


void Printer_::no_sensor(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Cancel:          no_sensor_back(); break;
        default:                        Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }

}

void Printer_::no_sensor_back()
{
    pages_.show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Change Filament
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::change_filament(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:            change_filament_show(); break;
        case KeyValue::Save:            change_filament_continue(); break;
        default:                        Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::change_filament_show()
{
}

void Printer_::change_filament_continue()
{
    // TODO
}


// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Firmware Settings
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::firmware(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:                firmware_settings_show(); break;
        case KeyValue::ThermalProtection:   firmware_settings_thermal_protection(); break;
        case KeyValue::USBBaudrateMinus:    firmware_settings_baudrate_minus(); break;
        case KeyValue::USBBaudratePlus:     firmware_settings_baudrate_plus(); break;
        case KeyValue::Save:                firmware_settings_save(); break;
        case KeyValue::Back:                firmware_settings_back(); break;
        default:                            Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::firmware_settings_show()
{
    usb_old_baudrate_ = usb_baudrate_;
    send_usb_baudrate();
    send_features();

    pages_.save_forward_page();
    pages_.show_page(Page::Firmware);
}

void Printer_::firmware_settings_thermal_protection()
{
    flip_bits(features_, Feature::ThermalProtection);
    send_features();
}

void Printer_::firmware_settings_save()
{
    save_settings();
    pages_.show_back_page();

    change_usb_baudrate();
}

void Printer_::firmware_settings_back()
{
    usb_baudrate_ = usb_old_baudrate_;
    pages_.show_back_page();
}

void Printer_::send_usb_baudrate()
{
    String value; value << usb_baudrate_;

    WriteRamDataRequest frame{Variable::ValueText};
    frame << FixedSizeString{value, 6};
    frame.send();
}

static size_t UsbBaudrateIndex(uint32_t baudrate)
{
    size_t nb = countof(usb_baudrates);
    for(size_t i = 0; i < nb; ++i)
        if(baudrate == usb_baudrates[i])
            return i;
    return 0;
}

void Printer_::firmware_settings_baudrate_minus()
{
    auto index = UsbBaudrateIndex(usb_baudrate_);
    usb_baudrate_ = index > 0 ? usb_baudrates[index - 1] : usb_baudrates[0];
    send_usb_baudrate();
}

void Printer_::firmware_settings_baudrate_plus()
{
    auto index = UsbBaudrateIndex(usb_baudrate_);
    static const auto max = countof(usb_baudrates) - 1;
    usb_baudrate_ = index < max ? usb_baudrates[index + 1] : usb_baudrates[max];
    send_usb_baudrate();
}

void Printer_::change_usb_baudrate()
{
    // We do not use Log because this message is always output (Log is only active in DEBUG
    SERIAL_ECHO(F("Switch USB baudrate to ")); SERIAL_ECHO(usb_baudrate_); SERIAL_ECHO("\r\n");

    // wait for last transmitted data to be sent
    SERIAL_FLUSH();
    SERIAL_ECHO_START();
    SERIAL_ECHO(usb_baudrate_);
    // empty out possible garbage from input buffer
    while(MYSERIAL0.available())
        MYSERIAL0.read();

    // We do not use Log because this message is always output (Log is only active in DEBUG
    SERIAL_ECHO(F("\r\nUSB baudrate switched to ")); SERIAL_ECHO(usb_baudrate_); SERIAL_ECHO("\r\n");
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// LCD Settings
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::lcd(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Show:                lcd_settings_show(); break;
        case KeyValue::LCDDimming:          lcd_settings_dimming(); break;
        case KeyValue::Buzzer:              lcd_settings_buzzer(); break;
        case KeyValue::BuzzOnPress:         lcd_settings_buzz_on_press(); break;
        case KeyValue::Back:                lcd_settings_back(); break;
        default:                            Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void Printer_::send_features()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(static_cast<uint16_t>(features_));
    frame.send();
}

void Printer_::lcd_settings_show()
{
    send_features();
    pages_.save_forward_page();
    pages_.show_page(Page::LCD);
}

void Printer_::lcd_settings_dimming()
{
    flip_bits(features_, Feature::Dimming);
    dimming_.enable(test_one_bit(features_, Feature::Dimming));
    send_features();
    save_settings();
}

void Printer_::lcd_settings_buzzer()
{
    flip_bits(features_, Feature::Buzzer);
    LCD::enable_buzzer(test_one_bit(features_, Feature::Buzzer));
    send_features();
    save_settings();
}

void Printer_::lcd_settings_buzz_on_press()
{
    flip_bits(features_, Feature::BuzzOnPress);
    LCD::enable_buzz_on_press(test_one_bit(features_, Feature::BuzzOnPress));
    send_features();
    save_settings();
}

void Printer_::lcd_settings_back()
{
    pages_.show_back_page();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Status
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Send statistics to the LCD screen.
void Printer_::send_stats()
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
                  << static_cast<unsigned int>(stats.filamentUsed / 100) % 10
                  << "m";
    frame.reset(Variable::TotalFilament);
    frame << FixedSizeString{filament_used, 16};
    frame.send();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Errors
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

//! Display the Thermal Runaway Error screen.
void Printer_::temperature_error(const __FlashStringHelper* message)
{
    LCD::set_status(message);
    send_status_data(true);
    pages_.show_page(advi3pp::Page::ThermalRunawayError);
}

void Printer_::check_and_fix()
{
    if(Planner::max_jerk[X_AXIS] == 0) Planner::max_jerk[X_AXIS] = DEFAULT_XJERK;
    if(Planner::max_jerk[Y_AXIS] == 0) Planner::max_jerk[Y_AXIS] = DEFAULT_YJERK;
    if(Planner::max_jerk[Z_AXIS] == 0) Planner::max_jerk[Z_AXIS] = DEFAULT_ZJERK;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Linear Advance
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

void Printer_::linear_advance_tuning(KeyValue key_value)
{

}

void Printer_::linear_advance_settings(KeyValue key_value)
{

}

void Printer_::diagnosis(KeyValue key_value)
{

}

// --------------------------------------------------------------------
// Settings
// --------------------------------------------------------------------

Handler::Handler(PagesManager& pages)
        : pages_{pages}
{
}

void Handler::handle(KeyValue value)
{
    if(!dispatch(value))
        invalid(value);
}

bool Handler::dispatch(KeyValue value)
{
    switch(value)
    {
        case KeyValue::Show:        show(); break;
        case KeyValue::Save:        save(); break;
        case KeyValue::Back:        back(); break;
        default:                    return false;
    }

    return true;
}

void Handler::invalid(KeyValue value)
{
    Log::error()
            << F("Invalid key value ")
            << static_cast<uint16_t>(value) << Log::endl();
}

void Handler::show(bool save_forward, bool save_back)
{
    do_backup();
    if(save_forward)
        pages_.save_forward_page();

    Page page = do_show();
    if(page != Page::None)
        pages_.show_page(page, save_back);
}

void Handler::do_backup()
{
}

void Handler::do_rollback()
{
}

Page Handler::do_show()
{
    return Page::None;
}

void Handler::save()
{
    do_save();
    enqueue_and_echo_commands_P(PSTR("M500"));
    pages_.show_forward_page();
}

void Handler::do_save()
{
}

void Handler::back()
{
    do_rollback();
    pages_.show_back_page();
}

// --------------------------------------------------------------------
// Print Settings
// --------------------------------------------------------------------

bool PrintSettings::dispatch(KeyValue key_value)
{
    if(Handler::dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::Baby001:     [this]{ multiplier_ = 0.01; }; break;
        case KeyValue::Baby005:     [this]{ multiplier_ = 0.05; }; break;
        case KeyValue::Baby010:     [this]{ multiplier_ = 0.10; }; break;
        default:                    return false;
    }

    return true;
}

//! Display on the LCD screen the printing settings.
Page PrintSettings::do_show()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(feedrate_percentage)
          << Uint16(scale(fanSpeeds[0], 255, 100))
          << Uint16(Temperature::degTargetHotend(0))
          << Uint16(Temperature::degTargetBed());
    frame.send();

    return Page::PrintSettings;
}

//! Save the printing settings.
void PrintSettings::do_save()
{
    ReadRamData response{Variable::Value0, 5};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Print Settings)") << Log::endl();
        return;
    }

    Uint16 speed, fan, hotend, bed;
    response >> speed >> hotend >> bed >> fan;

    feedrate_percentage = speed.word;
    Temperature::setTargetHotend(hotend.word, 0);
    Temperature::setTargetBed(bed.word);
    fanSpeeds[0] = scale(fan.word, 100, 255);
}

void PrintSettings::baby_minus()
{
    String auto_pid_command; auto_pid_command << F("M290 Z-") << multiplier_;
    enqueue_and_echo_command(auto_pid_command.c_str());
}

void PrintSettings::baby_plus()
{
    String auto_pid_command; auto_pid_command << F("M290 Z") << multiplier_;
    enqueue_and_echo_command(auto_pid_command.c_str());
}

// --------------------------------------------------------------------
// PidSettings
// --------------------------------------------------------------------

void PidSettings::do_backup()
{
    backup_.Kp_ = Temperature::Kp;
    backup_.Ki_ = Temperature::Ki;
    backup_.Kd_ = Temperature::Kd;
}

void PidSettings::do_rollback()
{
    Temperature::Kp = backup_.Kp_;
    Temperature::Ki = backup_.Ki_;
    Temperature::Kd = backup_.Kd_;
}

void PidSettings::set(uint16_t temperature, bool bed)
{
    bed_ = bed;
    for(size_t i = 0; i < NB_PIDs; ++i)
    {
        if(temperature == pid_[bed_][i].temperature_)
        {
            index_ = i;
            return;
        }
    }

    // Temperature not found, so assign index 0, move PIDs and forget the last one
    index_ = 0;
    for(size_t i = 1; i < NB_PIDs; ++i)
        pid_[bed_][i] = pid_[bed_][i - 1];
}

//! Show the PID settings
Page PidSettings::do_show()
{
    const Pid& pid = pid_[bed_][index_];
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(bed_ ? 0_u16 : 1_u16)
          << Uint16(pid.Kp_ * 100)
          << Uint16(unscalePID_i(pid.Ki_) * 100)
          << Uint16(unscalePID_d(pid.Kd_) * 100);
    frame.send();

    return Page::PidSettings;
}

//! Save the PID settings
void PidSettings::do_save()
{
    Pid& pid = pid_[bed_][index_];

    ReadRamData response{Variable::Value0, 4};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (PID Settings)") << Log::endl();
        return;
    }

    Uint16 temperature, p, i, d;
    response >> temperature >> p >> i >> d;

    Temperature::Kp = static_cast<float>(p.word) / 100;
    Temperature::Ki = scalePID_i(static_cast<float>(i.word) / 100);
    Temperature::Kd = scalePID_d(static_cast<float>(d.word) / 100);
}

// --------------------------------------------------------------------
// Steps Settings
// --------------------------------------------------------------------

//! Initialize temporary Step settings.
void StepSettings::do_backup()
{
    backup_[X_AXIS] = Planner::axis_steps_per_mm[X_AXIS];
    backup_[Y_AXIS] = Planner::axis_steps_per_mm[Y_AXIS];
    backup_[Z_AXIS] = Planner::axis_steps_per_mm[Z_AXIS];
    backup_[E_AXIS] = Planner::axis_steps_per_mm[E_AXIS];
}

//! Save temporary Step settings.
void StepSettings::do_rollback()
{
    Planner::axis_steps_per_mm[X_AXIS] = backup_[X_AXIS];
    Planner::axis_steps_per_mm[Y_AXIS] = backup_[Y_AXIS];
    Planner::axis_steps_per_mm[Z_AXIS] = backup_[Z_AXIS];
    Planner::axis_steps_per_mm[E_AXIS] = backup_[E_AXIS];

    Printer::save_settings();
}

//! Show the Steps settings
//! @param init     Initialize the settings are use those already set
Page StepSettings::do_show()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(Planner::axis_steps_per_mm[X_AXIS] * 10)
          << Uint16(Planner::axis_steps_per_mm[Y_AXIS] * 10)
          << Uint16(Planner::axis_steps_per_mm[Z_AXIS] * 10)
          << Uint16(Planner::axis_steps_per_mm[E_AXIS] * 10);
    frame.send();

    return Page::StepsSettings;
}

//! Save the Steps settings
void StepSettings::do_save()
{
    ReadRamData response{Variable::Value0, 4};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Steps Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z, e;
    response >> x >> y >> z >> e;

    Planner::axis_steps_per_mm[X_AXIS] = static_cast<float>(x.word) / 10;
    Planner::axis_steps_per_mm[Y_AXIS] = static_cast<float>(y.word) / 10;
    Planner::axis_steps_per_mm[Z_AXIS] = static_cast<float>(z.word) / 10;
    Planner::axis_steps_per_mm[E_AXIS] = static_cast<float>(e.word) / 10;
}

// --------------------------------------------------------------------
// Feedrate Settings
// --------------------------------------------------------------------

//! Initialize temporary Feedrate settings.
void FeedrateSettings::do_backup()
{
    backup_max_feedrate_mm_s_[X_AXIS] = Planner::max_feedrate_mm_s[X_AXIS];
    backup_max_feedrate_mm_s_[Y_AXIS] = Planner::max_feedrate_mm_s[Y_AXIS];
    backup_max_feedrate_mm_s_[Z_AXIS] = Planner::max_feedrate_mm_s[Z_AXIS];
    backup_max_feedrate_mm_s_[E_AXIS] = Planner::max_feedrate_mm_s[E_AXIS];
    backup_min_feedrate_mm_s_ = Planner::min_feedrate_mm_s;
    backup_min_travel_feedrate_mm_s_ = Planner::min_travel_feedrate_mm_s;
}

//! Save temporary Feedrate settings.
void FeedrateSettings::do_rollback()
{
    Planner::max_feedrate_mm_s[X_AXIS] = backup_max_feedrate_mm_s_[X_AXIS];
    Planner::max_feedrate_mm_s[Y_AXIS] = backup_max_feedrate_mm_s_[Y_AXIS];
    Planner::max_feedrate_mm_s[Z_AXIS] = backup_max_feedrate_mm_s_[Z_AXIS];
    Planner::max_feedrate_mm_s[E_AXIS] = backup_max_feedrate_mm_s_[E_AXIS];
    Planner::min_feedrate_mm_s = backup_min_feedrate_mm_s_;
    Planner::min_travel_feedrate_mm_s = backup_min_travel_feedrate_mm_s_;

    Printer::save_settings();
}

//! Show the Feedrate settings
Page FeedrateSettings::do_show()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(Planner::max_feedrate_mm_s[X_AXIS])
          << Uint16(Planner::max_feedrate_mm_s[Y_AXIS])
          << Uint16(Planner::max_feedrate_mm_s[Z_AXIS])
          << Uint16(Planner::max_feedrate_mm_s[E_AXIS])
          << Uint16(Planner::min_feedrate_mm_s)
          << Uint16(Planner::min_travel_feedrate_mm_s);
    frame.send();

    return Page::FeedrateSettings;
}

//! Save the Feedrate settings
void FeedrateSettings::do_save()
{
    ReadRamData response{Variable::Value0, 6};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Feedrate Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z, e, min, travel;
    response >> x >> y >> z >> e >> min >> travel;

    Planner::max_feedrate_mm_s[X_AXIS] = static_cast<float>(x.word);
    Planner::max_feedrate_mm_s[Y_AXIS] = static_cast<float>(y.word);
    Planner::max_feedrate_mm_s[Z_AXIS] = static_cast<float>(z.word);
    Planner::max_feedrate_mm_s[E_AXIS] = static_cast<float>(e.word);
    Planner::min_feedrate_mm_s         = static_cast<float>(min.word);
    Planner::min_travel_feedrate_mm_s  = static_cast<float>(travel.word);
}

// --------------------------------------------------------------------
// AccelerationSettings
// --------------------------------------------------------------------

//! Initialize temporary Acceleration settings.
void AccelerationSettings::do_backup()
{
    backup_max_acceleration_mm_per_s2_[X_AXIS] = Planner::max_acceleration_mm_per_s2[X_AXIS];
    backup_max_acceleration_mm_per_s2_[Y_AXIS] = Planner::max_acceleration_mm_per_s2[Y_AXIS];
    backup_max_acceleration_mm_per_s2_[Z_AXIS] = Planner::max_acceleration_mm_per_s2[Z_AXIS];
    backup_max_acceleration_mm_per_s2_[E_AXIS] = Planner::max_acceleration_mm_per_s2[E_AXIS];
    backup_acceleration_ = Planner::acceleration;
    backup_retract_acceleration_ = Planner::retract_acceleration;
    backup_travel_acceleration_ = Planner::travel_acceleration;
}

//! Save temporary Acceleration settings.
void AccelerationSettings::do_rollback()
{
    Planner::max_acceleration_mm_per_s2[X_AXIS] = backup_max_acceleration_mm_per_s2_[X_AXIS];
    Planner::max_acceleration_mm_per_s2[Y_AXIS] = backup_max_acceleration_mm_per_s2_[Y_AXIS];
    Planner::max_acceleration_mm_per_s2[Z_AXIS] = backup_max_acceleration_mm_per_s2_[Z_AXIS];
    Planner::max_acceleration_mm_per_s2[E_AXIS] = backup_max_acceleration_mm_per_s2_[E_AXIS];
    Planner::acceleration = backup_acceleration_;
    Planner::retract_acceleration = backup_retract_acceleration_;
    Planner::travel_acceleration =  backup_travel_acceleration_;

    Printer::save_settings();
}

//! Show the Acceleration settings
Page AccelerationSettings::do_show()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(static_cast<uint16_t>(Planner::max_acceleration_mm_per_s2[X_AXIS]))
          << Uint16(static_cast<uint16_t>(Planner::max_acceleration_mm_per_s2[Y_AXIS]))
          << Uint16(static_cast<uint16_t>(Planner::max_acceleration_mm_per_s2[Z_AXIS]))
          << Uint16(static_cast<uint16_t>(Planner::max_acceleration_mm_per_s2[E_AXIS]))
          << Uint16(static_cast<uint16_t>(Planner::acceleration))
          << Uint16(static_cast<uint16_t>(Planner::retract_acceleration))
          << Uint16(static_cast<uint16_t>(Planner::travel_acceleration));
    frame.send();

    return Page::AccelerationSettings;
}

//! Save the Acceleration settings
void AccelerationSettings::do_save()
{
    ReadRamData response{Variable::Value0, 7};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Acceleration Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z, e, print, retract, travel;
    response >> x >> y >> z >> e >> print >> retract >> travel;

    Planner::max_acceleration_mm_per_s2[X_AXIS] = static_cast<uint32_t>(x.word);
    Planner::max_acceleration_mm_per_s2[Y_AXIS] = static_cast<uint32_t>(y.word);
    Planner::max_acceleration_mm_per_s2[Z_AXIS] = static_cast<uint32_t>(z.word);
    Planner::max_acceleration_mm_per_s2[E_AXIS] = static_cast<uint32_t>(e.word);
    Planner::acceleration                       = static_cast<float>(print.word);
    Planner::retract_acceleration               = static_cast<float>(retract.word);
    Planner::travel_acceleration                = static_cast<float>(travel.word);
}

// --------------------------------------------------------------------
// JerkSettings
// --------------------------------------------------------------------

//! Initialize temporary Jerk settings.
void JerkSettings::do_backup()
{
    backup_max_jerk_[X_AXIS] = Planner::max_jerk[X_AXIS];
    backup_max_jerk_[Y_AXIS] = Planner::max_jerk[Y_AXIS];
    backup_max_jerk_[Z_AXIS] = Planner::max_jerk[Z_AXIS];
    backup_max_jerk_[E_AXIS] = Planner::max_jerk[E_AXIS];
}

//! Save temporary Jerk settings.
void JerkSettings::do_rollback()
{
    Planner::max_jerk[X_AXIS] = backup_max_jerk_[X_AXIS];
    Planner::max_jerk[Y_AXIS] = backup_max_jerk_[Y_AXIS];
    Planner::max_jerk[Z_AXIS] = backup_max_jerk_[Z_AXIS];
    Planner::max_jerk[E_AXIS] = backup_max_jerk_[E_AXIS];

    Printer::save_settings();
}

//! Show the Jerk settings
Page JerkSettings::do_show()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(Planner::max_jerk[X_AXIS] * 10)
          << Uint16(Planner::max_jerk[Y_AXIS] * 10)
          << Uint16(Planner::max_jerk[Z_AXIS] * 10)
          << Uint16(Planner::max_jerk[E_AXIS] * 10);
    frame.send();

    return Page::JerkSettings;
}

//! Save the Jerk settings
void JerkSettings::do_save()
{
    ReadRamData response{Variable::Value0, 4};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Acceleration Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z, e;
    response >> x >> y >> z >> e;

    Planner::max_jerk[X_AXIS] = x.word / 10.0;
    Planner::max_jerk[Y_AXIS] = y.word / 10.0;
    Planner::max_jerk[Z_AXIS] = z.word / 10.0;
    Planner::max_jerk[E_AXIS] = e.word / 10.0;
}

// --------------------------------------------------------------------
// Dimming
// --------------------------------------------------------------------

void Printer_::set_brightness(int16_t brightness)
{
    dimming_.change_brightness(brightness);
}

Dimming::Dimming()
{
    set_next_checking_time();
    set_next_dimmming_time();
}

void Dimming::enable(bool enable)
{
    enabled_ = enable;
    reset();
}

void Dimming::set_next_checking_time()
{
    next_check_time_ = millis() + 200;
}

void Dimming::set_next_dimmming_time()
{
    next_dimming_time_ = millis() + 1000 * DIMMING_DELAY;
}

uint8_t Dimming::get_adjusted_brithness()
{
    if(dimming_)
        lcd_contrast = lcd_contrast * DIMMING_RATIO / 100;
    if(lcd_contrast < BRIGHTNESS_MIN)
        lcd_contrast = BRIGHTNESS_MIN;
    if(lcd_contrast > BRIGHTNESS_MAX)
        lcd_contrast = BRIGHTNESS_MAX;
    return static_cast<uint8_t>(lcd_contrast);
}

void Dimming::check()
{
    if(!enabled_ || !ELAPSED(millis(), next_check_time_))
        return;
    set_next_checking_time();

    ReadRegister read{Register::TouchPanelFlag, 1};
    if(!read.send_and_receive(false))
    {
        Log::error() << F("Reading TouchPanelFlag") << Log::endl();
        return;
    }

    Uint8 value; read >> value;
    if(value.byte == 0x5A)
	{
		WriteRegisterDataRequest request{Register::TouchPanelFlag};
		request << 00_u8;
		request.send();
        reset();
	}
    else if(!dimming_ && ELAPSED(millis(), next_dimming_time_))
    {
        dimming_ = true;
        send_brightness();
    }
}

void Dimming::reset()
{
    dimming_ = false;
    set_next_dimmming_time();
    send_brightness();
}

void Dimming::send_brightness()
{
    auto brightness = get_adjusted_brithness();

    WriteRegisterDataRequest frame{Register::Brightness};
    frame << Uint8{brightness};
    frame.send(true);
}

void Dimming::change_brightness(int16_t brightness)
{
    reset();
    send_brightness();
}

// --------------------------------------------------------------------
// BLTouch
// --------------------------------------------------------------------

#ifdef ADVi3PP_BLTOUCH

Sensor::Sensor(PagesManager& pages)
: pages_{pages}
{
}

void Sensor::send_z_height_to_lcd(double z_height)
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(z_height * 100);
    frame.send();
}

void Sensor::save_lcd_z_height()
{
    ReadRamData response{Variable::Value0, 1};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Sensor Settings)") << Log::endl();
        return;
    }

    Uint16 offsetZ; response >> offsetZ;
    save_z_height(static_cast<int16_t>(offsetZ.word) / 100.0);
}

void Sensor::save_z_height(double height)
{
    String command; command << F("M851 Z") << height;
    enqueue_and_echo_command(command.c_str());
    Printer::save_settings();
}

void Sensor::self_test()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S120"));
}

void Sensor::reset()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S160"));
}

void Sensor::deploy()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S10"));
}

void Sensor::stow()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S90"));
}

#endif

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Preheat
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


// --------------------------------------------------------------------
// Graphs
// --------------------------------------------------------------------

Graphs::Graphs()
{
    next_update_graph_time_ = millis() + 1000L * 10; // Wait 10 sec before starting updating graphs
}

void Graphs::update()
{
    if(!ELAPSED(millis(), next_update_graph_time_))
        return;

    send_data();
    next_update_graph_time_ = millis() + 500;
}

//! Update the graphics (two channels: the bed and the hotend).
void Graphs::send_data()
{
    WriteCurveDataRequest frame{0b00000011};
    frame << Uint16{Temperature::degBed()}
          << Uint16{Temperature::degHotend(0)};
    frame.send(false);
}

void Graphs::clear()
{
    WriteRegisterDataRequest request{Register::TrendlineClear}; // TODO: Fix this (Mini DGUS)
    request << 0x55_u8;
    request.send();
}

// --------------------------------------------------------------------
// Background tasks
// --------------------------------------------------------------------

Task::Task(Printer_& printer, PagesManager& pages)
: printer_{printer}, pages_{pages}
{
}

//! Set the next (minimal) update time
//! @param delta    Duration to be added to the current time to compute the next (minimal) update time
void Task::set_next_update_time(unsigned int delta)
{
    next_update_time_ = millis() + delta;
}

bool Task::is_update_time()
{
    auto current_time = millis();
    if(!ELAPSED(current_time, next_update_time_))
        return false;
    set_next_update_time();
    return true;
}

//! Set the next background task and its delay
//! @param task     The next background task
//! @param delta    Duration to be added to the current time to execute the background task
void Task::set_background_task(BackgroundTask task, unsigned int delta)
{
    assert(!background_task_);

    op_time_delta_ = delta;
    background_task_ = task;
    next_op_time_ = millis() + delta;
}

//! Reset the background task
void Task::clear_background_task()
{
    background_task_ = nullptr;
}

//! If there is an operating running, execute its next step
void Task::execute_background_task()
{
    if(!background_task_ || !ELAPSED(millis(), next_op_time_))
        return;

    next_op_time_ = millis() + op_time_delta_;
    background_task_();
}

bool Task::has_background_task() const
{
    return bool(background_task_);
}

// --------------------------------------------------------------------
// Advance pause
// --------------------------------------------------------------------

void Printer_::advanced_pause_show_message(const AdvancedPauseMessage message)
{
    pause_.advanced_pause_show_message(message);
}

AdvancedPause::AdvancedPause(PagesManager& pages)
: pages_{pages}
{
}

void AdvancedPause::advanced_pause_show_message(const AdvancedPauseMessage message)
{
    if(message == last_advanced_pause_message_)
        return;
    last_advanced_pause_message_ = message;

    switch (message)
    {
        case ADVANCED_PAUSE_MESSAGE_INIT:                       init(); break;
        case ADVANCED_PAUSE_MESSAGE_UNLOAD:                     advi3pp::LCD::set_status(F("Unloading filament...")); break;
        case ADVANCED_PAUSE_MESSAGE_INSERT:                     insert_filament(); break;
        case ADVANCED_PAUSE_MESSAGE_EXTRUDE:                    advi3pp::LCD::set_status(F("Extruding some filament...")); break;
        case ADVANCED_PAUSE_MESSAGE_CLICK_TO_HEAT_NOZZLE:       advi3pp::LCD::set_status(F("Press continue to heat")); break;
        case ADVANCED_PAUSE_MESSAGE_RESUME:                     advi3pp::LCD::set_status(F("Resuming print...")); break;
        case ADVANCED_PAUSE_MESSAGE_STATUS:                     printing(); break;
        case ADVANCED_PAUSE_MESSAGE_WAIT_FOR_NOZZLES_TO_HEAT:   advi3pp::LCD::set_status(F("Waiting for heat...")); break;
        case ADVANCED_PAUSE_MESSAGE_OPTION:                     advanced_pause_menu_response = ADVANCED_PAUSE_RESPONSE_RESUME_PRINT; break;
        default: advi3pp::Log::log() << F("Unknown AdvancedPauseMessage: ") << static_cast<uint16_t>(message) << advi3pp::Log::endl(); break;
    }
}

void AdvancedPause::init()
{
    pages_.save_forward_page();
    pages_.show_wait_page(F("Pausing..."));
}

void AdvancedPause::insert_filament()
{
    pages_.show_wait_continue_page(F("Insert filament and press continue..."),
        WaitCalllback(this, &AdvancedPause::filament_inserted), false);
}

void AdvancedPause::filament_inserted()
{
    ::wait_for_user = false;
    this->pages_.show_wait_page(F("Filament inserted.."), false);
}

void AdvancedPause::printing()
{
    advi3pp::LCD::set_status(F("Printing"));
    pages_.show_forward_page();
}

}

#if ENABLED(ADVANCED_PAUSE_FEATURE)

void lcd_advanced_pause_show_message(const AdvancedPauseMessage message,
                                     const AdvancedPauseMode mode=ADVANCED_PAUSE_MODE_PAUSE_PRINT,
                                     const uint8_t extruder=active_extruder)
{
    advi3pp::printer.advanced_pause_show_message(message);
}
#endif // ADVANCED_PAUSE_FEATURE


void EepromMismatch::eeprom_mimatch(KeyValue key_value)
{
    switch(key_value)
    {
        case KeyValue::Continue:        eeprom_mimatch_continue(); break;
        default:                        Log::error() << F("Invalid key value ") << static_cast<uint16_t>(key_value) << Log::endl(); break;
    }
}

void EepromMismatch::eeprom_mimatch_continue()
{
    save_settings();
    pages_.show_page(Page::Main, false);
}


