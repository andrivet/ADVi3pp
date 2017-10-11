/**
 * Marlin 3D Printer Firmware For Wanhao Duplicator i3 Plus (ADVi3++)
 *
 * Copyright (C) 2017 Sebastien Andrivet [https://github.com/andrivet/]
 *
 * Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
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
#include "adv_i3_plus_plus.h"
#include "adv_i3_plus_plus_utils.h"
#include "adv_i3_plus_plus_impl.h"

using namespace advi3pp;

void Error(const char* message)
{
    // TODO
}

// --------------------------------------------------------------------
// Frame
// --------------------------------------------------------------------

//! Construct an input, empty, Frame.
Frame::Frame()
: position_{0}
{
}

//! Construct an output Frame.
//! @param command  The command to be set into this Frame
Frame::Frame(Command command)
{
    buffer_[Position::Header0] = HEADER_BYTE_0;
    buffer_[Position::Header1] = HEADER_BYTE_1;
    buffer_[Position::Length] = 1;
    buffer_[Position::Command] = static_cast<uint8_t>(command);
    position_ = Position::Data;
}

//! Append a byte to this Frame.
//! @param data     Byte to be appended
//! @return         Itself
Frame& Frame::operator<<(const Uint8 &data)
{
    if(position_ < FRAME_BUFFER_SIZE)
    {
        buffer_[position_++] = data.byte;
        buffer_[Position::Length] += 1;
    }
    else
        Error("Data truncated");
    return *this;
}

//! Append a word to this Frame.
//! @param data     Word to be appended
//! @return         Itself
Frame& Frame::operator<<(const Uint16 &data)
{
    if(position_ < FRAME_BUFFER_SIZE - 1)
    {
        buffer_[position_++] = highByte(data.word);
        buffer_[position_++] = lowByte(data.word);
        buffer_[Position::Length] += 2;
    }
    else
        Error("Data truncated");
    return *this;
}

//! Append a Register to this Frame.
//! @param reg      Register to be appended
//! @return         Itself
Frame& Frame::operator<<(Register reg)
{
    *this << Uint8(reg);
    return *this;
}

//! Append a Page to this Frame.
//! @param page     Page to be appended
//! @return         Itself
Frame& Frame::operator<<(Page page)
{
    *this << Uint8(page);
    return *this;
}

//! Append a Variable to this Frame.
//! @param var      Variable to be appended
//! @return         Itself
Frame& Frame::operator<<(Variable var)
{
    *this << Uint16(var);
    return *this;
}

//! Send tis Frame to the LCD display.
void Frame::send()
{
    Serial2.write(buffer_, 3 + buffer_[Position::Length]); // Header, length and data
    // Reset internals so it can be reused (same command)
}

//! Reset this Frame as an input Frame
void Frame::reset()
{
    position_ = 0;
}

//! Reset this Frame as an output Frame.
//! @param command      The command to be set into this Frame
void Frame::reset(Command command)
{
    buffer_[Position::Header0] = HEADER_BYTE_0;
    buffer_[Position::Header1] = HEADER_BYTE_1;
    buffer_[Position::Length] = 1;
    buffer_[Position::Command] = static_cast<uint8_t>(command);
    position_ = Position::Data;
}

//! Wait for the given amount of bytes from the LCD display.
//! @param length       Number of bvtes to be available before returning
void Frame::wait_for_data(uint8_t length)
{
    while (Serial2.available() < length)
        /*nothing*/;
}

//! Receive data from the LCD display.
uint8_t Frame::receive()
{
    if (Serial2.available() < 3)
        return 0;

    if (Serial2.read() != HEADER_BYTE_0)
        return 0;

    wait_for_data(1);
    if (Serial2.read() != HEADER_BYTE_1)
        return 0;

    wait_for_data(1);
    auto length = static_cast<uint8_t>(Serial2.read());

    buffer_[0] = HEADER_BYTE_0;
    buffer_[1] = HEADER_BYTE_1;

    if(length >= FRAME_BUFFER_SIZE - 3)
    {
        Error("Data to be received is too big for the Frame buffer so skip it");
        return 0;
    }

    buffer_[2] = length;
    auto read = Serial2.readBytes(buffer_ + 3, length);
    if(read != length)
        return 0;

    position_ = Position::Data;
    return length + 3;
}

//! Get the Command set inside this Frame.
Command Frame::get_command() const
{
    return static_cast<Command>(buffer_[Position::Command]);
}

//! Return the length of the data portion (including the Command).
size_t Frame::get_length() const
{
    return static_cast<size_t>(buffer_[Position::Length]);
}

//! Return the raw data. This is used by unit testing.
const uint8_t* Frame::get_data() const
{
    return buffer_;
}

//! Extract the next byte from this input Frame.
//! @param data     Next byte extracted from this Frame
//! @return         Itself
Frame& Frame::operator>>(Uint8& data)
{
    if(position_ >= 3 + get_length())
    {
        Error("Try to read a byte after the end of data");
        return *this;
    }

    data.byte = buffer_[position_++];
    return *this;
}

//! Extract the next word from this input Frame.
//! @param data     Next word extracted from this Frame
//! @return         Itself
Frame& Frame::operator>>(Uint16& data)
{
    if(position_ >= 3 + get_length() - 1)
    {
        Error("Try to read a word after the end of data");
        return *this;
    }

    Uint8 msb, lsb;
    (*this) >> msb >> lsb;
    data.word = lsb.byte + 256 * msb.byte;
    return *this;
}

//! Extract the Action from this input Frame.
//! @param action   Action extracted from this Frame
//! @return         Itself
Frame& Frame::operator>>(Action& action)
{
    Uint16 value;
    (*this) >> value;
    action = static_cast<Action>(value.word);
    return *this;
}


WriteRegisterDataFrame::WriteRegisterDataFrame(Register reg)
: Frame(Command::WriteRegisterData)
{
    *this << reg;
}

ReadRegisterDataFrame::ReadRegisterDataFrame(Register reg, uint8_t nb_bytes)
: Frame{Command::ReadRegisterData}
{
    *this << Uint8{nb_bytes};
}

WriteRamDataFrame::WriteRamDataFrame(Variable var)
: Frame{Command::WriteRamData}
{
    *this << var;
}

void WriteRamDataFrame::reset(Variable var)
{
    Frame::reset(get_command());
    *this << var;
}

ReadRamDataFrame::ReadRamDataFrame(Variable var, uint8_t nb_words)
: Frame{Command::ReadRamData}
{
    *this << var << Uint8{nb_words};
}

WriteCurveDataFrame::WriteCurveDataFrame(uint8_t channels)
: Frame{Command::WriteCurveData}
{
    *this << Uint16{channels};
}
