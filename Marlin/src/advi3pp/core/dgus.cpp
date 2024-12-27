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

#include "../../inc/MarlinConfig.h"
#include "logging.h"
#include "dgus.h"
#include "../../lcd/extui/ui_api.h"

namespace ADVi3pp::Dgus {

  inline namespace internals {
    auto& DgusSerial = Serial2;
    constexpr uint32_t  LCD_BAUDRATE = 115200; // Between the LCD panel and the mainboard
    constexpr uint16_t  LCD_READ_DELAY = 50; // ms
    constexpr uint16_t  LCD_READ_KILL_COUNT = 16; // must be less that the watchdog time
    constexpr byte      R2 = 0x0D; // SYS_CFG, disable buzzer, L22 init, auto key codes
    constexpr size_t    MAX_PUSH_BACK = 5;
    enum class State { Start = 0, Command = 1, Data = 2};

    State   state_ = State::Start;
    uint8_t length_ = 0;
    uint8_t read_ = 0;
    Command command_ = Command::None;
    uint8_t nb_pushed_back_ = 0;
    uint8_t pushed_back_[MAX_PUSH_BACK] = {};

    void kill();
    bool receive_header();
    bool has_pushed_back();
    uint8_t get_pushed_back();
  }

  // --------------------------------------------------------------------
  // Dgus - DGUS LCD panel
  // --------------------------------------------------------------------

  //! Open the serial communication between the mainboard and the LCD panel
  void open() {
    DgusSerial.begin(LCD_BAUDRATE);
  }

  void setup() {
    ReadRegister read{Register::R2};
    if(!read.send_receive(1))
      kill();
    uint8_t r2 = read.read_byte();

    if(r2 == R2)
      return;

    Log::warning() << F("Reprogram R2 register to") << R2 << F("was") << r2 << Log::endl();
    WriteRegisterRequest{Register::R2}.write_byte(R2);
  }

  void forwarding_loop() {
    while(true)
    {
      ExtUI::watchdogReset();

      if(MYSERIAL1.available())
        DgusSerial.write(MYSERIAL1.read());

      if(DgusSerial.available())
        MYSERIAL1.write(DgusSerial.read());
    }
  }

  void get_firmware_version(char version[4]) {
    ReadRegister response{Register::Version};
    if(!response.send_receive(1)) {
      version[0] = '-';
      version[1] = '-';
      version[2] = '-';
      version[3] = 0;
      return;
    }

    auto lcd = response.read_byte();
    version[0] = static_cast<char>('0' + lcd / 0x10);
    version[1] = '.';
    version[2] = static_cast<char>('0' + lcd % 0x10);
    version[3] = 0;
  }

  bool write_header(Command cmd, uint8_t param_size, uint8_t data_size) {
    // Format of the frame:
    // header | length | command | parameter | data
    // -------|--------|---------|-----------|-------
    //      2 |      1 |       1 | 1 or 2    | N bytes
    //  5A A5 |     06 |      80 | ...       | ...

    adv::array<uint8_t, 4> header =
    {
      HEADER_BYTE_0,
      HEADER_BYTE_1,
      static_cast<uint8_t>(1 + param_size + data_size),
      static_cast<uint8_t>(cmd)
    };

    Log::verbose(true) << F("<==S=") << header;
    if(header.size() != DgusSerial.write(header.data(), header.size()))
      return false;

    return true;
  }

  //! Wait for the given amount of bytes from the LCD display.
  //! @param length       Number of bytes to be available before returning
  bool wait_for_data(uint8_t size, bool blocking) {
    if(nb_pushed_back_ >= size)
      return true;
    size -= nb_pushed_back_;

    if(!blocking && DgusSerial.available() < size)
      return false;

    unsigned count = 0;
    while(DgusSerial.available() < size) {
        delay(LCD_READ_DELAY);
        count += 1;
        if(count > LCD_READ_KILL_COUNT)
          kill();
    }

    return true;
  }

  bool receive(Command cmd, bool blocking) {
    // Format of the frame:
    // header | length | command | data
    // -------|--------|---------|------
    //      2 |      1 |       1 |    N  bytes
    //  5A A5 |     06 |      83 |  ...

    if(state_ == State::Start) {
      if(!wait_for_data(4, blocking))
          return false;

      if(!receive_header())
          return false;

      auto length = static_cast<uint8_t>(DgusSerial.read());
      if(length < 3)       {
        Log::error() << F("Invalid frame length:") << length << Log::endl();
        return false;
      }
      auto command = static_cast<uint8_t>(DgusSerial.read());
      if(command < 0x80 || command > 0x84) {
        Log::error() << F("Invalid frame command:") << command << Log::endl();
        return false;
      }

      length_ = length;
      command_ = static_cast<Command>(command);
      state_ = State::Command;
      read_ = 1; // Command is 1 byte

      Log::verbose() << length << command;
    }

    if(command_ != cmd)
      return false;

    if(!wait_for_data(length_ - read_, blocking))
      return false;

    return true;
  }

  uint8_t read_byte() {
    uint8_t byte = has_pushed_back() ? get_pushed_back() : DgusSerial.read();
    read_ += 1;
    Log::verbose() << byte;
    if(read_ == length_) {
      state_ = State::Start;
      Log::verbose() << Log::endl();
    }
    return byte;
  }

  size_t read_bytes(uint8_t *buffer, size_t length) {
    size_t n = length;
    while(n--)
      *(buffer++) = read_byte();
    return length;
  }

  void push_back(uint8_t byte) {
    if(nb_pushed_back_ >= MAX_PUSH_BACK) {
      Log::error() << F("Pushback overflow") << Log::endl();
      return;
    }

    pushed_back_[nb_pushed_back_++] = byte;
    assert(read_ > 0);
    read_ -= 1;
    Log::verbose() << F("//") << byte << F("//");
  }

  bool write_byte(uint8_t byte) {
    Log::verbose() << byte;
    return DgusSerial.write(byte) == 1;
  }

  bool write_bytes(const uint8_t *bytes, size_t length) {
    Log::verbose().write(bytes, length);
    return DgusSerial.write(bytes, length) == length;
  }

  bool write_bytes(const char *bytes, size_t length) {
    Log::verbose().write(reinterpret_cast<const uint8_t*>(bytes), length);
    return DgusSerial.write(bytes, length) == length;
  }

  bool write_bytes(const FlashChar *bytes, size_t length) {
    const char *current = from_flash(bytes);
    while(length-- > 0) if(DgusSerial.write(pgm_read_byte(current++)) != 1) return false;
    return true;
  }

  bool write_word(uint16_t word) {
    if(!write_byte(highByte(word)) || !write_byte(lowByte(word))) return false;
    return true;
  }

  bool write_words(const uint16_t *words, size_t length) {
    for(size_t i = 0; i < length; ++i) if(!write_word(words[i])) return false;
    return true;
  }

  bool write_padding(size_t length) {
    while(length-- > 0) if(!write_byte(' ')) return false;
    return true;
  }

  bool write_text(const char* text, size_t text_length, size_t total_length) {
    if(text_length > total_length) text_length = total_length;
    if(!write_bytes(text, text_length)) return false;
    return write_padding(total_length - text_length);
  }

  bool write_text(const FlashChar *text, size_t text_length, size_t total_length) {
    if(text_length > total_length) text_length = total_length;
    if(!write_bytes(text, text_length)) return false;
    return write_padding(total_length - text_length);
  }

  bool write_centered_text(const char* text, size_t text_length, size_t total_length) {
    if(text_length > total_length) text_length = total_length;
    auto pad = (total_length - text_length) / 2;
    return write_padding(pad) && write_bytes(text, text_length) && write_padding(total_length - pad - text_length + pad);
  }

  bool write_centered_text(const FlashChar* text, size_t text_length, size_t total_length) {
    if(text_length > total_length) text_length = total_length;
    auto pad = (total_length - text_length) / 2;
    return write_padding(pad) && write_bytes(text, text_length) && write_padding(total_length - pad - text_length + pad);
  }

  inline namespace internals {

    void kill() {
      SERIAL_ERROR_START();
      SERIAL_ECHOLNPGM("LCD panel does not respond. Check cable between mainboard and LCD Panel. Printer is stopped.");
      ExtUI::killRightNow();
    }

    bool receive_header() {
      for(size_t index = 0; ; ++index) {
        uint8_t header0 = DgusSerial.read();
        if(header0 != HEADER_BYTE_0 && index >= MAX_GARBAGE_BYTES) {
          Log::error() << F("Not able to find the start of a frame") << Log::endl();
          return false;
        }
        if(header0 == HEADER_BYTE_0)
          break;
        Log::info() << F("Discard garbage") << header0 << Log::endl();
      }

      uint8_t header1 = DgusSerial.read();
      if(header1 != HEADER_BYTE_1) {
        Log::error() << F("Not able to find the start of a frame") << header1 << Log::endl();
        return false;
      }

      Log::verbose(true) << F("=R==>") << HEADER_BYTE_0 << HEADER_BYTE_1;
      return true;
    }

    bool has_pushed_back() {
      return nb_pushed_back_ > 0;
    }

    uint8_t get_pushed_back() {
      return pushed_back_[--nb_pushed_back_];
    }

  }
}
