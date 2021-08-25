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

#include "../parameters.h"
#include "logging.h"
#include "dgus.h"
#ifndef ADV_UNIT_TESTS
#include "../../lcd/extui/ui_api.h"
#endif

namespace ADVi3pp {

namespace
{
    auto& DgusSerial = Serial2;
    const uint32_t  LCD_BAUDRATE = 115200; // Between the LCD panel and the mainboard
    const uint16_t  LCD_READ_DELAY = 50; // ms
    const uint16_t  LCD_READ_KILL_COUNT = 100; // must be less that the watchdog time
    const byte      R2 = 0x0D; // SYS_CFG, disable buzzer, L22 init, auto key codes
}

// --------------------------------------------------------------------
// Dgus - DGUS LCD panel
// --------------------------------------------------------------------

Dgus::State Dgus::state_ = State::Start;
uint8_t Dgus::length_ = 0;
uint8_t Dgus::read_ = 0;
Command Dgus::command_ = Command::None;
uint8_t Dgus::pushed_back_[Dgus::MAX_PUSH_BACK] = {};

//! Open the serial communication between the mainboard and the LCD panel
void Dgus::open()
{
    DgusSerial.begin(LCD_BAUDRATE);
}

void Dgus::setup()
{
    ReadRegister read{Register::R2};
    if(!read.send_receive(1))
        kill();
    uint8_t r2 = read.read_byte();

    if(r2 == R2)
        return;

    Log::log() << F("Reprogram R2 register to 0x") << R2 << F(", was 0x") << r2 << Log::endl();
    WriteRegisterRequest{Register::R2}.write_byte(R2);
}

void Dgus::kill()
{
#ifndef ADV_UNIT_TESTS
    SERIAL_ERROR_START();
    SERIAL_ECHOLNPGM("LCD panel does not respond. Check cable between mainboard and LCD Panel. Printer is stopped.");
    ExtUI::killRightNow();
#endif
}

void Dgus::forwarding_loop()
{
    while(true)
    {
#ifndef ADV_UNIT_TESTS
        ExtUI::watchdogReset();

        if(MYSERIAL0.available())
            DgusSerial.write(MYSERIAL0.read());

        if(DgusSerial.available())
            MYSERIAL0.write(DgusSerial.read());
#endif
    }
}

#ifdef ADV_UNIT_TESTS
void Dgus::reset() {
  DgusSerial.reset();
  state_ = State::Start;
  length_ = 0;
  read_ = 0;
  command_ = Command::None;
}
#endif

bool Dgus::write_header(Command cmd, uint8_t param_size, uint8_t data_size)
{
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

    if(header.size() != DgusSerial.write(header.data(), header.size()))
        return false;

    return true;
}

//! Wait for the given amount of bytes from the LCD display.
//! @param length       Number of bytes to be available before returning
bool Dgus::wait_for_data(uint8_t size, ReceiveMode mode)
{
    auto pushed_back = static_cast<uint8_t>(state_) - static_cast<uint8_t>(State::PushedBack);
    if(pushed_back >= size)
        return true;
    size -= pushed_back;

    if(mode == ReceiveMode::NonBlocking && DgusSerial.available() < size)
        return false;

    unsigned count = 0;
    while(DgusSerial.available() < size)
    {
        delay(LCD_READ_DELAY);
        count += 1;
        if(count > LCD_READ_KILL_COUNT)
            kill();
    }

    return true;
}

bool Dgus::receive_header()
{
    for(size_t index = 0; ; ++index)
    {
        uint8_t header0 = DgusSerial.read();
        if(header0 != HEADER_BYTE_0 && index >= MAX_GARBAGE_BYTES)
            return false;
        if(header0 == HEADER_BYTE_0)
            break;
    }

    uint8_t header1 = DgusSerial.read();
    if(header1 != HEADER_BYTE_1)
        return false;

    return true;
}

bool Dgus::receive(Command cmd, ReceiveMode mode)
{
    // Format of the frame:
    // header | length | command | data
    // -------|--------|---------|------
    //      2 |      1 |       1 |    N  bytes
    //  5A A5 |     06 |      83 |  ...

    if(state_ == State::Start)
    {
        if(!wait_for_data(4, mode))
            return false;

        if(!receive_header())
            return false;

        auto length = static_cast<uint8_t>(DgusSerial.read());
        if(length < 3)
        {
            Log::error() << F("Invalid frame length: 0x") << length << Log::endl();
            return false;
        }
        auto command = static_cast<uint8_t>(DgusSerial.read());
        if(command < 0x80 || command > 0x84)
        {
            Log::error() << F("Invalid frame command: 0x") << command << Log::endl();
            return false;
        }

        length_ = length;
        command_ = static_cast<Command>(command);
        state_ = State::Command;
        read_ = 1; // Command is 1 byte

        Log::log() << F("Receive frame length: 0x") << length << F(", cmd: 0x") << command << Log::endl();
    }

    if(command_ != cmd)
        return false;

    return true;
}

bool Dgus::has_pushed_back()
{
    const auto pushed_back_state = static_cast<size_t>(State::PushedBack);
    const auto state = static_cast<size_t>(state_);
    return state >= pushed_back_state && state <= pushed_back_state + MAX_PUSH_BACK;
}

uint8_t Dgus::get_pushed_back()
{
    const auto pushed_back_state = static_cast<size_t>(State::PushedBack);
    const auto state = static_cast<size_t>(state_);
    uint8_t byte = pushed_back_[state - pushed_back_state];
    state_ = static_cast<State>(state - 1);
    return byte;
}

uint8_t Dgus::read_byte()
{
    uint8_t byte = has_pushed_back() ? get_pushed_back() : DgusSerial.read();
    read_ += 1;
    if(read_ == length_)
        state_ = State::Start;
    return byte;
}

size_t Dgus::read_bytes(uint8_t *buffer, size_t length)
{
    size_t n = length;
    while(n--)
        *(buffer++) = read_byte();
    return length;
}

void Dgus::push_back(uint8_t byte)
{
    const auto pushed_back_state = static_cast<size_t>(State::PushedBack);
    const auto state = static_cast<size_t>(state_);

    if(state >= pushed_back_state + MAX_PUSH_BACK)
    {
        Log::error() << F("Pushback overflow") << Log::endl();
        return;
    }

    if(state_ < State::Command)
    {
        Log::error() << F("Invalid Pushback state") << Log::endl();
        return;
    }

    size_t new_state = state + 1;
    state_ = static_cast<State>(new_state);
    pushed_back_[new_state] = byte;
    assert(read_ > 0);
    read_ -= 1;
}

bool Dgus::write_byte(uint8_t byte)
{
    return DgusSerial.write(byte);
}

bool Dgus::write_bytes(const uint8_t *bytes, size_t length)
{
    return DgusSerial.write(bytes, length);
}

bool Dgus::write_bytes(const char *bytes, size_t length)
{
    return DgusSerial.write(bytes, length);
}

bool Dgus::write_words(const uint16_t *words, size_t length)
{
    for(size_t i = 0; i < length; ++i)
    {
        if(!Dgus::write_byte(highByte(words[i])) || !Dgus::write_byte(lowByte(words[i])))
            return false;
    }
    return true;
}

bool Dgus::write_text(const char* text, size_t text_length, size_t total_length)
{
    if(!Dgus::write_bytes(text, text_length))
        return false;
    // Fill the remaining of string with spaces
    for(size_t i =text_length; i < total_length; ++i)
    {
        if(!write_byte(' '))
            return false;
    }
    return true;
}

bool Dgus::write_centered_text(const char* text, size_t text_length, size_t total_length)
{
    // Pad the beginning to center the string
    auto pad = (total_length - text_length) / 2;
    for(size_t i = 0; i < pad; ++i)
    {
        if(!Dgus::write_byte(' '))
            return false;
    }
    // The string itself
    if(!Dgus::write_bytes(text, text_length))
        return false;
    // Fill the remaining of the string with spaces
    for(size_t i = text_length + pad; i < total_length; ++i)
    {
        if(!Dgus::write_byte(' '))
            return false;
    }
    return true;
}

// --------------------------------------------------------------------

/*
bool ReadRegister::send_and_receive()
{
    ReadRegisterRequest request{register_, nb_bytes_};
    if(!request.write())
        return false;
    return receive(ReceiveMode::Blocking);
}

bool ReadRam::send_and_receive()
{
    ReadRamRequest request{variable_};
    if(!request.write_byte(1))
        return false;
    return receive(ReceiveMode::Blocking);
}
*/
// --------------------------------------------------------------------


Command last_command_;

}