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

#ifndef ADV_I3_PLUS_PLUS_PAGES_H
#define ADV_I3_PLUS_PLUS_PAGES_H

namespace advi3pp {

enum class Page: uint8_t
{
    None                    = 0,
    Boot                    = 1,
    Main                    = 22,
    Controls                = 24,
    Tuning                  = 26,
    Settings                = 28,
    LoadUnload              = 30,
    WaitBack                = 32,
    WaitBackContinue        = 34,
    Preheat                 = 36,
    Move                    = 38,
    SdCard                  = 40,
    Print                   = 42,
    Sponsors                = 44,
    Waiting                 = 46,
    ManualLeveling          = 48,
    ExtruderTuningTemp      = 50,
    WaitContinue            = 52,
    ExtruderTuningMeasure   = 54,
    Leveling                = 56,
    PidTuning               = 58,
    MotorsSettings          = 62,
    PidSettings             = 64,
    FactoryReset            = 66,
    Statistics              = 68,
    Versions                = 70,
    StepsSettings           = 72,
    FeedrateSettings        = 74,
    AccelerationSettings    = 76,
    JerkSettings            = 78,
    PrintSettings           = 80,
    ThermalRunawayError     = 82,
    VersionsMismatch        = 84,
    Temperature             = 86,
    Infos                   = 88,
    Firmware                = 90,
    NoSensor                = 92,
    SensorSettings          = 94,
    LCD                     = 96,
    Copyrights              = 98,
    SensorTuning            = 100,
    SensorGrid              = 102,
    EEPROMMismatch          = 104,
    ZHeightTuning           = 106,
    LinearAdvanceTuning     = 108,
    LinearAdvanceSettings   = 110,
    Diagnosis               = 112
};

}

#endif //ADV_I3_PLUS_PLUS_PAGES_H
