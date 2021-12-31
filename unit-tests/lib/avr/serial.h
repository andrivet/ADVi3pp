/**
 * ADVi3++ Unit Tests
 *
 * Copyright (C) 2018-2021 Sebastien Andrivet [https://github.com/andrivet/]
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
#include <string.h>
#include <stdio.h>
#include <vector>
#include "macros.h"


struct SerialBase {

  void begin(unsigned long baud) {}

  size_t write(uint8_t);
  size_t write(const uint8_t *buffer, size_t size);
  size_t write(const char *buffer, size_t size);

  int read();
  size_t readBytes(uint8_t *buffer, size_t length);

  size_t available() const {  return input_size_ - input_position_; }
  size_t written() const { return output_position_; }

  void println(const char *message);

  template<size_t S>
  void reset(const uint8_t (&buffer)[S]) {
      input_position_ = 0;
      input_size_ = S;
      memcpy(input_, buffer, S);
      output_position_ = 0;
  }

  void reset() {
      input_position_ = 0;
      input_size_ = 0;
      output_position_ = 0;
  }

  const std::vector<uint8_t> get_written_bytes() const { return std::vector<uint8_t>(output_, output_ + output_position_); }

private:
  static const size_t BUFFER_SIZE = 256;

  size_t input_size_ = 0;
  size_t input_position_ = 0;
  size_t output_position_ = 0;
  uint8_t input_[BUFFER_SIZE];
  uint8_t output_[BUFFER_SIZE];
};

extern SerialBase Serial2;
extern SerialBase Serial;

using HardwareSerial = SerialBase;

