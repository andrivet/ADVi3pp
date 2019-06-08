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
#ifndef ADV_I3_PLUS_PLUS_LOG_H
#define ADV_I3_PLUS_PLUS_LOG_H

#ifdef __CLION_IDE__
#define ADVi3PP_LOG
#endif

#include <stdint.h>
#include <stddef.h>
class __FlashStringHelper;
using FlashChar = __FlashStringHelper;

namespace advi3pp {

// --------------------------------------------------------------------
// Logging
// --------------------------------------------------------------------

#if defined(ADVi3PP_LOG) && !defined(ADVi3PP_UNIT_TEST)

struct Log
{
    struct EndOfLine {};

    Log& operator<<(const char* data);
    Log& operator<<(const FlashChar* data);
    Log& operator<<(uint8_t data);
    Log& operator<<(uint16_t data);
    Log& operator<<(uint32_t data);
    Log& operator<<(double data);
    Log& operator<<(EndOfLine eol);

    static Log& log();
    static Log& error();
    inline static EndOfLine endl() { return EndOfLine{}; }
    static void dump(const uint8_t* bytes, size_t size);

private:
    static Log logging_;
};

void assert_(const char *msg, const char *file, uint16_t line);
#define assert(E) (void)((E) || (advi3pp::assert_(#E, __FILE__, __LINE__), 0))

#else
struct Log
{
    struct EndOfLine {};

    inline Log& operator<<(const char* data) { return log(); }
    inline Log& operator<<(const FlashChar* data) { return log(); }
    inline Log& operator<<(uint8_t data) { return log(); }
    inline Log& operator<<(uint16_t data) { return log(); }
    inline Log& operator<<(uint32_t data) { return log(); }
    inline Log& operator<<(double data) { return log(); }
    inline void operator<<(EndOfLine eol) {};

    inline static Log& log() { static Log log; return log; }
    inline static Log& error() { return log(); }
    inline static EndOfLine endl() { return EndOfLine{}; }
    inline static void dump(const uint8_t* bytes, size_t size) {}

#ifdef ADVi3PP_UNIT_TEST
    inline Log& operator<<(unsigned long data) { return log(); }
#endif

};

#define assert(E) (void)(false)

#endif

// --------------------------------------------------------------------

}

#endif //ADV_I3_PLUS_PLUS_LOG_H
