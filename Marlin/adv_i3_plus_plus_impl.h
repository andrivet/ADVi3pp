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
#ifndef ADV_I3_PLUS_PLUS_PRIVATE_H
#define ADV_I3_PLUS_PLUS_PRIVATE_H

namespace advi3pp { inline namespace internals {

enum class BackgroundTask: uint8_t
{
    None                = 0,
    LevelInit           = 1,
    LoadFilament        = 2,
    UnloadFilament      = 3,
    Move                = 4
};

//! List of commands and their values.
enum class Command: uint8_t
{
    WriteRegisterData       = 0x80,
    ReadRegisterData        = 0x81,
    WriteRamData            = 0x82,
    ReadRamData             = 0x83,
    WriteCurveData          = 0x84
};

//! List of registers and their values.
enum class Register: uint8_t
{
    Version                 = 0x00,
    Brightness              = 0x01,
    BuzzerBeepingTime       = 0x02,
    PictureID               = 0x03,
    TouchPanelFlag          = 0x05,
    TouchPanelStatus        = 0x06,
    TouchPanelPosition      = 0x07,
    TouchPanelEnable        = 0x0B,
    RunTime                 = 0xC,
    R0                      = 0x10,
    R1                      = 0x11,
    R2                      = 0x12,
    R3                      = 0x13,
    R4                      = 0x14,
    R5                      = 0x15,
    R6                      = 0x16,
    R7                      = 0x17,
    R8                      = 0x18,
    R9                      = 0x19,
    RA                      = 0x1A,
    RtcComAdj               = 0x1F,
    RtcNow                  = 0x20,
    EnLibOP                 = 0x40,
    LibOPMode               = 0x41,
    LibID                   = 0x42,
    LibAddress              = 0x43,
    VP                      = 0x46,
    OPLength                = 0x48,
    Timer0                  = 0x4A,
    Timer1                  = 0x4C,
    Timer2                  = 0x4D,
    Timer3                  = 0x4E,
    KeyCode                 = 0x4F,
    TrendlineClear          = 0xEB,
    ResetTriger             = 0xEE
};

//! List of variables and their addresses.
enum class Variable: uint16_t
{
    TargetHotEnd            = 0x0000,
    HotEnd                  = 0x0001,
    TargetBed               = 0x0002,
    Bed                     = 0x0003,
    FanSpeed                = 0x0004,
    FilePercentDonc         = 0x0005,
    FileName1               = 0x0100,
    FileName2               = 0x010D,
    FileName3               = 0x011A,
    FileName4               = 0x0127,
    FileName5               = 0x0134,
    SelectedFileName        = 0x014E,

    MotorSettingsX          = 0x0324,
    MotorSettingsY          = 0x0325,
    MotorSettingsZ          = 0x0326,
    MotorSettingsE          = 0x0327,
    P                       = 0x0328,
    I                       = 0x0329,
    E                       = 0x032A,
    PrintSettingsSpeed      = 0x032B,
    PrintSettingsHotend     = 0x032C,
    PrintSettingsBed        = 0x032D,
    PrintSettingsFan        = 0x032E,

    MarlinVersion           = 0x0500,
    MotherboardVersion      = 0x0508,
    LcdVersion              = 0x0510,
    LcdFirmwareVersion      = 0x0518,
    TargetTemperature       = 0x0520,
    TotalPrints             = 0x0540,
    CompletedPrints         = 0x0541,
    TotalPrintTime          = 0x0542,
    LongestPrintTime        = 0x054D,
    TotalFilament           = 0x0558,
    Preset1Bed              = 0x0570,
    Preset1Hotend           = 0x0571,
    Preset2Bed              = 0x0572,
    Preset2Hotend           = 0x0573,
    Preset3Bed              = 0x0574,
    Preset4Hotend           = 0x0575
};

//! List of actions sent by the LCD.
enum class Action: uint16_t
{
    XPlus                   = 0x0400,
    XMinus                  = 0x0401,
    YPlus                   = 0x0402,
    YMinus                  = 0x0403,
    ZPlus                   = 0x0404,
    ZMinus                  = 0x0405,
    EPlus                   = 0x0406,
    EMinus                  = 0x0407,
    HomeAll                 = 0x041C,
    SdCard                  = 0x0432,
    SdCardSelectFile        = 0x0433,
    Cooldown                = 0x0434,
    PrintStop               = 0x0435,
    PrintPause              = 0x0436,
    PrintResume             = 0x0437,
    Preheat                 = 0x043C,
    TemperatureGraph        = 0x043D,
    MotorsSettings          = 0x043E,
    SaveSettings            = 0x043F,
    SavePrintSettings       = 0x0440,
    FactoryReset            = 0x0442,
    HomeX                   = 0x0443,
    HomeY                   = 0x0444,
    HomeZ                   = 0x0445,
    PrintSettings           = 0x0447,
    LoadUnloadBack          = 0x044A,
    Level                   = 0x044C,
    Filament                = 0x0451,
    DisableMotors           = 0x0454,
    Print                   = 0x0455,
    Statistics              = 0x045B,
    PidTuning               = 0x045C,
    About                   = 0x0460,
    LcdUpdate               = 0x0461
};

//! Key values used by the LCD screens.
enum class KeyValue: uint16_t
{
    Show                    = 0x0000,
    Up                      = 0x0001,
    Down                    = 0x002,
    Load                    = 0x0001,
    Unload                  = 0x0002,
    MotorsSettings          = 0x0000,
    PidSettings             = 0x0001,
    LevelStart              = 0x0000,
    LevelStep1              = 0x0001,
    LevelStep2              = 0x0002,
    LevelStep3              = 0x0003,
    LevelStep4              = 0x0004,
    LevelFinish             = 0x0006,
    AutoPid                 = 0x0001,
    Back                    = 0x0001
};

// --------------------------------------------------------------------
// Preset
// --------------------------------------------------------------------

//! Hostend and bad temperature preset.
struct Preset
{
    uint16_t hotend;
    uint16_t bed;
};

// --------------------------------------------------------------------
// i3PlusPrinterImpl
// --------------------------------------------------------------------

//! Implementation of the Duplication i3 Plus printer and its LCD
class i3PlusPrinterImpl
{
public:
    void setup();
    void task();
    void show_page(Page page);
    void update_graph_data();
    void auto_pid_finished();
    void store_presets(eeprom_write write, int eeprom_index, uint16_t& working_crc);
    void restore_presets(eeprom_read read, int eeprom_index, uint16_t& working_crc);
    void reset_presets();
    void temperature_error();

private:
    void send_versions();
    void execute_background_task();
    void leveling_init();
    void unload_filament();
    void load_filament();
    void send_status_update();
    Page get_current_page();
    void read_lcd_serial();
    void send_stats();
    template<size_t S> void get_file_name(uint8_t index, Name<S>& name);
    Name<16> get_lcd_firmware_version();
    void set_next_background_task_time(unsigned int delta = 500);
    void set_next_update_time(unsigned int delta = 500);

private: // Actions
    void sd_card(KeyValue key_value);
    void sd_card_select_file(KeyValue key_value);
    void print_stop(KeyValue key_value);
    void print_pause(KeyValue key_value);
    void print_resume(KeyValue key_value);
    void preheat(KeyValue key_value);
    void cooldown(KeyValue key_value);
    void motors_or_pid_settings(KeyValue key_value);
    void save_motors_or_pid_settings(KeyValue key_value);
    void factory_reset(KeyValue key_value);
    void print_settings(KeyValue key_value);
    void save_print_settings(KeyValue key_value);
    void load_unload_back(KeyValue key_value);
    void level(KeyValue key_value);
    void filament(KeyValue key_value);
    void move_x_plus(KeyValue key_value);
    void move_x_minus(KeyValue key_value);
    void move_y_plus(KeyValue key_value);
    void move_y_minus(KeyValue key_value);
    void move_z_plus(KeyValue key_value);
    void move_z_minus(KeyValue key_value);
    void move_e_plus(KeyValue key_value);
    void move_e_minus(KeyValue key_value);
    void disable_motors(KeyValue key_value);
    void home_X(KeyValue key_value);
    void home_y(KeyValue key_value);
    void home_z(KeyValue key_value);
    void home_all(KeyValue key_value);
    void statistics(KeyValue key_value);
    void pid_tuning(KeyValue key_value);
    void temperature_graph(KeyValue key_value);
    void print(KeyValue key_value);
    void about(KeyValue key_value);
    void lcd_update_mode(KeyValue key_value);

private:
    static const size_t NB_PRESETS = 3;

    uint16_t last_file_index_ = 0;
    millis_t next_op_time_ = 0;
    millis_t next_update_time_ = 0;
    BackgroundTask background_task_ = BackgroundTask::None;
    bool temp_graph_update_ = false;
    Page last_page_ = Page::None;
    Preset presets_[NB_PRESETS];
    uint16_t adv_i3_pp_lcd_version_ = 0x0000;
};

}}

#endif //ADV_I3_PLUS_PLUS_PRIVATE_H

