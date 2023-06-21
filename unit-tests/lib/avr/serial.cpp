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

#include "serial.h"

SerialBase Serial2;
SerialBase Serial;

size_t SerialBase::write(uint8_t value) {
  assert(output_position_ < BUFFER_SIZE);
  output_[output_position_++] = value;
  return 1;
}

size_t SerialBase::write(const uint8_t *buffer, size_t size) {
  assert(output_position_ + size <= BUFFER_SIZE);
  memcpy(output_ + output_position_, buffer, size);
  output_position_ += size;
  return size;
}

size_t SerialBase::write(const char *buffer, size_t size) {
  assert(output_position_ + size <= BUFFER_SIZE);
  memcpy(output_ + output_position_, buffer, size);
  output_position_ += size;
  return size;
}

size_t SerialBase::readBytes(uint8_t *buffer, size_t length) {
  length = length < available() ? length : available();
  assert(length > 0);
  memcpy(buffer, input_ + input_position_, length);
  input_position_ += length;
  return length;
}

void SerialBase::println(const char *message) {
  printf("%s\r\n", message);
}

int SerialBase::read() {
  assert(input_position_ < BUFFER_SIZE);
  if(input_position_ >= BUFFER_SIZE)
    return 0;
  return input_[input_position_++];
}
