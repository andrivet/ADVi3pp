/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2021 Sebastien Andrivet [https://github.com/andrivet/]
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

#include "../parameters.h"
#include "logging.h"

namespace ADVi3pp {

#ifdef ADVi3PP_LOG

Log Log::logging_;

void space() {
    SERIAL_CHAR(' ');
}

Log& Log::log()
{
    logging_ << F("// LOG:");
    return logging_;
}

Log& Log::error()
{
    logging_ << F("// ERROR:");
    return logging_;
}

Log& Log::operator<<(const char* data)
{
    if(enabled_) {
        SERIAL_ECHO(data);
        space();
    }
    return *this;
}

Log& Log::operator<<(const FlashChar* data)
{
    if(enabled_) {
        serialprintPGM(from_flash(data));
        space();
    }
    return *this;
}

Log& Log::operator<<(uint8_t data)
{
    if(enabled_) {
        dump(&data, 1);
        space();
    }
    return *this;
}

Log& Log::operator<<(uint16_t data)
{
    if(enabled_) {
        dump(reinterpret_cast<const uint8_t *>(&data), 2, false);
        space();
    }
    return *this;
}

Log& Log::operator<<(uint32_t data)
{
    if(enabled_) {
        dump(reinterpret_cast<const uint8_t *>(&data), 4, false);
        space();
    }
    return *this;
}

Log& Log::operator<<(double data)
{
    if(enabled_) {
        SERIAL_ECHO(data);
        space();
    }
    return *this;
}

Log& Log::operator<<(EndOfLine)
{
    if(enabled_)
        SERIAL_EOL();
    return *this;
}

Log& Log::write(const uint8_t* data, size_t size)
{
    if(enabled_) {
        dump(data, size);
        space();
    }
    return *this;
}

//! Dump the bytes in hexadecimal and print them (serial)
void Log::dump(const uint8_t* bytes, size_t size, bool separator)
{
    static const char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    if(!enabled_)
        return;

    for(size_t index = 0; index < size; ++index)
    {
        SERIAL_CHAR(digits[bytes[index] / 16]);
        SERIAL_CHAR(digits[bytes[index] % 16]);
        if(separator && index < size - 1)
            space();
    }
}

void assert_(const char *msg, const char *file, uint16_t line)
{
    Log::error() << F("ASSERTION FAILED:") << msg << "in file" << file << "line" << line << Log::endl();
    asm("break \n");
}

#endif

}