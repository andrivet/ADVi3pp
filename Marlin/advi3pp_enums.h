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

#include <stdint.h>
#include "advi3pp_bitmasks.h"

namespace advi3pp {


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
    ProgressLow             = 0x0044,
    ProgressHigh            = 0x0045,

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

    FileName1               = 0x0200,
    FileName2               = 0x0218,
    FileName3               = 0x0230,
    FileName4               = 0x0248,
    FileName5               = 0x0260,

    ADVi3ppLCDversion       = 0x0500,
    Value0                  = 0x0510,
    Value1                  = 0x0511,
    Value2                  = 0x0512,
    Value3                  = 0x0513,
    Value4                  = 0x0514,
    Value5                  = 0x0515,
    Value6                  = 0x0516,
    Value7                  = 0x0517,
    Value8                  = 0x0518,
    ValueText               = 0x0540,
    CenteredTextValue       = 0x0554
};

//! List of actions sent by the LCD.
enum class Action: uint16_t
{
    Screen                  = 0x0400,
    PrintCommand            = 0x0401,
    Wait                    = 0x0402,
    LoadUnload              = 0x0403,
    Preheat                 = 0x0404,
    Move                    = 0x0405,
    SdCard                  = 0x0406,
    FactoryReset            = 0x0407,
    ManualLeveling          = 0x0408,
    ExtruderTuning          = 0x0409,
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
    Sponsors                = 0x041E,
    LinearAdvanceTuning     = 0x041F,
    LinearAdvanceSettings   = 0x0420,
    Diagnosis               = 0x0421,

    MoveXMinus              = 0x0450,
    MoveXPlus               = 0x0451,
    MoveYMinus              = 0x0452,
    MoveYPlus               = 0x0453,
    MoveZMinus              = 0x0454,
    MoveZPlus               = 0x0455,
    MoveEMinus              = 0x0456,
    MoveEPlus               = 0x0457,
    LCDBrightness           = 0x0458,
    BabyMinus               = 0x0459,
    BabyPlus                = 0x045A,

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
    Leveling                = 0x0007,

    PrintStop               = 0x0000,
    PrintPauseResume        = 0x0001,
    PrintAdvancedPause      = 0x0002,

    Load                    = 0x0001,
    Unload                  = 0x0002,

    PresetPrevious          = 0x0001,
    PresetNext              = 0x0002,
    Cooldown                = 0xAAAA,

    MoveXHome               = 0x0001,
    MoveYHome               = 0x0002,
    MoveZHome               = 0x0003,
    MoveAllHome             = 0x0004,
    DisableMotors           = 0x0005,

    SDLine1                 = 0x0001,
    SDLine2                 = 0x0002,
    SDLine3                 = 0x0003,
    SDLine4                 = 0x0004,
    SDLine5                 = 0x0005,
    SDUp                    = 0x8000,
    SDDown                  = 0x8001,

    LevelingPoint1          = 0x0001,
    LevelingPoint2          = 0x0002,
    LevelingPoint3          = 0x0003,
    LevelingPoint4          = 0x0004,
    LevelingPoint5          = 0x0005,
    LevelingPointA          = 0x0006,
    LevelingPointB          = 0x0007,
    LevelingPointC          = 0x0008,
    LevelingPointD          = 0x0009,

    TuningStart             = 0x0001,
    TuningSettings          = 0x0002,

    PidTuningStep2          = 0x0001,
    PidTuningHotend         = 0x0002,
    PidTuningBed            = 0x0003,

    SensorSettingsPrevious  = 0x0001,
    SensorSettingsNext      = 0x0002,

    ThermalProtection       = 0x0001,
    USBBaudrateMinus        = 0x0002,
    USBBaudratePlus         = 0x0003,
    RunoutSensor            = 0x0004,

    LCDDimming              = 0x0001,
    Buzzer                  = 0x0002,
    BuzzOnPress             = 0x0003,

    MismatchForward         = 0x0001,

    PidSettingsHotend       = 0x0001,
    PidSettingsBed          = 0x0002,

    SensorSelfTest          = 0x0001,
    SensorReset             = 0x0002,
    SensorDeploy            = 0x0003,
    SensorStow              = 0x0004,
    SensorZHeight           = 0x0005,

    ZHeight01               = 0x0001,
    ZHeight05               = 0x0002,
    ZHeight10               = 0x0003,

    Baby001                 = 0x0001,
    Baby005                 = 0x0002,
    Baby010                 = 0x0003,

    Save                    = 0xFFFE,
    Back                    = 0xFFFF
};

enum class Feature: uint16_t
{
    None                = 0b0000000000000000,
    HeadParking         = 0b0000000000000001,
    ThermalProtection   = 0b0000000000000010,
    Dimming             = 0b0000000000000100,
    Buzzer              = 0b0000000000001000,
    BuzzOnPress         = 0b0000000000010000,
    RunoutSensor        = 0b0000000000100000
};
ENABLE_BITMASK_OPERATOR(Feature);

}

#endif //ADV_I3_PLUS_PLUS_ENUMS_H
