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

#include "advi3pp.h"
#include "advi3pp_.h"

#ifdef DEBUG
#pragma message "This is a DEBUG build"
#endif

#ifdef ADVi3PP_BLTOUCH
#pragma message "This is a BLTouch build"
#endif

namespace advi3pp {

inline namespace singletons
{
    extern ADVi3pp_ advi3pp;
    extern PidTuning pid_tuning;
    extern SensorTuning sensor_tuning;
}

// --------------------------------------------------------------------
// ADVi3++
// --------------------------------------------------------------------

//! Initialize the printer and its LCD.
void ADVi3pp::setup()
{
    advi3pp.setup();
}

//! Read data from the LCD and act accordingly.
void ADVi3pp::idle()
{
    advi3pp.idle();
}

//! PID automatic tuning is finished.
void ADVi3pp::auto_pid_finished()
{
    pid_tuning.finished();
}

void ADVi3pp::g29_leveling_finished(bool success)
{
    sensor_tuning.g29_leveling_finished(success);
}

//! Store presets in permanent memory.
//! @param write Function to use for the actual writing
//! @param eeprom_index
//! @param working_crc
void ADVi3pp::write(eeprom_write write, int& eeprom_index, uint16_t& working_crc)
{
    advi3pp.write(write, eeprom_index, working_crc);
}

//! Restore presets from permanent memory.
//! @param read Function to use for the actual reading
//! @param eeprom_index
//! @param working_crc
void ADVi3pp::read(eeprom_read read, int& eeprom_index, uint16_t& working_crc)
{
    advi3pp.read(read, eeprom_index, working_crc);
}

//! Reset presets.
void ADVi3pp::reset()
{
    advi3pp.reset();
}

//! Return the size of data specific to ADVi3++
uint16_t ADVi3pp::size_of()
{
    return advi3pp.size_of();
}

//! Inform the user that the EEPROM data are not compatible and have been reset
void ADVi3pp::eeprom_settings_mismatch()
{
    advi3pp.eeprom_settings_mismatch();
}

//! Called when a temperature error occurred and display the error on the LCD.
void ADVi3pp::temperature_error(const FlashChar* message)
{
    advi3pp.temperature_error(message);
}

bool ADVi3pp::is_thermal_protection_enabled()
{
    return advi3pp.is_thermal_protection_enabled();
}

void ADVi3pp::process_command()
{
    advi3pp.process_command();
}

void ADVi3pp::set_brightness(int16_t britghness)
{
    advi3pp.set_brightness(britghness);
}

bool ADVi3pp::has_status()
{
    return advi3pp.has_status();
}

void ADVi3pp::set_status(const char* message)
{
    advi3pp.set_status(message);
}

void ADVi3pp::set_status(const FlashChar* message)
{
    advi3pp.set_status(message);
}

void  ADVi3pp::set_status(const char * const fmt, va_list& args)
{
    advi3pp.set_status(fmt, args);
}

void  ADVi3pp::advanced_pause_show_message(AdvancedPauseMessage message)
{
    advi3pp.advanced_pause_show_message(message);
}

void ADVi3pp::reset_status()
{
    advi3pp.reset_status();
}

void ADVi3pp::buzz(long duration, uint16_t frequency)
{
    advi3pp.buzz(duration, frequency);
}

// --------------------------------------------------------------------

}
