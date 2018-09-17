/**
 * ADVi3++ Unit Tests
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

#ifndef UNIT_TESTS_SERIAL_H
#define UNIT_TESTS_SERIAL_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "../../vendors/avr/macros.h"


struct SerialBase
{
    size_t write(const uint8_t *buffer, size_t size) { return size; }

    size_t readBytes(uint8_t *buffer, size_t length);
    int available();
    void println(const char* message);

    template<size_t S>
    void set_serial_data(const uint8_t (&buffer)[S])
    {
        position_ = 0;
        size_ = S;
        buffer_ = buffer;
    }

    int read();

private:
    size_t position_ = 0;
    const uint8_t* buffer_;
    size_t size_;
};

extern SerialBase Serial2;
extern SerialBase Serial;

#endif //UNIT_TESTS_SERIAL_H
