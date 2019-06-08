/**
 * Marlin 3D Printer Firmware For Wanhao Duplicator i3 Plus (ADVi3++)
 *
 * Copyright (C) 2017-2019 Sebastien Andrivet [https://github.com/andrivet/]
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
#include "duration_t.h"

#include "advi3pp.h"
#include "advi3pp_log.h"
#include "advi3pp_dgus.h"
#include "advi3pp_stack.h"
#include "advi3pp_versions.h"
#include "advi3pp_.h"

#include <HardwareSerial.h>

// --------------------------------------------------------------------
// From Marlin and Arduino
// --------------------------------------------------------------------

uint8_t progress_bar_percent;
int16_t lcd_contrast;
extern int freeMemory();

#ifdef ADVi3PP_PROBE
bool set_probe_deployed(bool);
float run_z_probe();
extern float zprobe_zoffset;
#endif

// --------------------------------------------------------------------


namespace
{
    //! List of possible baudrates
    constexpr uint32_t usb_baudrates[] = {9600, 19200, 38400, 57600, 115200, 230400, 250000};

    //! Default preheat values
    const advi3pp::Preset DEFAULT_PREHEAT_PRESET[advi3pp::Preheat::NB_PRESETS] = {
        {180, 50, 0},
        {200, 60, 0},
        {220, 70, 0},
        {180, 00, 0},
        {200, 00, 0}
    };

    //! List of multipliers in Print Settings
    const double PRINT_SETTINGS_MULTIPLIERS[] = {0.04, 0.08, 0.12};

#ifdef ADVi3PP_PROBE
    //! List of multipliers in Z-height Tuning
    const double SENSOR_Z_HEIGHT_MULTIPLIERS[] = {0.04, 0.12, 1.0};

    //! Get the name of a sensor holder
    //! @param index Index of the holder
    //! @return The name (in Flash memory) of the holder
    const FlashChar* get_sensor_name(size_t index)
    {
        // Note: F macro can be used only in a function, this is why this is coded like this
        auto teaching_tech_side  = F("Teaching Tech L. Side");
        auto custom              = F("Custom");

#if defined(ADVi3PP_MARK2)
        auto mark2               = F("Mark II");
        static const FlashChar* names[advi3pp::SensorSettings::NB_SENSOR_POSITIONS] =
          {mark2, teaching_tech_side, custom};
#elif defined(ADVi3PP_BLTOUCH)
        auto baseggio            = F("Indianagio Front");
        static const FlashChar* names[advi3pp::SensorSettings::NB_SENSOR_POSITIONS] =
          {baseggio, teaching_tech_side, custom};
#endif
        assert(index < advi3pp::SensorSettings::NB_SENSOR_POSITIONS);
        return names[index];
    }

    //! Default position of the sensor for the different holders
    const advi3pp::SensorPosition DEFAULT_SENSOR_POSITION[advi3pp::SensorSettings::NB_SENSOR_POSITIONS] =
    {
#if defined(ADVi3PP_MARK2)
        {     0,  6000 },    // Mark II
        { -2400, -3800 },    // Teaching Tech L. Side
        {     0,     0 }     // Custom
#elif defined(ADVi3PP_BLTOUCH)
        {     0, -3890 },    // Baseggio Front
        { -2400, -3800 },    // Teaching Tech L. Side
        {     0,     0 }     // Custom
#endif
    };
#endif

    //! List of digital pins for the Diagnosis page
    const uint8_t diagnosis_digital_pins[] =
    {
        54,     // PF0 / ADC0 - A0
        24,     // PA2 / AD2
        23,     // PA1 / AD1
         6,     // PH3 / OC4A
        25,     // PA3 / AD3

        40,     // PG1 / !RD
        56,     // PF2 / ADC2 - A2
        36,     // PC1 / A9
        37,     // PC0 / A8

        34,     // PC3 / A11
        35,     // PC2 / A10
        32,     // PC5 / A13
        33,     // PC4 / A12
    };

    //! List of analogic pins for the Diagnosis page
    const uint8_t diagnosis_analog_pins[] = {55, 68, 54, 56}; // A1, A14, A0, A2
}

namespace advi3pp {

inline namespace singletons
{
    extern Feature features;
    extern Dimming dimming;
    extern Graphs graphs;

    Screens screens;
    Wait wait;
    Temperatures temperatures;
    LoadUnload load_unload;
    Preheat preheat;
    Move move;
    SdCard sd_card;
    FactoryReset factory_reset;
    ManualLeveling manual_leveling;
    ExtruderTuning extruder_tuning;
    PidTuning pid_tuning;
    SensorSettings sensor_settings;
    FirmwareSettings firmware_settings;
    NoSensor no_sensor;
    LcdSettings lcd_settings;
    Statistics statistics;
    Versions versions;
    PrintSettings print_settings;
    PidSettings pid_settings;
    StepSettings steps_settings;
    FeedrateSettings feedrates_settings;
    AccelerationSettings accelerations_settings;
    JerkSettings jerks_settings;
    Copyrights copyrights;
    AutomaticLeveling automatic_leveling;
    LevelingGrid leveling_grid;
    SensorTuning sensor_tuning;
    SensorZHeight sensor_z_height;
    ChangeFilament change_filament;
    EepromMismatch eeprom_mismatch;
    Sponsors sponsors;
    LinearAdvanceTuning linear_advance_tuning;
    LinearAdvanceSettings linear_advance_settings;
    Diagnosis diagnosis;
    Print print;
    AdvancedPause pause;
}

// --------------------------------------------------------------------
// Pages management
// --------------------------------------------------------------------

//! Show the given page on the LCD screen
//! @param [in] page The page to be displayed on the LCD screen
void Pages::show_page(Page page, ShowOptions options)
{
    Log::log() << F("Show page ") << static_cast<uint8_t>(page) << Log::endl();

    if(test_one_bit(options, ShowOptions::SaveBack))
    {
        auto current = get_current_page();
        Log::log() << F("Save back page ") << static_cast<uint8_t>(current) << Log::endl();
        back_pages_.push(current);
    }

    WriteRegisterDataRequest frame{Register::PictureID};
    frame << 00_u8 << page;
    frame.send(true);

    current_page_ = page;
}

//! Retrieve the current page on the LCD screen
Page Pages::get_current_page()
{
    // Boot page switches automatically (animation) to the Main page
    return current_page_ == Page::Boot ? Page::Main : current_page_;
}

//! Set page to display after the completion of an operation.
void Pages::save_forward_page()
{
    auto current = get_current_page();
    Log::log() << F("Save forward page ") << static_cast<uint8_t>(current) << Log::endl();
    forward_page_ = current;
}

//! Show the "Back" page on the LCD display.
void Pages::show_back_page()
{
    forward_page_ = Page::None;

    if(back_pages_.is_empty())
    {
        Log::log() << F("No back page, show Main") << Log::endl();
        show_page(Page::Main, ShowOptions::None);
        return;
    }

    auto back = back_pages_.pop();
    Log::log() << F("Pop back page ") << static_cast<uint8_t>(back) << Log::endl();
    show_page(back, ShowOptions::None);
}

//! Show the "Next" page on the LCD display.
void Pages::show_forward_page()
{
    if(forward_page_ == Page::None)
    {
        show_back_page();
        return;
    }

    if(!back_pages_.contains(forward_page_))
    {
        Log::error() << F("Back pages do not contain forward page ") << static_cast<uint8_t>(forward_page_) << Log::endl();
        return;
    }

    while(!back_pages_.is_empty())
    {
        Page back_page = back_pages_.pop();
        Log::log() << F("Pop back page ") << static_cast<uint8_t>(back_page) << Log::endl();
        if(back_page == forward_page_)
        {
            Log::log() << F("Show forward page ") << static_cast<uint8_t>(forward_page_) << Log::endl();
            show_page(forward_page_, ShowOptions::None);
            forward_page_ = Page::None;
            return;
        }
    }
}

// --------------------------------------------------------------------
// Screens
// --------------------------------------------------------------------

//! Dispatch a key value to the right handler
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool Screens::do_dispatch(KeyValue key_value)
{
    // Do not call Parent::do_dispatch

    switch(key_value)
    {
        case KeyValue::Temps:           show_temps(); break;
        case KeyValue::Print:           show_print(); break;
        case KeyValue::Controls:        pages.show_page(Page::Controls); break;
        case KeyValue::Tuning:          pages.show_page(Page::Tuning); break;
        case KeyValue::Settings:        pages.show_page(Page::Settings); break;
        case KeyValue::Infos:           pages.show_page(Page::Infos); break;
        case KeyValue::Motors:          pages.show_page(Page::MotorsSettings); break;
        case KeyValue::Leveling:        pages.show_page(Page::Leveling); break;
        case KeyValue::PrintSettings:   show_print_settings(); break;
        case KeyValue::Back:            back_command(); break;
        default:                        return false;
    }

    return true;
}

//! Show one of the temperature graph screens depending of the context: either the SD printing screen,
//! the printing screen or the temperature screen.
void Screens::show_temps()
{
    if(!PrintCounter::isRunning() && !PrintCounter::isPaused())
    {
        temperatures.show();
        return;
    }

    // If there is a print running (or paused), display the print screen.
    pages.show_page(Page::Print);
}

//! Show Print Settings page (only if a print is running or paused)
void Screens::show_print_settings()
{
    if(!PrintCounter::isRunning() && !PrintCounter::isPaused())
    {
        temperatures.show();
        return;
    }

    // If there is a print running (or paused), display the print settings.
    print_settings.show(ShowOptions::SaveBack);
}

//! Show one of the Printing screens depending of the context:
//! - If a print is running, display the Print screen
//! - Otherwise, try to access the SD card. Depending of the result, display the SD card Page or the Temperatures page
void Screens::show_print()
{
    // If there is a print running (or paused), display the SD or USB print screen
    if(PrintCounter::isRunning() || PrintCounter::isPaused())
    {
        pages.show_page(Page::Print);
        return;
    }

    wait.show(F("Try to access the SD card..."));
    task.set_background_task(BackgroundTask{this, &Screens::show_sd_or_temp_page});
}

//! Show the SD card page (if a SD card is inserted) or the Temperature page
void Screens::show_sd_or_temp_page()
{
    task.clear_background_task();

    card.initsd(); // Can take some time
    advi3pp.reset_status();
    if(!card.cardOK)
    {
        // SD card not accessible so fall back to Temperatures
        temperatures.show(ShowOptions::None);
        return;
    }

    sd_card.show(ShowOptions::None);
}

// --------------------------------------------------------------------
// Wait
// --------------------------------------------------------------------

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page Wait::do_prepare_page()
{
    return Page::Waiting;
}

//! Show a simple wait page with a message
//! @param message  The message to display
//! @param options  Options when displaying the page (i.e. save the current page or not)
void Wait::show(const FlashChar* message, ShowOptions options)
{
    advi3pp.set_status(message);
    back_ = nullptr;
    continue_ = nullptr;
    pages.show_page(Page::Waiting, options);
}

//! Show a simple wait page with a message
//! @param message  The message to display
//! @param back     Callback to be called when the back button is pressed
//! @param options  Options when displaying the page (i.e. save the current page or not)
void Wait::show(const FlashChar* message, const WaitCallback& back, ShowOptions options)
{
    advi3pp.set_status(message);
    back_ = back;
    continue_ = nullptr;
    pages.show_page(Page::WaitBack, options);
}

//! Show a simple wait page with a message
//! @param message  The message to display
//! @param back     Callback to be called when the back button is pressed
//! @param cont     Callback to be called when the continue button is pressed
//! @param options  Options when displaying the page (i.e. save the current page or not)
void Wait::show(const FlashChar* message, const WaitCallback& back, const WaitCallback& cont, ShowOptions options)
{
    advi3pp.set_status(message);
    back_ = back;
    continue_ = cont;
    pages.show_page(Page::WaitBackContinue, options);
}

//! Show a simple wait page with a message
//! @param message  The message to display
//! @param cont     Callback to be called when the continue button is pressed
//! @param options  Options when displaying the page (i.e. save the current page or not)
void Wait::show_continue(const FlashChar* message, const WaitCallback& cont, ShowOptions options)
{
    advi3pp.set_status(message);
    back_ = nullptr;
    continue_ = cont;
    pages.show_page(Page::WaitContinue, options);
}

//! Show a simple wait page with a message
//! @param message  The message to display
//! @param options  Options when displaying the page (i.e. save the current page or not)
void Wait::show_continue(const FlashChar* message, ShowOptions options)
{
    advi3pp.set_status(message);
    back_ = nullptr;
    continue_ = WaitCallback{this, &Wait::on_continue};
    pages.show_page(Page::WaitContinue, options);
}

//! Show a simple wait page without a message
//! @param message  The message to display
//! @param options  Options when displaying the page (i.e. save the current page or not)
void Wait::show_continue(ShowOptions options)
{
    back_ = nullptr;
    continue_ = WaitCallback{this, &Wait::on_continue};
    advi3pp.buzz();
    pages.show_page(Page::WaitContinue, options);
}

//! Default action when the continue button is pressed (inform Marlin)
bool Wait::on_continue()
{
    ::wait_for_user = false;
    return false;
}

//! Handles the Back command
void Wait::do_back_command()
{
    bool continue_processing = true;

    if(!back_)
        Log::error() << F("No Back action defined") << Log::endl();
    else
    {
        continue_processing = back_();
        back_ = nullptr;
    }

    if(continue_processing)
        Parent::do_back_command();
}

//! Handles the Save (Continue) command
void Wait::do_save_command()
{
    bool continue_processing = true;

    if(!continue_)
        Log::error() << F("No Continue action defined") << Log::endl();
    else
    {
        continue_processing = continue_();
        continue_ = nullptr;
    }

    if(continue_processing)
        pages.show_forward_page();
}

// --------------------------------------------------------------------
// Temperatures Graph
// --------------------------------------------------------------------

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page Temperatures::do_prepare_page()
{
    return Page::Temperature;
}

//! Show the temperature page and record and action to be executed when the back button is pressed
//! @param back Action to be executed when the back button is pressed
void Temperatures::show(const WaitCallback& back)
{
    back_ = back;
    Parent::show(ShowOptions::SaveBack);
}

//! Show the temperature page
//! @param options  Options when displaying the page (i.e. save the current page or not)
void Temperatures::show(ShowOptions options)
{
    back_ = nullptr;
    Parent::show(options);
}

//! Execute the Back command
void Temperatures::do_back_command()
{
    if(back_)
    {
        back_();
        back_ = nullptr;
    }

    Parent::do_back_command();
}

// --------------------------------------------------------------------
// Load and Unload Filament
// --------------------------------------------------------------------

//! Handle Load & Unload actions.
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool LoadUnload::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::Load:    load_command(); break;
        case KeyValue::Unload:  unload_command(); break;
        default:                return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page LoadUnload::do_prepare_page()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(advi3pp.get_last_used_temperature(TemperatureKind::Hotend));
    frame.send();
    return Page::LoadUnload;
}

//! Prepare Load or Unload step #1: set the target temperature, setup the next step and display a wait message
//! @param background Background task to detect if it is time for step #2
void LoadUnload::prepare(const BackgroundTask& background)
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

    task.set_background_task(background);
    wait.show(F("Wait until the target temp is reached..."), WaitCallback{this, &LoadUnload::stop});
}

//! Start Load action.
void LoadUnload::load_command()
{
    prepare(BackgroundTask(this, &LoadUnload::load_start_task));
}

//! Start Unload action.
void LoadUnload::unload_command()
{
    prepare(BackgroundTask(this, &LoadUnload::unload_start_task));
}

//! Handle back from the Load or Unload LCD screen: stop the process.
//! @return true to continue Back processing
bool LoadUnload::stop()
{
    Log::log() << F("Load/Unload Stop") << Log::endl();

    advi3pp.reset_status();
    task.set_background_task(BackgroundTask(this, &LoadUnload::stop_task));
    clear_command_queue();
    Temperature::setTargetHotend(0, 0);
    return true;
}

//! Check if the process is actually stopped and reset E axis
void LoadUnload::stop_task()
{
    if(advi3pp.is_busy() || !task.has_background_task())
        return;

    task.clear_background_task();

    // Do this asynchronously to avoid race conditions
    enqueue_and_echo_commands_P(PSTR("M82"));       // absolute E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));    // reset E axis
}

//! Check if the target temperature is reached and in this case, do step #2: extrude or unextrude
//! @param command      Command to extrude or unextrude
//! @param back_task    Background task to detect if the target temperature is reached and in this case, do step #2
void LoadUnload::start_task(const char* command, const BackgroundTask& back_task)
{
    if(Temperature::current_temperature[0] >= Temperature::target_temperature[0] - 10)
    {
        Log::log() << F("Load/Unload Filament") << Log::endl();
        advi3pp.buzz(); // Inform the user that the extrusion starts
        enqueue_and_echo_commands_P(command);
        task.set_background_task(back_task);
        advi3pp.set_status(F("Press Back when the filament comes out..."));
    }
}

//! Load the filament if the temperature is high enough.
void LoadUnload::load_start_task()
{
    start_task(PSTR("G1 E1 F120"), BackgroundTask(this, &LoadUnload::load_task));
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
    start_task(PSTR("G1 E-1 F120"), BackgroundTask(this, &LoadUnload::unload_task));
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
//! @return             True if the action was handled
bool Preheat::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::PresetPrevious:  previous_command(); break;
        case KeyValue::PresetNext:      next_command(); break;
        case KeyValue::Cooldown:        cooldown_command(); break;
        default:                        return false;
    }

    return true;
}

//! Store presets in permanent memory (EEPROM)
//! @param eeprom EEPROM writer
void Preheat::do_write(EepromWrite& eeprom) const
{
    for(auto& preset: presets_)
    {
        eeprom.write(preset.hotend);
        eeprom.write(preset.bed);
    }
}

//! Restore presets from permanent memory (EEPROM).
//! @param eeprom EEPROM reader
void Preheat::do_read(EepromRead& eeprom)
{
    for(auto& preset: presets_)
    {
        eeprom.read(preset.hotend);
        eeprom.read(preset.bed);
    }
}

//! Reset presets.
void Preheat::do_reset()
{
    for(size_t i = 0; i < NB_PRESETS; ++i)
    {
        presets_[i].hotend  = DEFAULT_PREHEAT_PRESET[i].hotend;
        presets_[i].bed     = DEFAULT_PREHEAT_PRESET[i].bed;
        presets_[i].fan     = DEFAULT_PREHEAT_PRESET[i].fan;
    }
}

//! Return the amount of data (in bytes) necessary to save settings in permanent memory (EEPROM).
//! @return Number of bytes
uint16_t Preheat::do_size_of() const
{
    return NB_PRESETS * (sizeof(Preset::hotend) + sizeof(Preset::bed));
}

//! Send the presets t the LCD Panel
void Preheat::send_presets()
{
    Log::log() << F("Preheat page") << Log::endl();
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(presets_[index_].hotend)
          << Uint16(presets_[index_].bed)
          << Uint16(presets_[index_].fan);
    frame.send();

    ADVString<8> preset;
    preset << index_ + 1 << F(" / ") << NB_PRESETS;
    frame.reset(Variable::ShortText0);
    frame << preset;
    frame.send();
}

//! Retrieve presets values from the LCD Panel
void Preheat::retrieve_presets()
{
    ReadRamData frame{Variable::Value0, 3};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Error receiving presets") << Log::endl();
        return;
    }

    Uint16 hotend, bed, fan;
    frame >> hotend >> bed >> fan;

    presets_[index_].hotend = hotend.word;
    presets_[index_].bed = bed.word;
    presets_[index_].fan = fan.word;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page Preheat::do_prepare_page()
{
    send_presets();
    return Page::Preheat;
}

//! Handle Previous command
void Preheat::previous_command()
{
    if(index_ <= 0)
        return;
    retrieve_presets();
    --index_;
    send_presets();
}

//! Handle Next command
void Preheat::next_command()
{
    if(index_ >= NB_PRESETS - 1)
        return;
    retrieve_presets();
    ++index_;
    send_presets();
}

//! Handles the Save (Continue) command
void Preheat::do_save_command()
{
    retrieve_presets();

    const Preset& preset = presets_[index_];

    ADVString<15> command;

    command = F("M104 S"); command << preset.hotend;
    enqueue_and_echo_command(command.get());

    command = F("M140 S"); command << preset.bed;
    enqueue_and_echo_command(command.get());

    command = F("M106 S"); command << scale(preset.fan, 100, 255);
    enqueue_and_echo_command(command.get());

    advi3pp.save_settings();
    temperatures.show(ShowOptions::None);
}

//! Cooldown the bed and the nozzle, turn off the fan
void Preheat::cooldown_command()
{
    Log::log() << F("Cooldown") << Log::endl();
    advi3pp.reset_status();
    Temperature::disable_all_heaters();
    enqueue_and_echo_commands_P(PSTR("M106 S0")); // Turn off fan
}


// --------------------------------------------------------------------
// Move & Home
// --------------------------------------------------------------------

//! Execute a move command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool Move::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::MoveXHome:           x_home_command(); break;
        case KeyValue::MoveYHome:           y_home_command(); break;
        case KeyValue::MoveZHome:           z_home_command(); break;
        case KeyValue::MoveAllHome:         all_home_command(); break;
        case KeyValue::DisableMotors:       disable_motors_command(); break;
        default:                            return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page Move::do_prepare_page()
{
    Planner::finish_and_disable(); // To circumvent homing problems
    return Page::Move;
}

//! Move the nozzle. Check that the command is not send too early when multiple move commands are send in a short time
//! (i.e. when the user keep the button presses)
//! @params command Actual command to move the nozzle.
void Move::move(const char* command, millis_t delay)
{
    if(!ELAPSED(millis(), last_move_time_ + delay))
        return;
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(command);
    enqueue_and_echo_commands_P(PSTR("G90"));
    last_move_time_ = millis();
}

//! Move the nozzle (+X)
void Move::x_plus_command()
{
    move(PSTR("G1 X4 F1000"), 150);
}

//! Move the nozzle (-X)
void Move::x_minus_command()
{
    move(PSTR("G1 X-4 F1000"), 150);
}

//! Move the nozzle (+Y)
void Move::y_plus_command()
{
    move(PSTR("G1 Y4 F1000"), 150);
}

//! Move the nozzle (-Y)
void Move::y_minus_command()
{
    move(PSTR("G1 Y-4 F1000"), 150);
}

//! Move the nozzle (+Z)
void Move::z_plus_command()
{
    move(PSTR("G1 Z0.5 F240"), 10);
}

//! Move the nozzle (-Z)
void Move::z_minus_command()
{
    move(PSTR("G1 Z-0.5 F240"), 10);
}

//! Extrude some filament.
void Move::e_plus_command()
{
    if(Temperature::degHotend(0) < 180)
        return;

    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 E1 F120"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Unextrude some filament.
void Move::e_minus_command()
{
    if(Temperature::degHotend(0) < 180)
        return;

    clear_command_queue();
    enqueue_and_echo_commands_P(PSTR("G91"));
    enqueue_and_echo_commands_P(PSTR("G1 E-1 F120"));
    enqueue_and_echo_commands_P(PSTR("G90"));
}

//! Disable the motors.
void Move::disable_motors_command()
{
    enqueue_and_echo_commands_P(PSTR("M84"));
    axis_homed = 0;
    axis_known_position = 0;
}

//! Go to home on the X axis.
void Move::x_home_command()
{
    enqueue_and_echo_commands_P(PSTR("G28 X F6000"));
}

//! Go to home on the Y axis.
void Move::y_home_command()
{
    enqueue_and_echo_commands_P(PSTR("G28 Y F6000"));
}

//! Go to home on the Z axis.
void Move::z_home_command()
{
    enqueue_and_echo_commands_P(PSTR("G28 Z F6000"));
}

//! Go to home on all axis.
void Move::all_home_command()
{
    enqueue_and_echo_commands_P(PSTR("G28 F6000"));
}

// --------------------------------------------------------------------
// Sensor Tuning
// --------------------------------------------------------------------

#ifdef ADVi3PP_PROBE

//! Execute a Sensor Tuning command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool SensorTuning::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::SensorSelfTest:  self_test_command(); break;
        case KeyValue::SensorReset:     reset_command(); break;
        case KeyValue::SensorDeploy:    deploy_command(); break;
        case KeyValue::SensorStow:      stow_command(); break;
        default:                        return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page SensorTuning::do_prepare_page()
{
    return Page::SensorTuning;
}

//! Execute the sensor Self-test command
void SensorTuning::self_test_command()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S120"));
}

//! Execute the sensor Reset command
void SensorTuning::reset_command()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S160"));
}

//! Execute the sensor Deploy command
void SensorTuning::deploy_command()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S10"));
}

//! Execute the sensor Stow command
void SensorTuning::stow_command()
{
    enqueue_and_echo_commands_P(PSTR("M280 P0 S90"));
}

#else

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page SensorTuning::do_prepare_page()
{
    return Page::NoSensor;
}

#endif

// --------------------------------------------------------------------
// Automatic Leveling
// --------------------------------------------------------------------

#ifdef ADVi3PP_PROBE

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page AutomaticLeveling::do_prepare_page()
{
    sensor_interactive_leveling_ = true;
    pages.save_forward_page();
    wait.show(F("Homing..."));
    enqueue_and_echo_commands_P(PSTR("G28 F6000"));             // homing
    enqueue_and_echo_commands_P(PSTR("G1 Z4 F1200"));           // raise head
    enqueue_and_echo_commands_P(PSTR("G29 E"));                 // leveling
    enqueue_and_echo_commands_P(PSTR("G28 X Y F6000"));         // go back to corner. Assumes RESTORE_LEVELING_AFTER_G28

    return Page::None;
}

//! Called by Marlin when G29 (automatic bed leveling) is finished.
//! @param success Boolean indicating if the leveling was successful or not.
void AutomaticLeveling::g29_leveling_finished(bool success)
{
    if(!success)
    {
        if(!sensor_interactive_leveling_)
            print.send_stop_usb_print(); // Send even during a SD print because you may monitor with OctoPrint

        if(sensor_interactive_leveling_)
            wait.show(F("Leveling failed"), WaitCallback{this, &AutomaticLeveling::g29_leveling_failed});
        else
            advi3pp.set_status(F("Leveling failed"));

        sensor_interactive_leveling_ = false;
        return;
    }

    advi3pp.reset_status();

    if(sensor_interactive_leveling_)
    {
        sensor_interactive_leveling_ = false;
        leveling_grid.show(ShowOptions::None);
    }
    else
    {
        enqueue_and_echo_commands_P(PSTR("M500"));      // Save settings (including mash)
        enqueue_and_echo_commands_P(PSTR("M420 S1"));   // Set bed leveling state (enable)
    }
}

//! Show the back page when G29 (automatic bed leveling) failed.
bool AutomaticLeveling::g29_leveling_failed()
{
    pages.show_back_page();
    return true;
}

#else

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page AutomaticLeveling::do_prepare_page()
{
    return Page::NoSensor;
}

#endif

// --------------------------------------------------------------------
// Leveling Grid
// --------------------------------------------------------------------

#ifdef ADVi3PP_PROBE

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page LevelingGrid::do_prepare_page()
{
    WriteRamDataRequest frame{Variable::Value0};
    for(auto y = 0; y < GRID_MAX_POINTS_Y; y++)
        for(auto x = 0; x < GRID_MAX_POINTS_X; x++)
            frame << Uint16(static_cast<int16_t>(z_values[x][y] * 100));
    frame.send();

    return Page::SensorGrid;
}

//! Handles the Save (Continue) command
void LevelingGrid::do_save_command()
{
    enqueue_and_echo_commands_P(PSTR("M500"));      // Save settings (including mash)
    enqueue_and_echo_commands_P(PSTR("M420 S1"));   // Set bed leveling state (enable)
    Parent::do_save_command();
}

#else

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page LevelingGrid::do_prepare_page()
{
    return Page::NoSensor;
}

#endif

// --------------------------------------------------------------------
// Manual Leveling
// --------------------------------------------------------------------

//! Execute a Manual Leveling command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool ManualLeveling::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::LevelingPoint1:  point1_command(); break;
        case KeyValue::LevelingPoint2:  point2_command(); break;
        case KeyValue::LevelingPoint3:  point3_command(); break;
        case KeyValue::LevelingPoint4:  point4_command(); break;
        case KeyValue::LevelingPoint5:  point5_command(); break;
        case KeyValue::LevelingPointA:  pointA_command(); break;
        case KeyValue::LevelingPointB:  pointB_command(); break;
        case KeyValue::LevelingPointC:  pointC_command(); break;
        case KeyValue::LevelingPointD:  pointD_command(); break;
        default:                        return false;
    }

    return true;
}

//! Execute the Back command
void ManualLeveling::do_back_command()
{
    enqueue_and_echo_commands_P(PSTR("G1 Z30 F1200"));
    Parent::do_back_command();
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page ManualLeveling::do_prepare_page()
{
    wait.show(F("Homing..."));
    ::axis_homed = 0;
    ::axis_known_position = 0;
    enqueue_and_echo_commands_P(PSTR("G90")); // absolute mode
    enqueue_and_echo_commands_P((PSTR("G28 F6000"))); // homing
    task.set_background_task(BackgroundTask(this, &ManualLeveling::leveling_task), 200);
    return Page::None;
}

//! Leveling Background task.
void ManualLeveling::leveling_task()
{
    if(!TEST(axis_homed, X_AXIS) || !TEST(axis_homed, Y_AXIS) || !TEST(axis_homed, Z_AXIS))
        return;

    Log::log() << F("Leveling Homed, start process") << Log::endl();
    advi3pp.reset_status();
    task.clear_background_task();
    pages.show_page(Page::ManualLeveling, ShowOptions::None);
}

//! Handle leveling point #1.
void ManualLeveling::point1_command()
{
    Log::log() << F("Level point 1") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z4 F1200"));
    enqueue_and_echo_commands_P(PSTR("G1 X30 Y30 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1200"));
}

//! Handle leveling point #2.
void ManualLeveling::point2_command()
{
    Log::log() << F("Level point 2") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z4 F1200"));
    enqueue_and_echo_commands_P(PSTR("G1 X30 Y170 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1200"));
}

//! Handle leveling point #3.
void ManualLeveling::point3_command()
{
    Log::log() << F("Level point 3") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z4 F1200"));
    enqueue_and_echo_commands_P(PSTR("G1 X170 Y170 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1200"));
}

//! Handle leveling point #4.
void ManualLeveling::point4_command()
{
    Log::log() << F("Level point 4") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z4 F1200"));
    enqueue_and_echo_commands_P(PSTR("G1 X170 Y30 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1200"));
}

//! Handle leveling point #5.
void ManualLeveling::point5_command()
{
    Log::log() << F("Level point 5") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z4 F1200"));
    enqueue_and_echo_commands_P(PSTR("G1 X100 Y100 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1200"));
}

//! Handle leveling point #A.
void ManualLeveling::pointA_command()
{
    Log::log() << F("Level point A") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z4 F1200"));
    enqueue_and_echo_commands_P(PSTR("G1 X100 Y30 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1200"));
}

//! Handle leveling point #B.
void ManualLeveling::pointB_command()
{
    Log::log() << F("Level point B") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z4 F1200"));
    enqueue_and_echo_commands_P(PSTR("G1 X30 Y100 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1200"));
}

//! Handle leveling point #C.
void ManualLeveling::pointC_command()
{
    Log::log() << F("Level point C") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z4 F1200"));
    enqueue_and_echo_commands_P(PSTR("G1 X100 Y170 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1200"));
}

//! Handle leveling point #D.
void ManualLeveling::pointD_command()
{
    Log::log() << F("Level point D") << Log::endl();
    enqueue_and_echo_commands_P(PSTR("G1 Z4 F1200"));
    enqueue_and_echo_commands_P(PSTR("G1 X170 Y100 F6000"));
    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1200"));
}


// --------------------------------------------------------------------
// SD Card
// --------------------------------------------------------------------

//! Execute a SD Card command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool SdCard::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

	switch(key_value)
	{
		case KeyValue::SDUp:	up_command(); break;
		case KeyValue::SDDown:	down_command(); break;
		case KeyValue::SDLine1:
		case KeyValue::SDLine2:
		case KeyValue::SDLine3:
		case KeyValue::SDLine4:
		case KeyValue::SDLine5:	select_file_command(static_cast<uint16_t>(key_value) - 1); break;
		default:                return false;
	}

	return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page SdCard::do_prepare_page()
{
    show_first_page();
    return Page::SdCard;
}

//! Show first SD card page
void SdCard::show_first_page()
{
	if(!card.cardOK)
		return;

    page_index_ = 0;
	nb_files_ = card.getnrfilenames();
	last_file_index_ = nb_files_ > 0 ? nb_files_ - 1 : 0;

    show_current_page();
}

//! Handle Page Down command.
void SdCard::down_command()
{
	if(!card.cardOK)
		return;

	if(last_file_index_ >= nb_visible_sd_files)
    {
        page_index_ += 1;
		last_file_index_ -= nb_visible_sd_files;
        show_current_page();
    }
}

//! Handle Page Up command.
void SdCard::up_command()
{
	if(!card.cardOK)
		return;

	if(last_file_index_ + nb_visible_sd_files < nb_files_)
    {
        page_index_ -= 1;
		last_file_index_ += nb_visible_sd_files;
        show_current_page();
    }
}

//! Show the list of files on SD (current page)
void SdCard::show_current_page()
{
    WriteRamDataRequest frame{Variable::LongText0};

    ADVString<sd_file_length> name;
    ADVString<48> aligned_name;

    for(uint8_t index = 0; index < nb_visible_sd_files; ++index)
    {
        get_file_name(index, name);
        aligned_name.set(name, Alignment::Left);
        frame << aligned_name;
    }
    frame.send(true);

    frame.reset(Variable::Value0);
    frame << Uint16(page_index_ + 1);
    frame.send();
}

//! Get a filename with a given index.
//! @param index    Index of the filename
//! @param name     Copy the filename into this Chars
void SdCard::get_file_name(uint8_t index_in_page, ADVString<sd_file_length>& name)
{
    name.reset();
	if(last_file_index_ >= index_in_page)
	{
		card.getfilename(last_file_index_ - index_in_page);
        if(card.filenameIsDir) name = "[";
		name += (card.longFilename[0] == 0) ? card.filename : card.longFilename;
		if(card.filenameIsDir) name += "]";
	}
}

//! Select a filename as sent by the LCD screen.
//! @param file_index    The index of the filename to select
void SdCard::select_file_command(uint16_t file_index)
{
    if(!card.cardOK)
        return;

    if(file_index > last_file_index_)
        return;

    card.getfilename(last_file_index_ - file_index);
    if(card.filenameIsDir)
        return;

    const char* filename = (card.longFilename[0] == 0) ? card.filename : card.longFilename;
    if(filename == nullptr) // If the SD card is not readable
        return;

    advi3pp.set_progress_name(filename);

    card.openFile(card.filename, true); // use always short filename so it will work even if the filename is long
    card.startFileprint();
    PrintCounter::start();

    pages.show_page(Page::Print);
}

// --------------------------------------------------------------------
// Printing
// --------------------------------------------------------------------

//! Handle print commands.
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool Print::do_dispatch(KeyValue value)
{
    if(Parent::do_dispatch(value))
        return true;

    switch(value)
    {
        case KeyValue::PrintStop:           stop_command(); break;
        case KeyValue::PrintPause:          pause_command(); break;
        case KeyValue::PrintAdvancedPause:  advanced_pause_command(); break;
        default:                            return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page Print::do_prepare_page()
{
    return Page::Print;
}

//! Stop printing
void Print::stop_command()
{
    enqueue_and_echo_commands_P(PSTR("A1"));
}

//! Pause printing
void Print::pause_command()
{
    enqueue_and_echo_commands_P(PSTR("A0"));
}

//! Advanced Pause for filament change
void Print::advanced_pause_command()
{
    enqueue_and_echo_commands_P(PSTR("M600"));
}

//! Process Stop (A1) code and actually stop the print (if any running).
void Print::process_stop_code()
{
    if(!is_printing())
        return;

    wait.show(F("Stop printing..."), ShowOptions::SaveBack);
    if(is_usb_printing())
        send_stop_usb_print();

    pause_print(PAUSE_PARK_RETRACT_LENGTH, NOZZLE_PARK_POINT, 0, true);

    thermalManager.disable_all_heaters();
    fanSpeeds[0] = 0;

    planner.quick_stop();
    print_job_timer.stop();

    advi3pp.set_status(F("Print Stopped"));
    pages.show_back_page();
}

//! Process Pause (A0) code and actually pause the print (if any running).
void Print::process_pause_code()
{
    if(!is_printing())
        return;

    wait.show(F("Pause printing..."), ShowOptions::SaveBack);
    if (!pause_print(PAUSE_PARK_RETRACT_LENGTH, NOZZLE_PARK_POINT, 0, true))
    {
        pause_finished(false);
        return;
    }

    wait.show_continue(F("Press Continue to resume printing..."), ShowOptions::None);

    bool nozzle_timed_out = false;
    const millis_t nozzle_timeout = 1000UL * static_cast<millis_t>(PAUSE_PARK_NOZZLE_TIMEOUT);
    HOTEND_LOOP() thermalManager.start_heater_idle_timer(e, nozzle_timeout);
    KEEPALIVE_STATE(PAUSED_FOR_USER);

    wait_for_user = true;    // LCD click or M108 will clear this
    while(wait_for_user)
    {
        // If the nozzle has timed out, wait for the user to press the button to re-heat the nozzle, then
        // re-heat the nozzle, restart the idle timers, and start over
        if (!nozzle_timed_out)
            HOTEND_LOOP() nozzle_timed_out |= thermalManager.is_heater_idle(e);

        idle(true);
    }

    wait.show(F("Resuming..."), ShowOptions::None);

    KEEPALIVE_STATE(IN_HANDLER);

    // Re-enable the heaters if they timed out
    HOTEND_LOOP() thermalManager.reset_heater_idle_timer(e);

    // Wait for the heaters to reach the target temperatures
    if(!ensure_safe_temperature())
        return;

    resume_print(0, 0, ADVANCED_PAUSE_PURGE_LENGTH, 0);
    print_job_timer.start();

    pause_finished(true);
}

//! Pause command done, show the back page
void Print::pause_finished(bool)
{
    pages.show_back_page();
}

//! Check if there is currently a print running (USB or SD)
//! @return True if a print is running.
bool Print::is_printing() const
{
    if(card.isFileOpen())
        return card.sdprinting;
    else
        return print_job_timer.isRunning();
}

//! Check if there is currently a print running (USB)
//! @return True if a USB print is running.
bool Print::is_usb_printing() const
{
    return print_job_timer.isRunning();
}

//! Send Stop print to the host.
//! Unfortunately, there no way to properly do this except disconnecting the host.
void Print::send_stop_usb_print()
{
    // "disconnect" is the only standard command to stop an USB print
    SERIAL_ECHOLNPGM("//action:disconnect");
}

// --------------------------------------------------------------------
// Advance pause
// --------------------------------------------------------------------

//! Show Advance Pause message (called from Marlin).
//! @param message Message to dislay.
void ADVi3pp_::advanced_pause_show_message(const AdvancedPauseMessage message)
{
    pause.advanced_pause_show_message(message);
}

//! Show Advance Pause message (called from Marlin).
//! @param message Message to dislay.
void AdvancedPause::advanced_pause_show_message(const AdvancedPauseMessage message)
{
    if(message == last_advanced_pause_message_)
        return;
    last_advanced_pause_message_ = message;

    switch (message)
    {
        case ADVANCED_PAUSE_MESSAGE_INIT:                       wait.show(F("Pausing...")); break;
        case ADVANCED_PAUSE_MESSAGE_UNLOAD:                     advi3pp.set_status(F("Unloading filament...")); break;
        case ADVANCED_PAUSE_MESSAGE_INSERT:                     insert_filament(); break;
        case ADVANCED_PAUSE_MESSAGE_LOAD:                       advi3pp.set_status(F("Loading...")); break;
        case ADVANCED_PAUSE_MESSAGE_PURGE:                      advi3pp.set_status(F("Extruding some filament...")); break;
        case ADVANCED_PAUSE_MESSAGE_CLICK_TO_HEAT_NOZZLE:       advi3pp.set_status(F("Press continue to heat")); break;
        case ADVANCED_PAUSE_MESSAGE_RESUME:                     advi3pp.set_status(F("Resuming print...")); break;
        case ADVANCED_PAUSE_MESSAGE_STATUS:                     break;
        case ADVANCED_PAUSE_MESSAGE_WAIT_FOR_NOZZLES_TO_HEAT:   advi3pp.set_status(F("Waiting for heat...")); break;
        case ADVANCED_PAUSE_MESSAGE_OPTION:                     advanced_pause_menu_response = ADVANCED_PAUSE_RESPONSE_RESUME_PRINT; break;
        default: advi3pp::Log::log() << F("Unknown AdvancedPauseMessage: ") << static_cast<uint16_t>(message) << advi3pp::Log::endl(); break;
    }
}

//! Show "Insert filament" message during Advance Pause
void AdvancedPause::insert_filament()
{
    wait.show_continue(F("Insert filament and press continue..."), WaitCallback{this, &AdvancedPause::filament_inserted}, ShowOptions::None);
}

//! Action to execute once the filament is inserted (Continue command)
bool AdvancedPause::filament_inserted()
{
    ::wait_for_user = false;
    wait.show(F("Filament inserted.."), ShowOptions::None);
    return false;
}

// --------------------------------------------------------------------
// Sensor Z-Height
// --------------------------------------------------------------------

#ifdef ADVi3PP_PROBE

//! Handle Sensor Z Height command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool SensorZHeight::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::Multiplier1:     multiplier1_command(); break;
        case KeyValue::Multiplier2:     multiplier2_command(); break;
        case KeyValue::Multiplier3:     multiplier3_command(); break;
        default:                        return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page SensorZHeight::do_prepare_page()
{
    pages.save_forward_page();

    zprobe_zoffset = 0;  // reset offset
    wait.show(F("Homing..."));
    enqueue_and_echo_commands_P((PSTR("G28 F6000")));  // homing
    task.set_background_task(BackgroundTask(this, &SensorZHeight::post_home_task), 200);
    return Page::None;
}

//! Reset Sensor Z Height data.
void SensorZHeight::reset()
{
    multiplier_ = Multiplier::M1;
}

//! Check if the printer is homed, and continue the Z Height Tuning process.
void SensorZHeight::post_home_task()
{
    if(!TEST(axis_homed, X_AXIS) || !TEST(axis_homed, Y_AXIS) || !TEST(axis_homed, Z_AXIS))
        return;
    if(advi3pp.is_busy())
        return;

    task.clear_background_task();
    advi3pp.reset_status();

    reset();

    enqueue_and_echo_commands_P(PSTR("G1 Z0 F1200"));  // raise head
    enqueue_and_echo_commands_P(PSTR("G1 X100 Y100 F6000")); // middle
    enqueue_and_echo_commands_P(PSTR("M211 S0")); // disable soft-endstops
    send_data();

    pages.show_page(Page::ZHeightTuning, ShowOptions::None);
}

//! Execute the Back command
void SensorZHeight::do_back_command()
{
    enqueue_and_echo_commands_P(PSTR("M211 S1")); // enable enstops
    enqueue_and_echo_commands_P(PSTR("G28 Z F1200"));  // since we have reset the offset, Z-home
    enqueue_and_echo_commands_P(PSTR("G28 X Y F6000")); // XY-homing
    Parent::do_back_command();
}

//! Handles the Save (Continue) command
void SensorZHeight::do_save_command()
{
    zprobe_zoffset = advi3pp.get_current_z_height();
    enqueue_and_echo_commands_P(PSTR("M211 S1")); // enable enstops
    enqueue_and_echo_commands_P(PSTR("G1 Z4 F1200"));  // raise head
    enqueue_and_echo_commands_P(PSTR("G28 X Y F6000")); // homing
    Parent::do_save_command();
}

//! Change the multiplier.
void SensorZHeight::multiplier1_command()
{
    multiplier_ = Multiplier::M1;
    send_data();
}

//! Change the multiplier.
void SensorZHeight::multiplier2_command()
{
    multiplier_ = Multiplier::M2;
    send_data();
}

//! Change the multiplier.
void SensorZHeight::multiplier3_command()
{
    multiplier_ = Multiplier::M3;
    send_data();
}

//! Change the position of the nozzle (-Z).
void SensorZHeight::minus()
{
    adjust_height(-get_multiplier_value());
}

//! Change the position of the nozzle (+Z).
void SensorZHeight::plus()
{
    adjust_height(+get_multiplier_value());
}

//! Get the current multiplier value on the LCD panel.
double SensorZHeight::get_multiplier_value() const
{
    if(multiplier_ < Multiplier::M1 || multiplier_ > Multiplier::M3)
    {
        Log::error() << F("Invalid multiplier value: ") << static_cast<uint16_t >(multiplier_) << Log::endl();
        return PRINT_SETTINGS_MULTIPLIERS[0];
    }

    return SENSOR_Z_HEIGHT_MULTIPLIERS[static_cast<uint16_t>(multiplier_)];
}

//! Adjust the Z height.
//! @param offset Offset for the adjustment.
void SensorZHeight::adjust_height(double offset)
{
	auto new_height = advi3pp.get_current_z_height() + offset;
    ADVString<10> command;
    command << F("G1 Z") << new_height << F(" F1200");
    enqueue_and_echo_command(command.get());
    send_data();
}

//! Send the current data (i.e. multiplier) to the LCD panel.
void SensorZHeight::send_data() const
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(static_cast<uint16_t>(multiplier_));
    frame.send();
}

#else

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page SensorZHeight::do_prepare_page()
{
    return Page::NoSensor;
}

#endif

// --------------------------------------------------------------------
// Extruder tuning
// --------------------------------------------------------------------

//! Handle Extruder Tuning command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool ExtruderTuning::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::TuningStart:     start_command(); break;
        case KeyValue::TuningSettings:  settings_command(); break;
        default:                        return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page ExtruderTuning::do_prepare_page()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(advi3pp.get_last_used_temperature(TemperatureKind::Hotend));
    frame.send();
    return Page::ExtruderTuningTemp;
}

//! Start extruder tuning.
void ExtruderTuning::start_command()
{
    ReadRamData frame{Variable::Value0, 1};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Target Temperature)") << Log::endl();
        return;
    }

    Uint16 hotend; frame >> hotend;
    wait.show(F("Heating the extruder..."), WaitCallback{this, &ExtruderTuning::cancel}, ShowOptions::None);
    Temperature::setTargetHotend(hotend.word, 0);

    task.set_background_task(BackgroundTask(this, &ExtruderTuning::heating_task));
}

//! Extruder tuning background task.
void ExtruderTuning::heating_task()
{
    if(Temperature::current_temperature[0] < Temperature::target_temperature[0] - 10)
        return;
    task.clear_background_task();

    advi3pp.set_status(F("Wait until the extrusion is finished..."));
    enqueue_and_echo_commands_P(PSTR("G1 Z20 F1200"));   // raise head
    enqueue_and_echo_commands_P(PSTR("M83"));           // relative E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));        // reset E axis

    ADVString<20> command; command << F("G1 E") << tuning_extruder_filament << " F50"; // Extrude slowly
    enqueue_and_echo_command(command.get());

    task.set_background_task(BackgroundTask(this, &ExtruderTuning::extruding_task));
}

//! Extruder tuning background task.
void ExtruderTuning::extruding_task()
{
    if(current_position[E_AXIS] < tuning_extruder_filament || advi3pp.is_busy())
        return;
    task.clear_background_task();

    extruded_ = current_position[E_AXIS];

    Temperature::setTargetHotend(0, 0);
    task.clear_background_task();
    advi3pp.reset_status();
    finished();
}

//! Record the amount of filament extruded.
void ExtruderTuning::finished()
{
    Log::log() << F("Filament extruded ") << extruded_ << Log::endl();
    enqueue_and_echo_commands_P(PSTR("M82"));       // absolute E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));    // reset E axis

    task.clear_background_task();

    // Always set ny default 20mm
    WriteRamDataRequest frame{Variable::Value0};
    frame << 200_u16; // 20.0
    frame.send();

    pages.show_page(Page::ExtruderTuningMeasure, ShowOptions::None);
}

//! Cancel Extruder tuning process.
bool ExtruderTuning::cancel()
{
    ::wait_for_user = ::wait_for_heatup = false;
    task.clear_background_task();
    Temperature::setTargetHotend(0, 0);
    return false;
}

//! Execute the Back command
void ExtruderTuning::do_back_command()
{
    task.clear_background_task();

    Temperature::setTargetHotend(0, 0);

    enqueue_and_echo_commands_P(PSTR("M82"));       // absolute E mode
    enqueue_and_echo_commands_P(PSTR("G92 E0"));    // reset E axis

    Parent::do_back_command();
}

//! Compute the extruder (E axis) new value and show the steps settings.
void ExtruderTuning::settings_command()
{
    ReadRamData frame{Variable::Value0, 1};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Measures)") << Log::endl();
        return;
    }

    Uint16 e; frame >> e;
    auto new_value = Planner::axis_steps_per_mm[E_AXIS] * extruded_ / (extruded_ + tuning_extruder_delta - e.word / 10.0);

    Log::log()
            << F("Adjust: old = ") << Planner::axis_steps_per_mm[E_AXIS]
            << F(", expected = ") << extruded_
            << F(", measured = ") << (extruded_ + tuning_extruder_delta - e.word)
            << F(", new = ") << new_value << Log::endl();

    Planner::axis_steps_per_mm[E_AXIS] = new_value;
    steps_settings.show(ShowOptions::None);
}

// --------------------------------------------------------------------
// PID Tuning
// --------------------------------------------------------------------

//! Handle PID tuning command
//! @param key_value    The step of the PID tuning
//! @return             True if the action was handled
bool PidTuning::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::PidTuningStep2:  step2_command(); break;
        case KeyValue::PidTuningHotend: hotend_command(); break;
        case KeyValue::PidTuningBed:    bed_command(); break;
        default:                        return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page PidTuning::do_prepare_page()
{
    pages.save_forward_page();
    hotend_command();
    advi3pp.reset_status();
    return Page::PidTuning;
}

//! Send the current data to the LCD panel.
void PidTuning::send_data()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(temperature_)
          << Uint16(kind_ != TemperatureKind::Hotend);
    frame.send();
}

//! Select the hotend PID
void PidTuning::hotend_command()
{
    temperature_ = advi3pp.get_last_used_temperature(TemperatureKind::Hotend);
    kind_ = TemperatureKind::Hotend;
    send_data();
}

//! Select the bed PID
void PidTuning::bed_command()
{
    temperature_ = advi3pp.get_last_used_temperature(TemperatureKind::Bed);
    kind_ = TemperatureKind::Bed;
    send_data();
}

//! Show step #2 of PID tuning
void PidTuning::step2_command()
{
    advi3pp.reset_status();

    ReadRamData frame{Variable::Value0, 2};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Target Temperature)") << Log::endl();
        return;
    }

    Uint16 temperature; frame >> temperature; temperature_ = temperature.word;

    if(kind_ == TemperatureKind::Hotend)
        enqueue_and_echo_commands_P(PSTR("M106 S255")); // Turn on fan (only for hotend)

    ADVString<20> auto_pid_command;
    auto_pid_command << F("M303 S") << temperature_
                     << (kind_ == TemperatureKind::Hotend ? F(" E0 U1") : F(" E-1 U1"));
    enqueue_and_echo_command(auto_pid_command.get());

    inTuning_ = true;
    temperatures.show(WaitCallback{this, &PidTuning::cancel_pid});
}

//! Cancel PID process.
bool PidTuning::cancel_pid()
{
    ::wait_for_user = ::wait_for_heatup = false;
    inTuning_ = false;
    return false;
}

//! PID automatic tuning is finished.
void PidTuning::finished(bool success)
{
    Log::log() << F("Auto PID finished: ") << (success ? F("success") : F("failed")) << Log::endl();
    enqueue_and_echo_commands_P(PSTR("M106 S0"));
    if(!success)
    {
        advi3pp.set_status(F("PID tuning failed"));
        return;
    }

    advi3pp.reset_status();
    pid_settings.add_pid(kind_, temperature_);
    bool inTuning = inTuning_;
    inTuning_ = false;
    pid_settings.show(inTuning ? ShowOptions::None : ShowOptions::SaveBack);
}

// --------------------------------------------------------------------
// Linear Advance Tuning
// --------------------------------------------------------------------

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page LinearAdvanceTuning::do_prepare_page()
{
    return Page::LinearAdvanceTuning;
}


// --------------------------------------------------------------------
// Diagnosis
// --------------------------------------------------------------------

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page Diagnosis::do_prepare_page()
{
    task.set_background_task(BackgroundTask{this, &Diagnosis::send_data}, 250);
    return Page::Diagnosis;
}

//! Execute the Back command
void Diagnosis::do_back_command()
{
    task.clear_background_task();
    Parent::do_back_command();
}

//! Get current digital pin state (adapted from Arduino source code).
//! @param pin  Pin number to check.
//! @return     The current state: On (input), Off (input), Output
Diagnosis::State Diagnosis::get_pin_state(uint8_t pin)
{
    uint8_t mask = digitalPinToBitMask(pin);
    uint8_t port = digitalPinToPort(pin);
    if(port == NOT_A_PIN)
        return State::Off;

    volatile uint8_t* reg = portModeRegister(port);
    if(*reg & mask)
        return State::Output;

    uint8_t timer = digitalPinToTimer(pin);
    if(timer != NOT_ON_TIMER)
        return State::Output;

    return (*portInputRegister(port) & mask) ? State::On : State::Off;
}

//! Send the current data to the LCD panel.
void Diagnosis::send_data()
{
    WriteRamDataRequest request{Variable::Value0};

    for(size_t i = 0; i < adv::count_of(diagnosis_digital_pins); ++i)
    {
        request.reset(static_cast<Variable>(static_cast<uint16_t>(Variable::Value0) + i));
        request << Uint16{static_cast<uint16_t>(get_pin_state(diagnosis_digital_pins[i]))};
        request.send(false);
    }

    for(size_t i = 0; i < adv::count_of(diagnosis_analog_pins); ++i)
    {
        request.reset(static_cast<Variable>(static_cast<uint16_t>(Variable::Value0) + 0x20 + i));
        request << Uint16{static_cast<uint16_t>(analogRead(diagnosis_analog_pins[i]))};
        request.send(false);
    }
}

// --------------------------------------------------------------------
// Sensor Settings
// --------------------------------------------------------------------

#ifdef ADVi3PP_PROBE

//! Constructor
SensorSettings::SensorSettings()
{
    do_reset();
}

//! Handle Sensor Settings command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool SensorSettings::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::SensorSettingsPrevious:  previous_command(); break;
        case KeyValue::SensorSettingsNext:      next_command(); break;
        default:                                return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page SensorSettings::do_prepare_page()
{
    send_data();
    pages.save_forward_page();
    return Page::SensorSettings;
}

//! Handles the Save (Continue) command
void SensorSettings::do_save_command()
{
    get_data();
    Parent::do_save_command();
}

//! Store current data in permanent memory (EEPROM)
//! @param eeprom EEPROM writer
void SensorSettings::do_write(EepromWrite& eeprom) const
{
    eeprom.write(index_);
    for(size_t i = 0; i < NB_SENSOR_POSITIONS; ++i)
    {
        eeprom.write(positions_[i].x);
        eeprom.write(positions_[i].y);
    }
}

//! Restore data from permanent memory (EEPROM).
//! @param eeprom EEPROM reader
void SensorSettings::do_read(EepromRead& eeprom)
{
    eeprom.read(index_);
    for(size_t i = 0; i < NB_SENSOR_POSITIONS; ++i)
    {
        eeprom.read(positions_[i].x);
        eeprom.read(positions_[i].y);
    }
}

//! Reset settings
void SensorSettings::do_reset()
{
    index_ = 0;
    for(size_t i = 0; i < NB_SENSOR_POSITIONS; ++i)
        positions_[i] = DEFAULT_SENSOR_POSITION[i];
}

//! Return the amount of data (in bytes) necessary to save settings in permanent memory (EEPROM).
//! @return Number of bytes
uint16_t SensorSettings::do_size_of() const
{
    return sizeof(index_) + NB_SENSOR_POSITIONS * sizeof(SensorPosition);
}

//! Show the previous settings.
void SensorSettings::previous_command()
{
    if(index_ <= 0)
        return;
    get_data();
    index_ -= 1;
    send_data();
}

//! Show the next settings.
void SensorSettings::next_command()
{
    if(index_ >= NB_SENSOR_POSITIONS - 1)
        return;
    get_data();
    index_ += 1;
    send_data();
}

//! Send current data to the LCD Panel.
void SensorSettings::send_data() const
{
    ADVString<32> title{get_sensor_name(index_)};

    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(positions_[index_].x) << Uint16(positions_[index_].y) << Uint16(zprobe_zoffset * 100);
    frame.send();

    frame.reset(Variable::LongTextCentered0);
    frame << title.align(Alignment::Center);
    frame.send();
}

//! Send current data from the LCD Panel.
void SensorSettings::get_data()
{
    ReadRamData frame{Variable::Value0, 3};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Sensor Settings)") << Log::endl();
        return;
    }

    Uint16 x, y, z;
    frame >> x >> y >> z;

    positions_[index_].x = static_cast<int16_t>(x.word);
    positions_[index_].y = static_cast<int16_t>(y.word);
    zprobe_zoffset = static_cast<int16_t>(z.word) / 100.0;
}

//! Get the current offset of the nozzle (depending of the sensor holder).
//! @return The offset between the nozzle and the sensor.
int SensorSettings::x_probe_offset_from_extruder() const
{
    return static_cast<int>(positions_[index_].x / 100.0 + 0.5); // 0.5 for rounding
}

//! Get the current offset of the nozzle (depending of the sensor holder).
//! @return The offset between the nozzle and the sensor.
int SensorSettings::y_probe_offset_from_extruder() const
{
    return static_cast<int>(positions_[index_].y / 100.0 + 0.5); // 0.5 for rounding
}

//! Get the position of the bed (depending of the sensor holder).
//! @return The position reachable (left).
int SensorSettings::left_probe_bed_position()
{
    return max(X_MIN_BED + (MIN_PROBE_EDGE), X_MIN_POS + x_probe_offset_from_extruder());
}

//! Get the position of the bed (depending of the sensor holder).
//! @return The position reachable (right).
int SensorSettings::right_probe_bed_position()
{
    return min(X_MAX_BED - (MIN_PROBE_EDGE), X_MAX_POS + x_probe_offset_from_extruder());
}

//! Get the position of the bed (depending of the sensor holder).
//! @return The position reachable (front).
int SensorSettings::front_probe_bed_position()
{
    return max(Y_MIN_BED + (MIN_PROBE_EDGE), Y_MIN_POS + y_probe_offset_from_extruder());
}

//! Get the position of the bed (depending of the sensor holder).
//! @return The position reachable (bottom).
int SensorSettings::back_probe_bed_position()
{
    return min(Y_MAX_BED - (MIN_PROBE_EDGE), Y_MAX_POS + y_probe_offset_from_extruder());
}

#else

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page SensorSettings::do_prepare_page()
{
    return Page::NoSensor;
}

#endif



// --------------------------------------------------------------------
// Firmware Settings
// --------------------------------------------------------------------

//! Handle Firmware Settings command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool FirmwareSettings::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::ThermalProtection:   thermal_protection_command(); break;
        case KeyValue::RunoutSensor:        runout_sensor_command(); break;
        case KeyValue::USBBaudrateMinus:    baudrate_minus_command(); break;
        case KeyValue::USBBaudratePlus:     baudrate_plus_command(); break;
        default:                            return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page FirmwareSettings::do_prepare_page()
{
    usb_baudrate_ = advi3pp.get_current_baudrate();
    features_ = advi3pp.get_current_features();
    send_usb_baudrate();
    send_features();
    return Page::Firmware;
}

//! Toggle the Thermal Runaway Protection feature.
void FirmwareSettings::thermal_protection_command()
{
    flip_bits(features_, Feature::ThermalProtection);
    send_features();
}

//! Toggle the Runout Sensor feature (currently not fully implemented)
void FirmwareSettings::runout_sensor_command()
{
    flip_bits(features_, Feature::RunoutSensor);
    send_features();
}

//! Handles the Save (Continue) command
void FirmwareSettings::do_save_command()
{
    if(advi3pp.get_current_baudrate() != usb_baudrate_)
        advi3pp.change_usb_baudrate(usb_baudrate_, true);
    advi3pp.change_features(features_);
    Parent::do_save_command();
}

//! Send the current selected features to the LCD Panel.
void FirmwareSettings::send_features() const
{
    WriteRamDataRequest frame{Variable::Value0}; frame << Uint16(static_cast<uint16_t>(features_)); frame.send();
}

//! Send the current selected baudrate to the LCD Panel.
void FirmwareSettings::send_usb_baudrate() const
{
    ADVString<6> value; value << usb_baudrate_;

    WriteRamDataRequest frame{Variable::ShortText0};
    frame << value;
    frame.send();
}

//! Get the baudrate index from its value.
//! @param baudrate Baudrate
//! @return The index corresponding to the baudrate, or 0 if none.
size_t FirmwareSettings::usb_baudrate_index(uint32_t baudrate)
{
    size_t nb = adv::count_of(usb_baudrates);
    for(size_t i = 0; i < nb; ++i)
        if(baudrate == usb_baudrates[i])
            return i;
    return 0;
}

//! Handle the -Baudrate command.
void FirmwareSettings::baudrate_minus_command()
{
    auto index = usb_baudrate_index(usb_baudrate_);
    usb_baudrate_ = index > 0 ? usb_baudrates[index - 1] : usb_baudrates[0];
    send_usb_baudrate();
}

//! Handle the +Baudrate command.
void FirmwareSettings::baudrate_plus_command()
{
    auto index = usb_baudrate_index(usb_baudrate_);
    static const auto max = adv::count_of(usb_baudrates) - 1;
    usb_baudrate_ = index < max ? usb_baudrates[index + 1] : usb_baudrates[max];
    send_usb_baudrate();
}

// --------------------------------------------------------------------
// LCD Settings
// --------------------------------------------------------------------

//! Handle LCD Settings command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool LcdSettings::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::LCDDimming:          dimming_command(); break;
        case KeyValue::BuzzerOnAction:      buzz_on_action_command(); break;
        case KeyValue::BuzzOnPress:         buzz_on_press_command(); break;
        default:                            return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page LcdSettings::do_prepare_page()
{
    features_ = advi3pp.get_current_features();
    send_data();
    return Page::LCD;
}

//! Handle the Dimming (On/Off) command
void LcdSettings::dimming_command()
{
    flip_bits(features_, Feature::Dimming);
    dimming.enable(test_one_bit(features_, Feature::Dimming));
    send_data();
    advi3pp.change_features(features_);
    advi3pp.save_settings();
}

//! Handle the change brightness command.
void LcdSettings::change_brightness(uint16_t brightness)
{
    dimming.change_brightness(brightness);
    send_data();
    advi3pp.save_settings();
}

//! Handle the Buzz on Action command
void LcdSettings::buzz_on_action_command()
{
    flip_bits(features_, Feature::Buzzer);
    advi3pp.enable_buzzer(test_one_bit(features_, Feature::Buzzer));
    send_data();
    advi3pp.change_features(features_);
    advi3pp.save_settings();
}

//! Handle the Buzz on Press command
void LcdSettings::buzz_on_press_command()
{
    flip_bits(features_, Feature::BuzzOnPress);
    advi3pp.enable_buzz_on_press(test_one_bit(features_, Feature::BuzzOnPress));
    send_data();
    advi3pp.change_features(features_);
    advi3pp.save_settings();
}

//! Send the current data to the LCD panel.
void LcdSettings::send_data() const
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(static_cast<uint16_t>(features_)) << Uint16(::lcd_contrast);
    frame.send();
}


// --------------------------------------------------------------------
// Print Settings
// --------------------------------------------------------------------

//! Handle Print Settings command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool PrintSettings::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::Baby1:       multiplier_ = Multiplier::M1; break;
        case KeyValue::Baby2:       multiplier_ = Multiplier::M2; break;
        case KeyValue::Baby3:       multiplier_ = Multiplier::M3; break;
        default:                    return false;
    }

    send_data();
    return true;
}

//! Get the value corresponding the the current multiplier.
//! @return The value of the current multiplier, or the first one in the case of an invalid multiplier
double PrintSettings::get_multiplier_value() const
{
    if(multiplier_ < Multiplier::M1 || multiplier_ > Multiplier::M3)
    {
        Log::error() << F("Invalid multiplier value: ") << static_cast<uint16_t >(multiplier_) << Log::endl();
        return PRINT_SETTINGS_MULTIPLIERS[0];
    }

    return PRINT_SETTINGS_MULTIPLIERS[static_cast<uint16_t>(multiplier_)];
}

//! Send the current data to the LCD panel.
void PrintSettings::send_data() const
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(static_cast<uint16_t>(multiplier_));
    frame.send();
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page PrintSettings::do_prepare_page()
{
    send_data();
    return Page::PrintSettings;
}

//! Handle the -Feedrate command
void PrintSettings::feedrate_minus_command()
{
    if(feedrate_percentage <= 50)
        return;

    feedrate_percentage -= 1;
}

//! Handle the +Feedrate command
void PrintSettings::feedrate_plus_command()
{
    if(feedrate_percentage >= 150)
        return;

    feedrate_percentage += 1;
}

//! Handle the -Fan command
void PrintSettings::fan_minus_command()
{
    auto speed = scale(fanSpeeds[0], 255, 100);
    if(speed <= 0)
        return;

	speed = speed <= 5 ? 0 : speed - 5;
    fanSpeeds[0] = scale(speed, 100, 255);
}

//! Handle the +Fan command
void PrintSettings::fan_plus_command()
{
    auto speed = scale(fanSpeeds[0], 255, 100);
    if(speed >= 100)
        return;

    speed = speed >= 100 - 5 ? 100 : speed + 5;
    fanSpeeds[0] = scale(speed, 100, 255);
}

//! Handle the -Hotend Temperature command
void PrintSettings::hotend_minus_command()
{
    auto temperature = Temperature::degTargetHotend(0);
    if(temperature <= 0)
        return;

    Temperature::setTargetHotend(temperature - 1, 0);
}

//! Handle the +Hotend Temperature command
void PrintSettings::hotend_plus_command()
{
    auto temperature = Temperature::degTargetHotend(0);
    if(temperature >= 300)
        return;

    Temperature::setTargetHotend(temperature + 1, 0);
}

//! Handle the -Bed Temperature command
void PrintSettings::bed_minus_command()
{
    auto temperature = Temperature::degTargetBed();
    if(temperature <= 0)
        return;

    Temperature::setTargetBed(temperature - 1);
}

//! Handle the +Bed Temperature command
void PrintSettings::bed_plus_command()
{
    auto temperature = Temperature::degTargetBed();
    if(temperature >= 180)
        return;

    Temperature::setTargetBed(temperature + 1);
}

//! Handle the -Babystep command
void PrintSettings::baby_minus_command()
{
    auto distance = static_cast<int16_t>(-get_multiplier_value() * planner.axis_steps_per_mm[Z_AXIS]);
	Temperature::babystep_axis(Z_AXIS, distance);
}

//! Handle the +Babystep command
void PrintSettings::baby_plus_command()
{
    auto distance = static_cast<int16_t>(get_multiplier_value() * planner.axis_steps_per_mm[Z_AXIS]);
    Temperature::babystep_axis(Z_AXIS, distance);
}

// --------------------------------------------------------------------
// PidSettings
// --------------------------------------------------------------------

PidSettings::PidSettings()
{
    do_reset();
}

//! Handle PID Settings command
//! @param key_value    The sub-action to handle
//! @return             True if the action was handled
bool PidSettings::do_dispatch(KeyValue key_value)
{
    if(Parent::do_dispatch(key_value))
        return true;

    switch(key_value)
    {
        case KeyValue::PidSettingsHotend:   hotend_command(); break;
        case KeyValue::PidSettingsBed:      bed_command(); break;
        case KeyValue::PidSettingPrevious:  previous_command(); break;
        case KeyValue::PidSettingNext:      next_command(); break;
        default:                            return false;
    }

    return true;
}

//! Handle the select Hotend PID command
void PidSettings::hotend_command()
{
    save_data();
    kind_ = TemperatureKind::Hotend;
    send_data();
}

//! Handle the select Bed PID command
void PidSettings::bed_command()
{
    save_data();
    kind_ = TemperatureKind::Bed;
    send_data();
}

//! Handle the show previous PID values command
void PidSettings::previous_command()
{
    if(index_ <= 0)
        return;
    save_data();
    index_ -= 1;
    send_data();
}

//! Handle the show next PID values command
void PidSettings::next_command()
{
    if(index_ >= NB_PIDs - 1)
        return;
    save_data();
    index_ += 1;
    send_data();
}

//! Store current data in permanent memory (EEPROM)
//! @param eeprom EEPROM writer
void PidSettings::do_write(EepromWrite& eeprom) const
{
    for(size_t i = 0; i < NB_PIDs; ++i)
    {
        eeprom.write(bed_pid_[i]);
        eeprom.write(hotend_pid_[i]);
    }
}

//! Restore data from permanent memory (EEPROM).
//! @param eeprom EEPROM reader
void PidSettings::do_read(EepromRead& eeprom)
{
    for(size_t i = 0; i < NB_PIDs; ++i)
    {
        eeprom.read(bed_pid_[i]);
        eeprom.read(hotend_pid_[i]);
    }
}

//! Reset settings
void PidSettings::do_reset()
{
    for(size_t i = 0; i < NB_PIDs; ++i)
    {
        bed_pid_[i].temperature_ = default_bed_temperature;
        bed_pid_[i].Kp_ = DEFAULT_bedKp;
        bed_pid_[i].Ki_ = DEFAULT_bedKi;
        bed_pid_[i].Kd_ = DEFAULT_bedKd;

        hotend_pid_[i].temperature_ = default_hotend_temperature;
        hotend_pid_[i].Kp_ = DEFAULT_Kp;
        hotend_pid_[i].Ki_ = DEFAULT_Ki;
        hotend_pid_[i].Kd_ = DEFAULT_Kd;
    }
}

//! Return the amount of data (in bytes) necessary to save settings in permanent memory (EEPROM).
//! @return Number of bytes
uint16_t PidSettings::do_size_of() const
{
    return NB_PIDs * 2 * sizeof(Pid);
}

//! Set the current PID values from what is recorded
void PidSettings::set_current_pid() const
{
    if(kind_ == TemperatureKind::Hotend)
    {
        const Pid& pid = hotend_pid_[index_];

        Temperature::Kp = pid.Kp_;
        Temperature::Ki = scalePID_i(pid.Ki_);
        Temperature::Kd = scalePID_d(pid.Kd_);

        Log::log() << F("Set Hotend PID #") << index_ << F(" for temperature ") << pid.temperature_
                   << F(", P = ") << pid.Kp_ << F(", I = ") << pid.Ki_ << F(", D = ") << pid.Kd_ << Log::endl();
    }
    else
    {
        const Pid& pid = bed_pid_[index_];

        Temperature::bedKp = pid.Kp_;
        Temperature::bedKi = scalePID_i(pid.Ki_);
        Temperature::bedKd = scalePID_d(pid.Kd_);

        Log::log() << F("Set Bed PID #") << index_ << F(" for temperature ") << pid.temperature_
                   << F(", P = ") << pid.Kp_ << F(", I = ") << pid.Ki_ << F(", D = ") << pid.Kd_ << Log::endl();
    }
}

//! Record the current PID values
void PidSettings::get_current_pid()
{
    if(kind_ == TemperatureKind::Hotend)
    {
        Pid& pid = hotend_pid_[index_];

        pid.Kp_ = Temperature::Kp;
        pid.Ki_ = unscalePID_i(Temperature::Ki);
        pid.Kd_ = unscalePID_d(Temperature::Kd);

        Log::log() << F("Get Hotend PID #") << index_
                   << F(", P = ") << pid.Kp_ << F(", I = ") << pid.Ki_ << F(", D = ") << pid.Kd_ << Log::endl();
    }
    else
    {
        Pid& pid = bed_pid_[index_];

        pid.Kp_ = Temperature::bedKp;
        pid.Ki_ = unscalePID_i(Temperature::bedKi);
        pid.Kd_ = unscalePID_d(Temperature::bedKd);

        Log::log() << F("Get Hotend PID #") << index_
                   << F(", P = ") << pid.Kp_ << F(", I = ") << pid.Ki_ << F(", D = ") << pid.Kd_ << Log::endl();
    }
}

//! Record new PID values for a given temperature
//! @param kind Kind of PID values: Hotend or Bed
//! @param temperature Temperature for these PID values
void PidSettings::add_pid(TemperatureKind kind, uint16_t temperature)
{
    kind_ = kind;
    Pid* pid = kind_ == TemperatureKind::Hotend ? hotend_pid_ : bed_pid_;
    for(size_t i = 0; i < NB_PIDs; ++i)
    {
        if(temperature == pid[i].temperature_)
        {
			Log::log() << (kind == TemperatureKind::Bed ? F("Bed") : F("Hotend"))
			           << F(" PID with temperature 0x") << temperature << F(" found, update settings") << Log::endl();
            index_ = i;
            get_current_pid();
            return;
        }
    }

	Log::log() << (kind == TemperatureKind::Bed ? F("Bed") : F("Hotend"))
	           << F(" PID with temperature 0x") << temperature << F(" NOT found, update settings #0") << Log::endl();
    // Temperature not found, so move PIDs and forget the last one, set index to 0 and update values
    for(size_t i = NB_PIDs - 1; i > 0; --i)
        pid[i] = pid[i - 1];
    index_ = 0;
    pid[0].temperature_ = temperature;
    get_current_pid();
}

//! Choose the best PID values for the given temperature
//! @param kind Kind of PID values: Hotend or Bed
//! @param temperature Temperature for these PID values
void PidSettings::set_best_pid(TemperatureKind kind, uint16_t temperature)
{
    index_ = 0;
    kind_ = kind;

    uint16_t best_difference = 500;
    Pid* pid = kind_ == TemperatureKind::Hotend ? hotend_pid_ : bed_pid_;

    for(size_t i = 0; i < NB_PIDs; ++i)
    {
        auto difference = abs(temperature - pid[i].temperature_);
        if(difference < best_difference)
        {
            best_difference = difference;
            index_ = i;
        }
    }
	
	Log::log() << (kind_ == TemperatureKind::Bed ? F("Bed") : F("Hotend"))
	           << F(" PID with smallest difference (") << best_difference << F(") is at index #") << index_ << Log::endl();
    set_current_pid();
}

//! Send the current data to the LCD panel.
void PidSettings::send_data() const
{
    const Pid& pid = (kind_ == TemperatureKind::Hotend ? hotend_pid_ : bed_pid_)[index_];
    Log::log() << F("Send ") << (kind_ == TemperatureKind::Bed ? F("Bed") : F("Hotend")) << F(" PID #") << index_
               << F(", P = ") << pid.Kp_ << F(", I = ") << pid.Ki_ << F(", D = ") << pid.Kd_ << Log::endl();

    WriteRamDataRequest frame{Variable::Value0};
    frame << (kind_ == TemperatureKind::Hotend ? 0_u16 : 1_u16)
          << Uint16(pid.temperature_)
          << Uint16(pid.Kp_ * 100)
          << Uint16(pid.Ki_ * 100)
          << Uint16(pid.Kd_ * 100);
    frame.send();

    ADVString<8> indexes;
    indexes << index_ + 1 << F(" / ") << NB_PIDs;
    frame.reset(Variable::ShortText0);
    frame << indexes;
    frame.send();
}

//! Save the settings from the LCD Panel.
void PidSettings::save_data()
{
    Pid& pid = (kind_ == TemperatureKind::Hotend ? hotend_pid_ : bed_pid_)[index_];

    ReadRamData response{Variable::Value0, 5};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (PID Settings)") << Log::endl();
        return;
    }

    Uint16 kind, temperature, p, i, d;
    response >> kind >> temperature >> p >> i >> d;

    assert(kind.word == (kind_ == TemperatureKind::Hotend ? 0 : 1));

    pid.Kp_ = static_cast<float>(p.word) / 100;
    pid.Ki_ = static_cast<float>(i.word) / 100;
    pid.Kd_ = static_cast<float>(d.word) / 100;
	
	set_current_pid();
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page PidSettings::do_prepare_page()
{
    send_data();
    return Page::PidSettings;
}

//! Save the PID settings
void PidSettings::do_save_command()
{
    save_data();

    Pid& pid = (kind_ == TemperatureKind::Hotend ? hotend_pid_ : bed_pid_)[index_];
    Temperature::Kp = pid.Kp_;
    Temperature::Ki = pid.Ki_;
    Temperature::Kd = pid.Kd_;

    Parent::do_save_command();
}

//! Execute the Back command
void PidSettings::do_back_command()
{
    advi3pp.restore_settings();
    pid_tuning.send_data();
    Parent::do_back_command();
}

// --------------------------------------------------------------------
// Steps Settings
// --------------------------------------------------------------------

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page StepSettings::do_prepare_page()
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
void StepSettings::do_save_command()
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

    Parent::do_save_command();
}

// --------------------------------------------------------------------
// Feedrate Settings
// --------------------------------------------------------------------

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page FeedrateSettings::do_prepare_page()
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
void FeedrateSettings::do_save_command()
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

    Parent::do_save_command();
}

// --------------------------------------------------------------------
// AccelerationSettings
// --------------------------------------------------------------------

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page AccelerationSettings::do_prepare_page()
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
void AccelerationSettings::do_save_command()
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

    Parent::do_save_command();
}

// --------------------------------------------------------------------
// JerkSettings
// --------------------------------------------------------------------

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page JerkSettings::do_prepare_page()
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
void JerkSettings::do_save_command()
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

    Parent::do_save_command();
}

// --------------------------------------------------------------------
// Linear Advance Settings
// --------------------------------------------------------------------

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page LinearAdvanceSettings::do_prepare_page()
{
    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(Planner::extruder_advance_K * 10);
    frame.send();

    return Page::LinearAdvanceSettings;
}

//! Handles the Save (Continue) command
void LinearAdvanceSettings::do_save_command()
{
    ReadRamData response{Variable::Value0, 1};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Linear Advance Settings)") << Log::endl();
        return;
    }

    Uint16 k; response >> k;
    Planner::extruder_advance_K = k.word / 10.0;

    Parent::do_save_command();
}

// --------------------------------------------------------------------
// Factory Reset
// --------------------------------------------------------------------

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page FactoryReset::do_prepare_page()
{
    return Page::FactoryReset;
}

//! Handles the Save (Continue) command
void FactoryReset::do_save_command()
{
    enqueue_and_echo_commands_P(PSTR("M502"));
    Parent::do_save_command();
}

// --------------------------------------------------------------------
// Statistics
// --------------------------------------------------------------------

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page Statistics::do_prepare_page()
{
    send_stats();
    return Page::Statistics;
}

void Statistics::send_stats()
{
    printStatistics stats = PrintCounter::getStats();

    ADVString<48> printTime{duration_t{stats.printTime}};
    ADVString<48> longestPrint{duration_t{stats.longestPrint}};

    ADVString<48> filament_used;
    filament_used << static_cast<unsigned int>(stats.filamentUsed / 1000)
                  << '.'
                  << (static_cast<unsigned int>(stats.filamentUsed / 100) % 10)
                  << 'm';

    WriteRamDataRequest frame{Variable::Value0};
    frame << Uint16(stats.totalPrints)
          << Uint16(stats.finishedPrints)
          << Uint16(static_cast<uint16_t>(freeMemory()));
    frame.send();

    frame.reset(Variable::LongText0);
    frame << printTime.align(Alignment::Left)
          << longestPrint.align(Alignment::Left)
          << filament_used.align(Alignment::Left);
    frame.send();
}

// --------------------------------------------------------------------
// Versions
// --------------------------------------------------------------------

//! Check if the versions of the different parts (LCD Panel, Mainboard) are compatible and if not, display a message
//! @return True if the versions are compatible
bool Versions::check()
{
    if(!is_lcd_version_valid())
    {
        send_versions();
        pages.show_page(Page::VersionsMismatch, ShowOptions::None);
        return false;
    }

    return true;
}

//! Get the version of the LCD Panel part.
void Versions::get_version_from_lcd()
{
    ReadRamData frame{Variable::ADVi3ppLCDversion, 1};
    if(!frame.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Measures)") << Log::endl();
        return;
    }

    Uint16 version; frame >> version;
    Log::log() << F("ADVi3++ LCD version = ") <<  version.word << Log::endl();
    lcd_version_ = version.word;
}

//! Get the current LCD firmware version.
//! @return     The version as a string.
template<size_t L>
ADVString<L>& get_lcd_firmware_version(ADVString<L>& lcd_version)
{
    ReadRegister response{Register::Version, 1};
    if(!response.send_and_receive())
    {
        Log::error() << F("Receiving Frame (Version)") << Log::endl();
        return lcd_version;
    }

    Uint8 version; response >> version;
    Log::log() << F("LCD Firmware raw version = ") << version.byte << Log::endl();

    lcd_version << (version.byte / 0x10) << '.' << (version.byte % 0x10);
    return lcd_version;
}

//! Convert a version from its hexadecimal representation.
//! @param hex_version  Hexadecimal representation of the version
//! @return             Version as a string
template<size_t L>
ADVString<L>& convert_version(ADVString<L>& version, uint16_t hex_version)
{
    version << hex_version / 0x0100 << '.' << (hex_version % 0x100) / 0x10 << '.' << hex_version % 0x10;
    return version;
}

//! Send the different versions to the LCD screen.
void Versions::send_versions()
{
    ADVString<16> marlin_version{SHORT_BUILD_VERSION};
    ADVString<16> advi3pp_lcd_version;
    ADVString<16> lcd_firmware_version;

    marlin_version.align(Alignment::Left);
    convert_version(advi3pp_lcd_version, lcd_version_).align(Alignment::Left);
    get_lcd_firmware_version(lcd_firmware_version).align(Alignment::Left);

    WriteRamDataRequest frame{Variable::ShortText0};
    frame << advi3pp_lcd_version
          << lcd_firmware_version
          << marlin_version;
    frame.send();
}

//! Send ADVi3++ version and build number to the LCD Panel
void Versions::send_advi3pp_version()
{
    ADVString<16> motherboard_version;
    convert_version(motherboard_version, advi3_pp_version).align(Alignment::Left);

    ADVString<16> build;
    build << (YEAR__ - 2000)
          << (MONTH__ < 10 ? "0" : "") << MONTH__
          << (DAY__   < 10 ? "0" : "") << DAY__
          << (HOUR__  < 10 ? "0" : "") << HOUR__
          << (MIN__   < 10 ? "0" : "") << MIN__
          << (SEC__   < 10 ? "0" : "") << SEC__;

    WriteRamDataRequest frame{Variable::ADVi3ppVersion};
    frame << motherboard_version << build;
    frame.send();
}

//! Check if the versions of the different parts (LCD Panel, Mainboard) are compatible.
//! @return True if the versions are compatible
bool Versions::is_lcd_version_valid()
{
    return lcd_version_ >= advi3_pp_oldest_lcd_compatible_version && lcd_version_ <= advi3_pp_newest_lcd_compatible_version;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page Versions::do_prepare_page()
{
    send_versions();
    return Page::Versions;
}

// --------------------------------------------------------------------
// Sponsors
// --------------------------------------------------------------------

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page Sponsors::do_prepare_page()
{
    return Page::Sponsors;
}

// --------------------------------------------------------------------
// Copyrights
// --------------------------------------------------------------------

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page Copyrights::do_prepare_page()
{
    return Page::Copyrights;
}

// --------------------------------------------------------------------
// Change Filament
// --------------------------------------------------------------------

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page ChangeFilament::do_prepare_page()
{
    return Page::None;
}


// --------------------------------------------------------------------
// EEPROM data mismatch
// --------------------------------------------------------------------

//! Check if the EEPROM values are compatible or not. If not, display a message.
//! @return False if the values are compatible
bool EepromMismatch::check()
{
    if(does_mismatch())
    {
        show(ShowOptions::None);
        return false;
    }

    return true;
}

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page EepromMismatch::do_prepare_page()
{
    return Page::EEPROMMismatch;
}

//! Handles the Save (Continue) command
void EepromMismatch::do_save_command()
{
    advi3pp.save_settings();
    pages.show_page(Page::Main);
}

//! Check the EEPROM value mismatch flag.
bool EepromMismatch::does_mismatch() const
{
    return mismatch_;
}

//! Set the EEPROM value mismatch flag.
void EepromMismatch::set_mismatch()
{
    mismatch_ = true;
}

//! Reset the EEPROM value mismatch flag.
void EepromMismatch::reset_mismatch()
{
    mismatch_ = false;
}

// --------------------------------------------------------------------
// No Sensor
// --------------------------------------------------------------------

//! Prepare the page before being displayed and return the right Page value
//! @return The index of the page to display
Page NoSensor::do_prepare_page()
{
    return Page::NoSensor;
}


// --------------------------------------------------------------------
// Dimming
// --------------------------------------------------------------------

//! Set LCD Panel brightness
//! @param brightness New brightness
void ADVi3pp_::set_brightness(int16_t brightness)
{
    dimming.change_brightness(brightness);
}

//! Constructor. Initialize dimming check time and dimming delay time
Dimming::Dimming()
{
    set_next_checking_time();
    set_next_dimmming_time();
}

//! Enable or disable dimming of the LCD Panel
//! @param enable Enable or disable dimming
//! @param doIt Chnage the dimming right now or not
void Dimming::enable(bool enable, bool doIt)
{
    enabled_ = enable;
    if(doIt)
        reset(true);
}

//! Set the next dimming check time
void Dimming::set_next_checking_time()
{
    next_check_time_ = millis() + 200;
}

//! Set the next dimming delay time
void Dimming::set_next_dimmming_time()
{
    next_dimming_time_ = millis() + 1000ul * dimming_delay;
}

//! Get adjusted brightness (depended of the active dimming or not)
//! @return The adjusted brightness
uint8_t Dimming::get_adjusted_brightness()
{
    int16_t brightness = ::lcd_contrast;
    if(dimming_)
        brightness = brightness * dimming_ratio / 100;
    if(brightness < brightness_min)
        brightness = brightness_min;
    if(brightness > brightness_max)
        brightness = brightness_max;
    return static_cast<uint8_t>(brightness);
}

//! Check the dimming state: LCD Panel was touched? Dimming delay was elapsed?
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

    // Reset TouchPanelFlag
    WriteRegisterDataRequest request{Register::TouchPanelFlag};
    request << 00_u8;
    request.send(false);

    if(value.byte == 0x5A)
    {
        Log::log() << F("Panel touched, reset dimming") << Log::endl();
        reset();
        return;
    }

    if(!dimming_ && ELAPSED(millis(), next_dimming_time_))
    {
        Log::log() << F("Delay elapsed, dim the panel") << Log::endl();
        dimming_ = true;
        send_brightness();
    }
}

//! Reset the dimming
//! @param force Change the brightness right now
void Dimming::reset(bool force)
{
    set_next_dimmming_time();
    if(!force && !dimming_) // Already reset, nothing more to do (unless force is true)
        return;

    dimming_ = false;
    send_brightness();
}

//! Adjust the brigthness of the LCD panel
void Dimming::send_brightness()
{
    auto brightness = get_adjusted_brightness();

    WriteRegisterDataRequest frame{Register::Brightness};
    frame << Uint8{brightness};
    frame.send(true);
}

//! Change the brightness of the LCD Panel.
//! @param brightness New brightness
void Dimming::change_brightness(int16_t brightness)
{
    ::lcd_contrast = brightness;
    reset();
    send_brightness();
}


// --------------------------------------------------------------------

}
