/**
 * ADVstring - String without dynamic allocation and a fixed maximum size
 *
 * Copyright (C) 2018 Sebastien Andrivet [https://github.com/andrivet/]
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

#ifndef ADVSTRING_H
#define ADVSTRING_H

#include <stdlib.h>
#include "Marlin.h"
#include "duration_t.h"

namespace advi3pp {

using FlashChar = __FlashStringHelper;

// --------------------------------------------------------------------
// String without dynamic allocation and a fixed maximum size
// --------------------------------------------------------------------

enum class Base: uint8_t
{
    Decimal = 10,
    Hexadecimal = 16
};

template<size_t L>
struct ADVString
{
    ADVString() = default;
    explicit ADVString(const char* s);
    explicit ADVString(const FlashChar* s);
    explicit ADVString(duration_t d);
    explicit ADVString(int16_t n, Base b = Base::Decimal);
    explicit ADVString(int32_t n, Base b = Base::Decimal);
    explicit ADVString(uint16_t n, Base b = Base::Decimal);
    explicit ADVString(uint32_t n, Base b = Base::Decimal);
    explicit ADVString(double n, uint8_t decimals = 2);

    ADVString& operator=(const char* str) ;
    ADVString& operator=(const FlashChar* str) ;
    template<size_t L2> ADVString& operator=(const ADVString<L2>& str);

    void set(const char* s);
    void set(const char* fmt, va_list& args);
    void set(const FlashChar* s);
    void set(const FlashChar* fmt, va_list& args);
    template<size_t L2> void set(const ADVString<L2>& s);
    void set(duration_t d);
    void set(int16_t n, Base base);
    void set(int32_t n, Base base);
    void set(uint16_t n, Base base);
    void set(uint32_t n, Base base);
    void set(double n, uint8_t decimals = 2);
    template<size_t L2> void set_padded(const ADVString<L2>& s, bool centered = false);
    void reset();

    void append(const char* s);
    void append(const FlashChar* s);
    void append(int16_t n, Base base);
    void append(int32_t n, Base base);
    void append(uint16_t n, Base base);
    void append(uint32_t n, Base base);
    void append(double n, uint8_t decimals = 2);
    void operator+=(const char* s);
    void operator+=(const FlashChar* s);
    void operator+=(int16_t n);
    void operator+=(int32_t n);
    void operator+=(uint16_t n);
    void operator+=(uint32_t n) ;
    void operator+=(double n);

    size_t length() const;
    char operator[](size_t i) const;
    bool is_empty() const;
    const char* get() const;
    bool has_changed(bool reset = true);

private:
    char string_[L + 1] = {};
    bool dirty_ = true;
};

// --------------------------------------------------------------------

template<size_t L> inline ADVString<L>::ADVString(const char* s) { set(s); }
template<size_t L> inline ADVString<L>::ADVString(const FlashChar* s) { set(s); }
template<size_t L> inline ADVString<L>::ADVString(duration_t d) { set(d); }
template<size_t L> inline ADVString<L>::ADVString(int16_t n, Base b) { set(n, b); }
template<size_t L> inline ADVString<L>::ADVString(int32_t n, Base b) { set(n, b); }
template<size_t L> inline ADVString<L>::ADVString(uint16_t n, Base b) { set(n, b); }
template<size_t L> inline ADVString<L>::ADVString(uint32_t n, Base b) { set(n, b); }
template<size_t L> inline ADVString<L>::ADVString(double n, uint8_t decimals) { set(n, decimals); }

template<size_t L> inline ADVString<L>& ADVString<L>::operator=(const char* str)  { set(str); return *this; }
template<size_t L> inline ADVString<L>& ADVString<L>::operator=(const FlashChar* str)  { set(str); return *this; }
template<size_t L> template<size_t L2> inline ADVString<L>& ADVString<L>::operator=(const ADVString<L2>& str) { set(str); return *this; }

template<size_t L> void ADVString<L>::set(const char* s)
{
    strlcpy(string_, s, L + 1);
    dirty_ = true;
}

template<size_t L> void ADVString<L>::set(const char* fmt, va_list& args)
{
    vsnprintf(string_, L + 1, fmt, args);
    dirty_ = true;
}

template<size_t L> void ADVString<L>::set(const FlashChar* s)
{
    strlcpy_P(string_, reinterpret_cast<const char*>(s), L + 1);
    dirty_ = true;
}

template<size_t L> void ADVString<L>::set(const FlashChar* fmt, va_list& args)
{
    vsnprintf_P(string_, L + 1, reinterpret_cast<const char*>(fmt), args);
    dirty_ = true;
}

template<size_t L> template<size_t L2> inline void ADVString<L>::set(const ADVString<L2>& s)
{
    strlcpy(string_, s.get(), L);
    dirty_ = true;
}

template<size_t L> void ADVString<L>::set(duration_t d)
{
    char buffer[22]; // 21 + 1, from the doc
    d.toString(buffer);
    set(buffer);
}

template<size_t L> void ADVString<L>::set(int16_t n, Base base)
{
    char buffer[2 + 8 * sizeof(int16_t)];
    itoa(n, buffer, static_cast<int>(base));
    set(buffer);
}

template<size_t L> void ADVString<L>::set(int32_t n, Base base)
{
    char buffer[2 + 8 * sizeof(int32_t)];
    ltoa(n, buffer, static_cast<int>(base));
    set(buffer);
}

template<size_t L> void ADVString<L>::set(uint16_t n, Base base)
{
    char buffer[1 + 8 * sizeof(uint16_t)];
    utoa(n, buffer, static_cast<int>(base));
    set(buffer);
}

template<size_t L> void ADVString<L>::set(uint32_t n, Base base)
{
    char buffer[1 + 8 * sizeof(uint16_t)];
    ultoa(n, buffer, static_cast<int>(base));
    set(buffer);
}

template<size_t L> void ADVString<L>::set(double n, uint8_t decimals)
{
    char buffer[33];
    dtostrf(n, decimals + 2, decimals, buffer);
    set(buffer);
}

template<size_t L>
template<size_t L2>
void ADVString<L>::set_padded(const ADVString<L2>& s, bool centered)
{
    auto l = s.length();

    if(centered)
    {
        size_t pad = (L - l) / 2;
        for(size_t i = 0; i < pad; ++i)
            string_[i] = ' ';
        l += pad;
    }

    append(s);

    for(; l < L; ++l)
        string_[l] = ' ';
    string_[L] = 0;
}


template<size_t L> inline void ADVString<L>::reset() { string_[0] = 0; dirty_ = true; }

template<size_t L> inline void ADVString<L>::operator+=(const char* s) { append(s); }
template<size_t L> inline void ADVString<L>::operator+=(const FlashChar* s) { append(s); }
template<size_t L> inline void ADVString<L>::operator+=(int16_t n) { append(n); }
template<size_t L> inline void ADVString<L>::operator+=(int32_t n) { append(n); }
template<size_t L> inline void ADVString<L>::operator+=(uint16_t n) { append(n); }
template<size_t L> inline void ADVString<L>::operator+=(uint32_t n) { append(n); }
template<size_t L> inline void ADVString<L>::operator+=(double n) { append(n); }

template<size_t L> void ADVString<L>::append(const char* s)
{
    strlcat(string_, s, L + 1);
    dirty_ = true;
}

template<size_t L> void ADVString<L>::append(const FlashChar* s)
{
    strlcat_P(string_, reinterpret_cast<const char*>(s), L + 1);
    dirty_ = true;
}

template<size_t L> void ADVString<L>::append(int16_t n, Base base)
{
    char buffer[2 + 8 * sizeof(int16_t)];
    itoa(n, buffer, static_cast<int>(base));
    append(buffer);
}

template<size_t L> void ADVString<L>::append(int32_t n, Base base)
{
    char buffer[2 + 8 * sizeof(int32_t)];
    ltoa(n, buffer, static_cast<int>(base));
    append(buffer);
}

template<size_t L> void ADVString<L>::append(uint16_t n, Base base)
{
    char buffer[1 + 8 * sizeof(uint16_t)];
    utoa(n, buffer, static_cast<int>(base));
    append(buffer);
}

template<size_t L> void ADVString<L>::append(uint32_t n, Base base)
{
    char buffer[1 + 8 * sizeof(uint16_t)];
    ultoa(n, buffer, static_cast<int>(base));
    append(buffer);
}

template<size_t L> void ADVString<L>::append(double n, uint8_t decimals)
{
    char buffer[33];
    dtostrf(n, decimals + 2, decimals, buffer);
    append(buffer);
}

template<size_t L> inline size_t ADVString<L>::length() const { return lstrlen(string_); }
template<size_t L> inline char ADVString<L>::operator[](size_t i) const { return string_[i]; }
template<size_t L> inline bool ADVString<L>::is_empty() const { return string_[0] == 0; }
template<size_t L> inline const char* ADVString<L>::get() const { return string_; }

template<size_t L> bool ADVString<L>::has_changed(bool reset)
{
    auto dirty = dirty_;
    if(reset)
        dirty_ = false;
    return dirty;
}


// --------------------------------------------------------------------

template<size_t L, typename T>
inline ADVString<L>& operator<<(ADVString<L>& rhs, T lhs) { rhs += lhs; return rhs; }

// --------------------------------------------------------------------

}

#endif // ADVSTRING_H
