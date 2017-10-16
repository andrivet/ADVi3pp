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
#ifndef ADV_I3_PLUS_PLUS_ENUMS_H
#define ADV_I3_PLUS_PLUS_ENUMS_H

namespace advi3pp { inline namespace internals {


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
    TargetBed               = 0x0000,
    Bed                     = 0x0001,
    TargetHotEnd            = 0x0002,
    HotEnd                  = 0x0003,
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

}}

#endif //ADV_I3_PLUS_PLUS_ENUMS_H
