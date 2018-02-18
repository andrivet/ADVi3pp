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
    ZHeight                 = 0x0005,
    LogoAnimation           = 0x0006,

    MotherboardVersion      = 0x0100,
    LcdVersion              = 0x0108,
    LcdFirmwareVersion      = 0x0110,
    MarlinVersion           = 0x0118,
    Message                 = 0x0120,
    Progress                = 0x0138,
    Error                   = 0x0150,
    CurrentFileName         = 0x0168,
    TotalPrints             = 0x0180,
    CompletedPrints         = 0x0181,
    TotalPrintTime          = 0x0182,
    LongestPrintTime        = 0x018A,
    TotalFilament           = 0x0192,

    FileName1               = 0x0200,
    FileName2               = 0x0218,
    FileName3               = 0x0230,
    FileName4               = 0x0248,
    FileName5               = 0x0260,

    StepSettingsX           = 0x0300,
    StepSettingsY           = 0x0301,
    StepSettingsZ           = 0x0302,
    StepSettingsE           = 0x0303,
    PidP                    = 0x0304,
    PidI                    = 0x0305,
    PidD                    = 0x0306,
    PrintSettingsSpeed      = 0x0307,
    PrintSettingsHotend     = 0x0308,
    PrintSettingsBed        = 0x0309,
    PrintSettingsFan        = 0x030A,
    FeedrateMaxX            = 0x030B,
    FeedrateMaxY            = 0x030C,
    FeedrateMaxZ            = 0x030D,
    FeedrateMaxE            = 0x030E,
    FeedratMin              = 0x030F,
    FeedratTravel           = 0x0310,
    AccelerationMaxX        = 0x0311,
    AccelerationMaxY        = 0x0312,
    AccelerationMaxZ        = 0x0313,
    AccelerationMaxE        = 0x0314,
    AccelerationPrint       = 0x0315,
    AccelerationRetract     = 0x0316,
    AccelerationTravel      = 0x0317,
    JerkX                   = 0x0318,
    JerkY                   = 0x0319,
    JerkZ                   = 0x031A,
    JerkE                   = 0x031B,
    USBBaudrate             = 0x031C,
    CurrentSensor           = 0x031D,
    Features                = 0x031E,
    Preset1Bed              = 0x031F,
    Preset1Hotend           = 0x0320,
    Preset2Bed              = 0x0321,
    Preset2Hotend           = 0x0322,
    Preset3Bed              = 0x0323,
    Preset4Hotend           = 0x0324,

    TargetTemperature       = 0x0500,
    Measure1                = 0x0501,
    Measure2                = 0x0502,
    Measure3                = 0x0503,
};

//! List of actions sent by the LCD.
enum class Action: uint16_t
{
    Screen                  = 0x0400,
    SdPrintCommand          = 0x0401,
    UsbPrintCommand         = 0x0402,
    LoadUnload              = 0x0403,
    Preheat                 = 0x0404,
    Move                    = 0x0405,
    SdCard                  = 0x0406,
    FactoryReset            = 0x0407,
    Leveling                = 0x0408,
    ExtruderCalibration     = 0x0409,
    XYZMotorsCalibration    = 0x040A,
    PidTuning               = 0x040B,
    Sensor                  = 0x040C,
    Firmware                = 0x040D,
    USB                     = 0x040E,
    LCD                     = 0x040F,
    Statistics              = 0x0410,
    About                   = 0x0411,
    PrintSettings           = 0x0412,
    PIDSettings             = 0x0413,
    StepsSettings           = 0x0414,
    FeedrateSettings        = 0x0415,
    AccelerationSettings    = 0x0416,
    JerkSettings            = 0x0417,

    MoveXMinus              = 0x0420,
    MoveXPlus               = 0x0421,
    MoveYMinus              = 0x0422,
    MoveYPlus               = 0x0423,
    MoveZMinus              = 0x0424,
    MoveZPlus               = 0x0425,
    MoveEMinus              = 0x0426,
    MoveEPlus               = 0x0427,

    Undefined               = 0xFFFF
};

//! Key values used by the LCD screens.
enum class KeyValue: uint16_t
{
    Show                    = 0x0000,

    Temps                   = 0x0000,
    Print                   = 0x0001,
    Controls                = 0x0002,
    Tuning                  = 0x0003,
    Settings                = 0x0004,
    Infos                   = 0x0005,
    Motors                  = 0x0006,

    PrintStop               = 0x0000,
    PrintPause              = 0x0001,
    PrintResume             = 0x0002,

    Load                    = 0x0001,
    Unload                  = 0x0002,

    Preset1                 = 0x0001,
    Preset2                 = 0x0002,
    Preset3                 = 0x0003,
    Cooldown                = 0xAAAA,

    MoveXHome               = 0x0001,
    MoveYHome               = 0x0002,
    MoveZHome               = 0x0003,
    MoveAllHome             = 0x0004,
    MoveAllDisable          = 0x0005,

    SDLine1                 = 0x0001,
    SDLine2                 = 0x0002,
    SDLine3                 = 0x0003,
    SDLine4                 = 0x0004,
    SDLine5                 = 0x0005,
    SDUp                    = 0x8000,
    SDDown                  = 0x8001,

    ResetConfirm            = 0x0001,

    LevelingPoint1          = 0x0001,
    LevelingPoint2          = 0x0002,
    LevelingPoint3          = 0x0003,
    LevelingPoint4          = 0x0004,
    LevelingPoint5          = 0x0005,

    CalibrationStart        = 0x0001,
    CalibrationSettings     = 0x0002,

    PidTuningStep1          = 0x0000,
    PidTuningStep2          = 0x0001,

    SensorNone              = 0x0001,
    SensorBLTouch           = 0x0002,
    Sensor3Wires            = 0x0003,

    FirmwareProtection      = 0x0001,
    FirmwareHeadParking     = 0x0002,

    USBBaudrateMinus        = 0x0001,
    USBBaudratePlus         = 0x0002,

    LCDBuzzer               = 0x0001,
    LCDDimming              = 0x0002,

    AboutForward            = 0x0001,

    Save                    = 0xFFFE,
    Back                    = 0xFFFF
};

}}

#endif //ADV_I3_PLUS_PLUS_ENUMS_H
