/*
  Copyright (c) 2011 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdio.h>
#include <string.h>

char* ltoa(long value, char* string, int radix)
{
    if(string == nullptr)
        return nullptr;
    if (radix > 36 || radix <= 1)
        return nullptr;

    int sign = (radix == 10 && value < 0);
    unsigned long v = sign ? static_cast<unsigned long>(-value) : value;

    char tmp[33];
    char* tp = tmp;
    while(v || tp == tmp)
    {
        long i = v % radix;
        v = v / radix;
        if (i < 10)
            *tp++ = static_cast<char>(i + '0');
        else
            *tp++ = static_cast<char>(i + 'a' - 10);
    }

    char* sp = string;
    if(sign)
        *sp++ = '-';
    while(tp > tmp)
        *sp++ = *--tp;
    *sp = 0;
    return string;
}

char* itoa(int value, char *string, int radix)
{
    return ltoa(value, string, radix);
}

char* ultoa(unsigned long value, char* string, int radix)
{
    if(string == nullptr)
        return nullptr;
    if(radix > 36 || radix <= 1)
        return nullptr;

    char tmp[33];
    char* tp = tmp;
    unsigned long v = value;
    while (v || tp == tmp)
    {
        long i = v % radix;
        v = v / radix;
        if (i < 10)
            *tp++ = static_cast<char>(i+'0');
        else
            *tp++ = static_cast<char>(i + 'a' - 10);
    }

    char* sp = string;
    while (tp > tmp)
        *sp++ = *--tp;
    *sp = 0;
    return string;
}

char* utoa(unsigned long value, char *string, int radix)
{
    return ultoa(value, string, radix);
}

char* dtostrf(double val, signed char width, unsigned char prec, char *sout)
{
    char fmt[20];
    sprintf(fmt, "%%%d.%df", width, prec);
    sprintf(sout, fmt, val);
    return sout;
}
