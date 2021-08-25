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

#include "serial.h"

SerialBase Serial2;
SerialBase Serial;

size_t SerialBase::write(uint8_t value) {
  buffer_[position_++] = value;
  return 1;
}

size_t SerialBase::write(const uint8_t *buffer, size_t size) {
  for(size_t i = 0; i < size; ++i)
    buffer_[position_++] = buffer[i];
  return size;
}

size_t SerialBase::write(const char *buffer, size_t size) {
  for(size_t i = 0; i < size; ++i)
    buffer_[position_++] = buffer[i];
  return size;
}

size_t SerialBase::readBytes(uint8_t *buffer, size_t length) {
  length = length < available() ? length : available();
  memcpy(buffer, buffer_ + position_, length);
  return length;
}

void SerialBase::println(const char *message) {
  printf("%s\r\n", message);
}

int SerialBase::read() {
  if(position_ >= BUFFER_SIZE)
    return 0;
  return buffer_[position_++];
}
