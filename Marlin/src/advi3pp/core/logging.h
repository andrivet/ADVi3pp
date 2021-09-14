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
#ifndef ADV_UNIT_TESTS
#include "../../core/serial.h"
#else
#include "serial-out.h"
#endif
#include "flash_char.h"


namespace ADVi3pp {

enum class LogState { Start, Continue };

#ifdef ADV_UNIT_TESTS
struct log_exception: std::exception {};
#endif

#ifdef ADVi3PP_LOG

// --------------------------------------------------------------------
// NoLogging
// --------------------------------------------------------------------

struct NoFrameLogging
{
    NoFrameLogging();
    ~NoFrameLogging();

    void allow();

private:
    bool suspend_ = false;
};

// --------------------------------------------------------------------
// Log
// --------------------------------------------------------------------

struct Log
{
    Log(bool enabled);

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

    static Log& log(LogState state = LogState::Start);
    static Log& error();
    static Log& frame(LogState state = LogState::Continue);
    static EndOfLine endl() { return EndOfLine{}; }
    void dump(const uint8_t* bytes, size_t size = 1, bool separator = true);

private:
    static Log logging_;
    static Log frame_logging_;
    bool enabled_ = true;
    bool suspend_ = false;

    friend NoFrameLogging;
};

// --------------------------------------------------------------------
// Log
// --------------------------------------------------------------------

inline Log::Log(bool enabled): enabled_{enabled} {
}

template<typename T, size_t S>
Log& Log::operator<<(adv::array<T, S> data) {
    write(data.data(), S);
    return *this;
}

// --------------------------------------------------------------------
// NoFrameLogging
// --------------------------------------------------------------------

inline NoFrameLogging::NoFrameLogging()
: suspend_{Log::frame_logging_.suspend_} {
#ifndef ADVi3PP_LOG_ALL_FRAMES
    Log::frame_logging_.suspend_ = true;
#endif
}

inline NoFrameLogging::~NoFrameLogging() {
    allow();
}

inline void NoFrameLogging::allow() {
#ifndef ADVi3PP_LOG_ALL_FRAMES
    Log::frame_logging_.suspend_ = suspend_;
#endif
}

// --------------------------------------------------------------------

#ifndef assert
void assert_(const char *msg, const char *file, uint16_t line);
#define assert(E) (void)((E) || (ADVi3pp::assert_(#E, __FILE__, __LINE__), 0))
#endif

void debug_break();

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
    static Log& error();
    static Log& frame(LogState state = LogState::Continue) { return log(); }
    static EndOfLine endl() { return EndOfLine{}; }
    static void dump(const uint8_t*, size_t) {}

#ifdef ADVi3PP_UNIT_TEST
    Log& operator<<(unsigned long data) { return log(); }
#endif
};

struct NoFrameLogging
{
    NoFrameLogging() {}
    ~NoFrameLogging() {}

    void allow() {}
};

inline Log& Log::error() {
#ifdef ADV_UNIT_TESTS
    throw log_exception();
#endif
    return log();
}

#ifndef assert
#define assert(E) (void)(false)
#endif

inline void debug_break() {}

#endif

}