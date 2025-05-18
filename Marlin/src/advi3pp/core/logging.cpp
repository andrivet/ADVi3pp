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

#include "../../inc/MarlinConfig.h"
#include "logging.h"

#if ADVi3PP_LOG > 0

namespace ADVi3pp::internals {
  enum class MODE: uint8_t { NORMAL = 0x00, SUSPENDED_UNTIL_RESUME = 0x01, SUSPENDED_UNTIL_ENDL = 0x02, DECIMAL = 0x04 };
  MODE mode{MODE::NORMAL};
}

ENABLE_BITMASK_OPERATOR(ADVi3pp::internals::MODE);

namespace ADVi3pp {

#if ADVi3PP_LOG < 5
  void SuspendLogging::suspend() {
    set_one_bit(internals::mode, internals::MODE::SUSPENDED_UNTIL_RESUME, true);
  }

  void SuspendLogging::resume() {
    set_one_bit(internals::mode, internals::MODE::SUSPENDED_UNTIL_RESUME, false);
  }
#endif
}

namespace ADVi3pp::internals {

  void suspend_until_endl() {
    set_one_bit(internals::mode, internals::MODE::SUSPENDED_UNTIL_ENDL, true);
  }

  inline bool can_log() {
    return !test_one_bit(mode, MODE::SUSPENDED_UNTIL_RESUME | MODE::SUSPENDED_UNTIL_ENDL);
  }

  void space() {
    SERIAL_CHAR(' ');
  }

  #if ADVi3PP_LOG >= 1
  void write_error() {
    internals::write(F("// ERROR:"));
  }
  #endif

  #if ADVi3PP_LOG >= 2
  void write_warning() {
    internals::write(F("// WARNING:"));
  }
  #endif

  #if ADVi3PP_LOG >= 3
  void write_info() {
    internals::write(F("// LOG:"));
  }
  #endif

  #if ADVi3PP_LOG >= 4
  void write_verbose(bool start) {
    if(start) internals::write(F("// VERBOSE:"));
  }
  #endif

  void write(const FlashChar* data) {
    if(!can_log()) return;
    SERIAL_ECHO_P(from_flash(data));
    space();
  }

  void write(const char* data) {
    if(!can_log()) return;
    SERIAL_ECHO(data);
    space();
  }

  void write(uint8_t data) {
    if(!can_log()) return;
    dump(&data);
    space();
  }

  void write(uint16_t data) {
    if(!can_log()) return;
    auto bytes = reinterpret_cast<const uint8_t *>(&data);
    dump(bytes + 1);
    dump(bytes + 0);
    space();
  }

  void write(uint32_t data) {
    if(!can_log()) return;
    auto bytes = reinterpret_cast<const uint8_t *>(&data);
    dump(bytes + 3);
    dump(bytes + 2);
    dump(bytes + 1);
    dump(bytes + 0);
    space();
  }

#if defined(ADV_STD_SIZE_T)
  void write(size_t data) {
    return write(static_cast<uint32_t>(data));
  }
#endif

  void write(int8_t data) {
    write(static_cast<uint8_t>(data));
  }

  void write(int16_t data) {
    write(static_cast<uint16_t>(data));
  }

  void write(int32_t data) {
    write(static_cast<uint32_t>(data));
  }

  void write(double data) {
    if(!can_log()) return;
    SERIAL_ECHO(data);
    space();
  }

  void write_eol() {
    if(can_log()) SERIAL_EOL();
    clear_bits(mode, MODE::DECIMAL | MODE::SUSPENDED_UNTIL_ENDL);
  }

  void write_decimal() {
    set_one_bit(mode, MODE::DECIMAL, true);
  }

  void write_hexadecimal() {
    set_one_bit(mode, MODE::DECIMAL, false);
  }

  void write(const uint8_t* data, size_t size) {
    if(can_log() && size > 0) {
      dump(data, size);
      space();
    }
  }

  char digit(size_t value) {
    return static_cast<char>(value < 10 ?  '0' +  value : ('A' + value - 10));
  }

  void dump(const uint8_t* bytes, size_t size, bool separator) {
    if(!can_log()) return;

    bool decimal = test_one_bit(mode, MODE::DECIMAL);
    for(size_t index = 0; index < size; ++index) {
      if(decimal)
        SERIAL_ECHO(bytes[index]);
      else {
        SERIAL_CHAR(digit(bytes[index] / 16));
        SERIAL_CHAR(digit(bytes[index] % 16));
      }
      if(separator && index < size - 1)
        space();
    }
  }

}

namespace ADVi3pp {

  void invalid_key_code(Page page, uint16_t key_code) {
    Log::error() << F("Invalid key code") << key_code << F("for page") << page << Log::endl();
  }

  void assert_(const FlashChar *msg, const FlashChar *file, uint16_t line) {
    Log::error() << F("ASSERTION FAILED:") << msg << F("in file") << file << F("line") << Log::decimal() << line << Log::endl();
    debug_break();
  }

  void debug_break() {
  #ifdef ADVi3PP_HARD_BREAK
    asm("break \n");
  #endif
  }

}

#endif
