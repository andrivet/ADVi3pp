/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2025 Sebastien Andrivet [https://github.com/andrivet/]
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
#include "../../inc/MarlinConfig.h"
#include "../lib/ADVstd/array.h"
#include "../lib/ADVstd/bitmasks.h"
#include "flash_char.h"
#include "enums.h"

#ifndef ADVi3PP_LOG
#define ADVi3PP_LOG 0
#endif

namespace ADVi3pp {

  struct SuspendLogging {
    SuspendLogging() { suspend(); }
    ~SuspendLogging() { resume(); }

    static void resume();

  private:
    static void suspend();
  };

  template<typename T, size_t S>
  struct Stack;

  struct EndOfLine {};
  struct Decimal {};
  struct Hexadecimal {};

  struct Logger {
    #if ADVi3PP_LOG > 0
    const Logger& operator<<(const FlashChar* data) const;
    const Logger& operator<<(const char* data) const;
    const Logger& operator<<(uint8_t data) const;
    const Logger& operator<<(uint16_t data) const;
    const Logger& operator<<(uint32_t data) const;
    const Logger& operator<<(int8_t data) const;
    const Logger& operator<<(int16_t data) const;
    const Logger& operator<<(int32_t data) const;
    const Logger& operator<<(double data) const;
    template<typename T, size_t S>
    const Logger& operator<<(const adv::array<T, S> &data) const;
    void operator<<(const EndOfLine& eol) const;
    const Logger& operator<<(const Decimal& dec) const;
    const Logger& operator<<(const Hexadecimal& hex) const;
    const Logger& operator<<(Page page) const;
    template<typename T, size_t S>
    const Logger& operator<<(const Stack<T, S> &stack) const;

    static void write(const uint8_t* data, size_t size);
    #else
    const Logger& operator<<(const FlashChar* data) const { return *this; }
    const Logger& operator<<(const char* data) const { return *this; }
    const Logger& operator<<(uint8_t data) const { return *this; }
    const Logger& operator<<(uint16_t data) const { return *this; }
    const Logger& operator<<(uint32_t data) const { return *this; }
    const Logger& operator<<(int8_t data) const { return *this; }
    const Logger& operator<<(int16_t data) const { return *this; }
    const Logger& operator<<(int32_t data) const { return *this; }
    const Logger& operator<<(double data) const { return *this; }
    template<typename T, size_t S>
    const Logger& operator<<(const adv::array<T, S> &data) const { return *this; }
    void operator<<(const EndOfLine& eol) const { }
    const Logger& operator<<(const Decimal& dec) const { return *this; }
    const Logger& operator<<(const Hexadecimal& hex) const { return *this; }
    const Logger& operator<<(Page page) const { return *this; }
    template<typename T, size_t S>
    const Logger& operator<<(const Stack<T, S> &stack) const { return *this; }

    static void write(const uint8_t* data, size_t size) {}
    #endif
  };

  namespace Log {
    Logger error();
    Logger warning();
    Logger info();
    Logger verbose(bool start = false);
    Decimal decimal();
    Hexadecimal hexadecimal();
    EndOfLine endl();
  }

  #if ADVi3PP_LOG > 0
  namespace internals {
    void write_error();
    void write_warning();
    void write_info();
    void write_verbose(bool start);
    void write(const FlashChar* data);
    void write(const char* data);
    void write(uint8_t data);
    void write(uint16_t data);
    void write(uint32_t data);
    void write(int8_t data);
    void write(int16_t data);
    void write(int32_t data);
    void write(double data);
    void write_eol();
    void write_decimal();
    void write_hexadecimal();
    void write(const uint8_t* data, size_t size);
    void dump(const uint8_t* bytes, size_t size = 1, bool separator = true);
    void suspend_until_endl();
  }
  #endif

  void invalid_key_code(Page page, uint16_t key_code);

  #if ADVi3PP_LOG <= 0
  inline void invalid_key_code(Page page, uint16_t key_code) {}
  #endif

  namespace Log {
    #if ADVi3PP_LOG <= 0
    inline Logger error() { return Logger{}; }
    inline Logger warning() { return Logger{}; }
    inline Logger info() {return Logger{}; }
    inline Logger verbose(bool) { return Logger{}; }
    #else
    inline Logger error() { internals::write_error(); return Logger{}; }
    #if ADVi3PP_LOG > 1
    inline Logger warning() { internals::write_warning(); return Logger{}; }
    #else
    inline Logger warning() { internals::suspend_until_endl(); return Logger{}; }
    #endif
    #if ADVi3PP_LOG > 2
    inline Logger info() { internals::write_info(); return Logger{}; }
    #else
    inline Logger info() { internals::suspend_until_endl(); return Logger{}; }
    #endif
    #if ADVi3PP_LOG > 3
    inline Logger verbose(bool start) { internals::write_verbose(start); return Logger{}; }
    #else
    inline Logger verbose(bool) { internals::suspend_until_endl(); return Logger{}; }
    #endif
    #endif

    inline Decimal decimal() { return Decimal{}; }
    inline Hexadecimal hexadecimal() { return Hexadecimal{}; }
    inline EndOfLine endl() { return EndOfLine{}; }
  }

  #if ADVi3PP_LOG > 0
  inline const Logger& Logger::operator<<(const FlashChar* data) const {
    internals::write(data);
    return *this;
  }

  inline const Logger& Logger::operator<<(const char* data) const {
    internals::write(data);
    return *this;
  }

  inline const Logger& Logger::operator<<(Page page) const {
    *this << Log::decimal() << static_cast<uint8_t>(page) << Log::hexadecimal();
    return *this;
  }

  inline const Logger& Logger::operator<<(uint8_t data) const {
    internals::write(data);
    return *this;
  }

  inline const Logger& Logger::operator<<(uint16_t data) const {
    internals::write(data);
    return *this;
  }

  inline const Logger& Logger::operator<<(uint32_t data) const {
    internals::write(data);
    return *this;
  }

  inline const Logger& Logger::operator<<(int8_t data) const {
    internals::write(data);
    return *this;
  }

  inline const Logger& Logger::operator<<(int16_t data) const {
    internals::write(data);
    return *this;
  }

  inline const Logger& Logger::operator<<(int32_t data) const {
    internals::write(data);
    return *this;
  }

  inline const Logger& Logger::operator<<(double data) const {
    internals::write(data);
    return *this;
  }

  template<typename T, size_t S>
  const Logger& Logger::operator<<(const adv::array<T, S> &data) const {
    write(data.data(), S);
    return *this;
  }

  inline void Logger::operator<<(const EndOfLine&) const {
    internals::write_eol();
  }

  inline const Logger& Logger::operator<<(const Decimal&) const {
    internals::write_decimal();
    return *this;
  }

  inline const Logger& Logger::operator<<(const Hexadecimal&) const {
    internals::write_hexadecimal();
    return *this;
  }

  template<typename T, size_t S>
  const Logger& Logger::operator<<(const Stack<T, S> &stack) const {
    if(stack.is_empty())
      *this << F("<empty>");
    for(size_t i = 0; i < stack.size(); ++i)
      *this << stack[i];
    return *this;
  }

  inline void Logger::write(const uint8_t* data, size_t size) {
    internals::write(data, size);
  }
  #endif

  #if ADVi3PP_LOG >= 5 || ADVi3PP_LOG == 0
  inline void SuspendLogging::suspend() {}
  inline void SuspendLogging::resume() {}
  #endif

  #ifndef assert
  #if ADVi3PP_LOG > 0
  void assert_(const FlashChar*msg, const FlashChar *file, uint16_t line);
  #define assert(E) (void)((E) || (ADVi3pp::assert_(F(#E), F(__FILE__), __LINE__), 0))
  #else
  #define assert(E) (void)(false)
  #endif
  #endif

  #if ADVi3PP_LOG > 0
  void debug_break();
  #else
  inline void debug_break() {}
  #endif
}
