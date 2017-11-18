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
    WriteRegisterData       = 0x80, // 128
    ReadRegisterData        = 0x81, // 129
    WriteRamData            = 0x82, // 130
    ReadRamData             = 0x83, // 131
    WriteCurveData          = 0x84  // 132
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
    RunTime                 = 0x0C,
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
    Message                 = 0x0005,
    FileName1               = 0x0100,
    FileName2               = 0x010D,
    FileName3               = 0x011A,
    FileName4               = 0x0127,
    FileName5               = 0x0134,
    SelectedFileName        = 0x014E,

    StepSettingsX           = 0x0300,
    StepSettingsY           = 0x0301,
    StepSettingsZ           = 0x0302,
    StepSettingsE           = 0x0303,
    PidP                    = 0x0310,
    PidI                    = 0x0311,
    PidD                    = 0x0312,
    PrintSettingsSpeed      = 0x0320,
    PrintSettingsHotend     = 0x0321,
    PrintSettingsBed        = 0x0322,
    PrintSettingsFan        = 0x0323,
    FeedrateMaxX            = 0x0330,
    FeedrateMaxY            = 0x0331,
    FeedrateMaxZ            = 0x0332,
    FeedrateMaxE            = 0x0333,
    FeedratMin              = 0x0334,
    FeedratTravel           = 0x0335,
    AccelerationMaxX        = 0x0340,
    AccelerationMaxY        = 0x0341,
    AccelerationMaxZ        = 0x0342,
    AccelerationMaxE        = 0x0343,
    AccelerationPrint       = 0x0344,
    AccelerationRetract     = 0x0345,
    AccelerationTravel      = 0x0346,
    JerkX                   = 0x0350,
    JerkY                   = 0x0351,
    JerkZ                   = 0x0352,
    JerkE                   = 0x0353,

    MarlinVersion           = 0x0500,
    MotherboardVersion      = 0x0508,
    LcdVersion              = 0x0510,
    LcdFirmwareVersion      = 0x0518,
    TargetTemperature       = 0x0520,
    Measure1                = 0x0521,
    Measure2                = 0x0522,
    Measure3                = 0x0523,
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
    Main                    = 0x0400,
    SdPrintCommand          = 0x0401,
    UsbPrintCommand         = 0x0402,
    LoadUnload              = 0x0410,
    Preheat                 = 0x0412,
    Cooldown                = 0x0413,
    Move                    = 0x0414,
    SdCard                  = 0x0420,
    SdCardSelectFile        = 0x0421,
    ShowSettings            = 0x0430,
    SaveSettings            = 0x0431,
    CancelSettings          = 0x0432,
    FactoryReset            = 0x0433,
    Leveling                = 0x0440,
    ExtruderCalibration     = 0x0441,
    XYZMotorsCalibration    = 0x0442,
    PidTuning               = 0x0443,
    Statistics              = 0x0450,
    About                   = 0x0451,

    MoveXplus               = 0x0200,
    MoveXminus              = 0x0201,
    MoveYplus               = 0x0210,
    MoveYminus              = 0x0211,
    MoveZplus               = 0x0220,
    MoveZminus              = 0x0221,
    MoveEplus               = 0x0230,
    MoveEminus              = 0x0231,

    Undefined               = 0xFFFF
};

//! Key values used by the LCD screens.
enum class KeyValue: uint16_t
{
    MainTemps               = 0x0000,
    MainSD                  = 0x0001,
    MainControls            = 0x0002,
    MainCalibrations        = 0x0003,
    MainSettings            = 0x0004,
    MainMotors              = 0x0005,

    PrintStop               = 0x0000,
    PrintPause              = 0x0001,
    PrintResume             = 0x0002,

    LoadUnload              = 0x0000,
    Load                    = 0x0001,
    Unload                  = 0x0002,

    PreheatShow             = 0x0000,

    MoveShow                = 0x0000,
    HomeAll                 = 0x0100,
    HomeX                   = 0x0101,
    HomeY                   = 0x0102,
    HomeZ                   = 0x0103,
    DisableMotors           = 0x0200,

    SdUp                    = 0x0000,
    SdDown                  = 0x0001,

    SettingsPrint           = 0x0000,
    SettingsPID             = 0x0001,
    SettingsSteps           = 0x0002,
    SettingsFeedrate        = 0x0003,
    SettingsAcceleration    = 0x0004,
    SettingsJerk            = 0x0005,

    StatisticsShow          = 0x0000,

    LevelingHome            = 0x0000,
    LevelingPoint1          = 0x0001,
    LevelingPoint2          = 0x0002,
    LevelingPoint3          = 0x0003,
    LevelingPoint4          = 0x0004,
    LevelingPoint5          = 0x0005,

    CalibrationShow         = 0x0000,
    CalibrationStart        = 0x0001,
    CalibrationSettings     = 0x0002,

    PidTuningStep1          = 0x0000,
    PidTuningStep2          = 0x0001,

    AboutForward            = 0x0001,

    ResetShow               = 0x0000,
    ResetConfirm            = 0x0001,

    Back                    = 0x0099,
    Cancel                  = 0x0099
};

}}

#endif //ADV_I3_PLUS_PLUS_ENUMS_H
