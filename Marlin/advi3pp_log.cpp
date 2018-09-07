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

#include "advi3pp_defines.h"
#include "advi3pp_log.h"

#include <HardwareSerial.h>
#include "serial.h"

namespace advi3pp {

// --------------------------------------------------------------------
// Logging
// --------------------------------------------------------------------

#ifdef ADVi3PP_LOG

Log Log::logging_;

Log& Log::error()
{
    log() << F("### ERROR: ");
    return log();
}

Log& Log::operator<<(const String& data)
{
    SERIAL_ECHO(data.c_str());
    return log();
}

Log& Log::operator<<(uint8_t data)
{
    SERIAL_ECHO_F(data, HEX);
    return log();
}

Log& Log::operator<<(uint16_t data)
{
    SERIAL_ECHO_F(data, HEX);
    return log();
}

Log& Log::operator<<(uint32_t data)
{
    SERIAL_ECHO_F(data, HEX);
    return log();
}

Log& Log::operator<<(double data)
{
    SERIAL_ECHO(data);
    return log();
}

void Log::operator<<(EndOfLine)
{
    SERIAL_ECHOLN("");
}

//! Dump the bytes in hexadecimal and print them (serial)
void Log::dump(const uint8_t* bytes, size_t size)
{
    static const size_t MAX_LENGTH = 20;

    static const char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    if(size > MAX_LENGTH)
        size = MAX_LENGTH;

    // TODO: output one byte at a time
    char buffer[MAX_LENGTH * 3 + 1];
    for(size_t index = 0; index < size; ++index)
    {
        buffer[index * 3 + 0] = digits[bytes[index] / 16];
        buffer[index * 3 + 1] = digits[bytes[index] % 16];
        buffer[index * 3 + 2] = ' ';
    }
    buffer[size * 3] = 0;

    SERIAL_ECHOLN(buffer);
}

void __assert(const char *msg, const char *file, uint16_t line)
{
    asm("break \n");
    Log::log() << F("### ASSERTION FAILED: ") << msg << " in file " << file << ", line " << line << Log::endl();
}

#endif

// --------------------------------------------------------------------

}
