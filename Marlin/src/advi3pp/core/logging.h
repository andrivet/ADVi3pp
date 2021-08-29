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

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "../lib/ADVstd/array.h"
#include "../../core/serial.h"
#include "flash_char.h"


namespace ADVi3pp {

#if defined(ADVi3PP_LOG) && !defined(ADVi3PP_UNIT_TEST)


// --------------------------------------------------------------------
// Log
// --------------------------------------------------------------------

struct Log
{
    struct EndOfLine {};

    Log& operator<<(const char* data);
    Log& operator<<(const FlashChar* data);
    Log& operator<<(uint8_t data);
    Log& operator<<(uint16_t data);
    Log& operator<<(uint32_t data);
    Log& operator<<(double data);
    template<typename T, size_t S> Log& operator<<(adv::array<T, S> data);
    Log& write(const uint8_t* data, size_t size);
    Log& operator<<(EndOfLine eol);
    void enable();
    void disable();

    static Log& log();
    static Log& error();
    static Log& cont();
    static EndOfLine endl() { return EndOfLine{}; }
    void dump(const uint8_t* bytes, size_t size, bool separator = true);

private:
    static Log logging_;
    bool enabled_ = true;
};

// --------------------------------------------------------------------
// NoLogging
// --------------------------------------------------------------------

struct NoLogging
{
    NoLogging();
    ~NoLogging();
};


// --------------------------------------------------------------------
// Log
// --------------------------------------------------------------------

inline Log& Log::cont() {
    return logging_;
}

template<typename T, size_t S>
Log& Log::operator<<(adv::array<T, S> data) {
    write(data.data(), S);
    return *this;
}

inline void Log::enable() {
  enabled_ = true;
}

inline void Log::disable() {
    enabled_ = false;
}

// --------------------------------------------------------------------
// NoLogging
// --------------------------------------------------------------------

inline NoLogging::NoLogging() {
#ifndef ADVi3PP_LOG_ALL_FRAMES
    Log::cont().disable();
#endif
}

inline NoLogging::~NoLogging() {
#ifndef ADVi3PP_LOG_ALL_FRAMES
    Log::cont().enable();
#endif
}

// --------------------------------------------------------------------

void assert_(const char *msg, const char *file, uint16_t line);
#define assert(E) (void)((E) || (ADVi3pp::assert_(#E, __FILE__, __LINE__), 0))

#else
struct Log
{
    struct EndOfLine {};

    Log& operator<<(const char*) { return log(); }
    Log& operator<<(const FlashChar*) { return log(); }
    Log& operator<<(uint8_t) { return log(); }
    Log& operator<<(uint16_t) { return log(); }
    Log& operator<<(uint32_t) { return log(); }
    Log& operator<<(double) { return log(); }
    template<typename T, size_t S> Log& operator<<(adv::array<T, S> data) { return log(); }
    Log& write(const uint8_t* data, size_t size) { return log(); }
    void operator<<(EndOfLine) {};

    static Log& log() { static Log log; return log; }
    static Log& error() { return log(); }
    static Log cont() { return log(); }
    static EndOfLine endl() { return EndOfLine{}; }
    static void dump(const uint8_t*, size_t) {}

#ifdef ADVi3PP_UNIT_TEST
    Log& operator<<(unsigned long data) { return log(); }
#endif
};

struct NoLogging
{
    NoLogging() {}
    ~NoLogging() {}
};

#define assert(E) (void)(false)

#endif

}