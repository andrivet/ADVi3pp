/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2020 Sebastien Andrivet [https://github.com/andrivet/]
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

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/ADVstd/array.h"
#ifndef ADV_UNIT_TESTS
#include "flash_char.h"
#endif

namespace ADVi3pp {

// --------------------------------------------------------------------
// String without dynamic allocation and a fixed maximum size
// --------------------------------------------------------------------

enum class Base: uint8_t {
  Decimal = 10,
  Hexadecimal = 16
};

template<size_t L>
struct ADVString {
  ADVString() = default;
  template<size_t L2> explicit ADVString(const ADVString<L2>& str);
  explicit ADVString(const char* s);
  explicit ADVString(const FlashChar* s);
  explicit ADVString(char c);
  explicit ADVString(int16_t n, Base b = Base::Decimal);
  explicit ADVString(int32_t n, Base b = Base::Decimal);
  explicit ADVString(uint16_t n, Base b = Base::Decimal);
  explicit ADVString(uint32_t n, Base b = Base::Decimal);
  explicit ADVString(double n, uint8_t decimals = 2);

  ADVString& operator=(const char* str);
  ADVString& operator=(const FlashChar* str);
  ADVString& operator=(char c);
  template<size_t L2> ADVString& operator=(const ADVString<L2>& str);

  ADVString& set(const char* s);
  ADVString& set(const char* fmt, va_list& args);
  ADVString& set(const FlashChar* s);
  ADVString& set(const FlashChar* fmt, va_list& args);
  template<size_t L2> ADVString& set(const ADVString<L2>& s);
  ADVString& set(char c);
  ADVString& set(int16_t n, Base base = Base::Decimal);
  ADVString& set(int32_t n, Base base = Base::Decimal);
  ADVString& set(uint16_t n, Base base = Base::Decimal);
  ADVString& set(uint32_t n, Base base = Base::Decimal);
  ADVString& set(double n, uint8_t decimals = 2);
  ADVString& reset();

  ADVString& format(const char* fmt, ...);
  ADVString& format(const FlashChar* fmt, ...);

  ADVString& append(const char* s);
  ADVString& append(const FlashChar* s);
  ADVString& append(char c);
  ADVString& append(int16_t n, Base base = Base::Decimal);
  ADVString& append(int32_t n, Base base = Base::Decimal);
  ADVString& append(uint16_t n, Base base = Base::Decimal);
  ADVString& append(uint32_t n, Base base = Base::Decimal);
  ADVString& append(double n, uint8_t decimals = 2);
  void operator+=(const char* s);
  void operator+=(const FlashChar* s);
  void operator+=(char c);

  size_t length() const;
  char operator[](size_t i) const;
  bool is_empty() const;
  const char* get() const;

private:
  adv::array<char, L+1> string_{};
};

// --------------------------------------------------------------------

template<size_t L, typename T>
inline ADVString<L>& operator<<(ADVString<L>& rhs, T lhs) { rhs.append(lhs); return rhs; }

// --------------------------------------------------------------------

template<size_t L> template<size_t L2> inline ADVString<L>::ADVString(const ADVString<L2>& str) { set(str); }
template<size_t L> inline ADVString<L>::ADVString(const char* s) { set(s); }
template<size_t L> inline ADVString<L>::ADVString(const FlashChar* s) { set(s); }
template<size_t L> inline ADVString<L>::ADVString(const char c) { set(c); }
template<size_t L> inline ADVString<L>::ADVString(int16_t n, Base b) { set(n, b); }
template<size_t L> inline ADVString<L>::ADVString(int32_t n, Base b) { set(n, b); }
template<size_t L> inline ADVString<L>::ADVString(uint16_t n, Base b) { set(n, b); }
template<size_t L> inline ADVString<L>::ADVString(uint32_t n, Base b) { set(n, b); }
template<size_t L> inline ADVString<L>::ADVString(double n, uint8_t decimals) { set(n, decimals); }

template<size_t L> inline ADVString<L>& ADVString<L>::operator=(const char* str)  { set(str); return *this; }
template<size_t L> inline ADVString<L>& ADVString<L>::operator=(const FlashChar* str)  { set(str); return *this; }
template<size_t L> inline ADVString<L>& ADVString<L>::operator=(const char c)  { set(c); return *this; }
template<size_t L> template<size_t L2> inline ADVString<L>& ADVString<L>::operator=(const ADVString<L2>& str) { set(str); return *this; }

template<size_t L> template<size_t L2> ADVString<L>& ADVString<L>::set(const ADVString<L2>& s) {
  strlcpy(string_.data(), s.get(), L + 1);
  return *this;
}

template<size_t L>
inline ADVString<L>& ADVString<L>::set(const char* s) {
  strlcpy(string_.data(), s, L + 1);
  return *this;
}

template<size_t L>
ADVString<L>& ADVString<L>::set(const char c) {
  if(L < 1)
    return *this;
  string_[0] = c;
  string_[1] = 0;

  return *this;
}

template<size_t L>
ADVString<L>& ADVString<L>::set(const char* fmt, va_list& args) {
  vsnprintf(string_.data(), L + 1, fmt, args);
  return *this;
}

#ifndef ADVi3PP_UNIT_TEST

template<size_t L>
inline ADVString<L>& ADVString<L>::set(const FlashChar* s) {
  strlcpy_P(string_.data(), reinterpret_cast<const char*>(s), L + 1);
  return *this;
}

template<size_t L>
ADVString<L>& ADVString<L>::set(const FlashChar* fmt, va_list& args) {
  vsnprintf_P(string_.data(), L + 1, reinterpret_cast<const char*>(fmt), args);
  return *this;
}

#endif

template<size_t L>
ADVString<L>& ADVString<L>::set(int16_t n, Base base) {
  char buffer[2 + 8 * sizeof(int16_t)];
  itoa(n, buffer, static_cast<int>(base));
  return set(buffer);
}

template<size_t L>
ADVString<L>& ADVString<L>::set(int32_t n, Base base) {
  char buffer[2 + 8 * sizeof(int32_t)];
  ltoa(n, buffer, static_cast<int>(base));
  return set(buffer);
}

template<size_t L>
ADVString<L>& ADVString<L>::set(uint16_t n, Base base) {
  char buffer[1 + 8 * sizeof(uint16_t)];
  utoa(n, buffer, static_cast<int>(base));
  return set(buffer);
}

template<size_t L>
ADVString<L>& ADVString<L>::set(uint32_t n, Base base) {
  char buffer[1 + 8 * sizeof(uint16_t)];
  ultoa(n, buffer, static_cast<int>(base));
  return set(buffer);
}

template<size_t L>
ADVString<L>& ADVString<L>::set(double n, uint8_t decimals) {
  char buffer[33];
  dtostrf(n, decimals + 2, decimals, buffer);
  return set(buffer);
}

template<size_t L>
inline ADVString<L>& ADVString<L>::reset() { string_[0] = 0; return *this; }

template<size_t L>
ADVString<L>& ADVString<L>::format(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  set(fmt, args);
  va_end(args);
  return *this;
}

template<size_t L>
ADVString<L>& ADVString<L>::format(const FlashChar* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  set(fmt, args);
  va_end(args);
  return *this;
}

template<size_t L> inline void ADVString<L>::operator+=(const char* s) { append(s); }
template<size_t L> inline void ADVString<L>::operator+=(const FlashChar* s) { append(s); }
template<size_t L> inline void ADVString<L>::operator+=(char c) { append(c); }

template<size_t L>
ADVString<L>& ADVString<L>::append(const char* s) {
  strlcat(string_.data(), s, L + 1);
  return *this;
}

template<size_t L>
ADVString<L>& ADVString<L>::append(char c) {
  auto l = length();
  if(l >= L)
    return *this;

  string_[l] = c;
  string_[l + 1] = 0;
  return *this;
}

template<size_t L>
ADVString<L>& ADVString<L>::append(const FlashChar* s) {
  strlcat_P(string_.data(), reinterpret_cast<const char*>(s), L + 1);
  return *this;
}

template<size_t L>
ADVString<L>& ADVString<L>::append(int16_t n, Base base) {
  char buffer[2 + 8 * sizeof(int16_t)];
  itoa(n, buffer, static_cast<int>(base));
  append(buffer);
  return *this;
}

template<size_t L>
ADVString<L>& ADVString<L>::append(int32_t n, Base base) {
  char buffer[2 + 8 * sizeof(int32_t)];
  ltoa(n, buffer, static_cast<int>(base));
  append(buffer);
  return *this;
}

template<size_t L>
ADVString<L>& ADVString<L>::append(uint16_t n, Base base) {
  char buffer[1 + 8 * sizeof(uint16_t)];
  utoa(n, buffer, static_cast<int>(base));
  append(buffer);
  return *this;
}

template<size_t L>
ADVString<L>& ADVString<L>::append(uint32_t n, Base base) {
  char buffer[1 + 8 * sizeof(uint16_t)];
  ultoa(n, buffer, static_cast<int>(base));
  append(buffer);
  return *this;
}

template<size_t L>
ADVString<L>& ADVString<L>::append(double n, uint8_t decimals) {
  char buffer[33];
  dtostrf(n, decimals + 2, decimals, buffer);
  append(buffer);
  return *this;
}

template<size_t L> inline size_t ADVString<L>::length() const { return strlen(string_.data()); }
template<size_t L> inline char ADVString<L>::operator[](size_t i) const { return string_[i]; }
template<size_t L> inline bool ADVString<L>::is_empty() const { return string_[0] == 0; }
template<size_t L> inline const char* ADVString<L>::get() const { return string_.data(); }


}