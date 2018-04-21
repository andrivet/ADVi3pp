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
    ResetTrigger            = 0xEE
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
    Message                 = 0x0006,
    Progress                = 0x001A,
    CenteredMessage         = 0x002E,

    MotherboardVersion      = 0x0100,
    LcdVersion              = 0x0108,
    LcdFirmwareVersion      = 0x0110,
    MarlinVersion           = 0x0118,

    CurrentFileName         = 0x0169,
    TotalPrints             = 0x0181,
    CompletedPrints         = 0x0182,
    TotalPrintTime          = 0x0183,
    LongestPrintTime        = 0x018B,
    TotalFilament           = 0x0193,
    Value                   = 0x019B,

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
    LCDBrightness           = 0x031C,  // Also an action
    CurrentSensor           = 0x031D,
    Features                = 0x031E,
    Preset1Bed              = 0x031F,
    Preset1Hotend           = 0x0320,
    Preset2Bed              = 0x0321,
    Preset2Hotend           = 0x0322,
    Preset3Bed              = 0x0323,
    Preset4Hotend           = 0x0324,
    USBBaudrate             = 0x0325,
    SensorOffsetZ           = 0x0327,

    ADVi3ppLCDversion       = 0x0500,
    TargetTemperature       = 0x0501,
    Value0                  = 0x0510,
    Value1                  = 0x0511,
    Value2                  = 0x0512,
    Value3                  = 0x0513,
    Value4                  = 0x0514,
    Value5                  = 0x0515,
    Value6                  = 0x0516,
    Value7                  = 0x0517,
    Value8                  = 0x0518
};

//! List of actions sent by the LCD.
enum class Action: uint16_t
{
    Screen                  = 0x0400,
    PrintCommand            = 0x0401,
    LoadUnload              = 0x0403,
    Preheat                 = 0x0404,
    Move                    = 0x0405,
    SdCard                  = 0x0406,
    FactoryReset            = 0x0407,
    Leveling                = 0x0408,
    ExtruderTuning          = 0x0409,
    XYZMotorsTuning         = 0x040A,
    PidTuning               = 0x040B,
    SensorSettings          = 0x040C,
    Firmware                = 0x040D,
    NoSensor                = 0x040E,
    LCD                     = 0x040F,
    Statistics              = 0x0410,
    Versions                = 0x0411,
    PrintSettings           = 0x0412,
    PIDSettings             = 0x0413,
    StepsSettings           = 0x0414,
    FeedrateSettings        = 0x0415,
    AccelerationSettings    = 0x0416,
    JerkSettings            = 0x0417,
    Copyrights              = 0x0418,
    SensorTuning            = 0x0419,
    SensorGrid              = 0x041A,
    SensorZHeight           = 0x041B,
    ChangeFilament          = 0x041C,
    EEPROMMismatch          = 0x041D,

    MoveXMinus              = 0x0420,
    MoveXPlus               = 0x0421,
    MoveYMinus              = 0x0422,
    MoveYPlus               = 0x0423,
    MoveZMinus              = 0x0424,
    MoveZPlus               = 0x0425,
    MoveEMinus              = 0x0426,
    MoveEPlus               = 0x0427,

    LCDBrightness           = 0x031C, // Also a Variable

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

    TuningStart             = 0x0001,
    TuningSettings          = 0x0002,

    PidTuningStep1          = 0x0000,
    PidTuningStep2          = 0x0001,

    ThermalProtection       = 0x0001,
    USBBaudrateMinus        = 0x0002,
    USBBaudratePlus         = 0x0003,

    LCDDimming              = 0x0001,
    Buzzer                  = 0x0002,
    BuzzOnPress             = 0x0003,

    MismatchForward         = 0x0001,

    SensorLeveling          = 0x0001,
    SensorSelfTest          = 0x0002,
    SensorReset             = 0x0003,
    SensorDeploy            = 0x0004,
    SensorStow              = 0x0005,
    SensorZHeight           = 0x0006,

    Save                    = 0xFFFE,
    Continue                = 0xFFFE,
    Back                    = 0xFFFF,
    Cancel                  = 0xFFFF
};

enum class Feature: uint16_t
{
    HeadParking         = 0b0000000000000001,
    ThermalProtection   = 0b0000000000000010,
    Dimming             = 0b0000000000000100,
    Buzzer              = 0b0000000000001000,
    BuzzOnPress          = 0b0000000000010000
};
ENABLE_BITMASK_OPERATOR(Feature);

enum class Brightness
{
    Min = 0x00,
    Max = 0x40
};

}}

#endif //ADV_I3_PLUS_PLUS_ENUMS_H
