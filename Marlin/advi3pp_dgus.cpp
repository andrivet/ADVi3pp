/**
 * Marlin 3D Printer Firmware For Wanhao Duplicator i3 Plus (ADVi3++)
 * DWIN DGUS utility classes
 *
 * Copyright (C) 2017-2019 Sebastien Andrivet [https://github.com/andrivet/]
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

#include <string.h>
#include "advi3pp_defines.h"
#include "advi3pp_dgus.h"
#include "advi3pp_log.h"

namespace advi3pp {

namespace { const size_t MAX_GARBAGE_BYTES = 5; }

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
//! @param frame    The frame
//! @param data     Byte to be appended
//! @return         Itself
Frame& operator<<(Frame& frame, const Uint8 &data)
{
    if(frame.position_ < Frame::FRAME_BUFFER_SIZE)
    {
        frame.buffer_[frame.position_++] = data.byte;
        frame.buffer_[Frame::Position::Length] += 1;
    }
    else
        Log::error() << F("Data truncated") << Log::endl();
    return frame;
}

//! Append a word to this Frame.
//! @param frame    The frame
//! @param data     Word to be appended
//! @return         Itself
Frame& operator<<(Frame& frame, const Uint16 &data)
{
    if(frame.position_ < Frame::FRAME_BUFFER_SIZE - 1)
    {
        frame.buffer_[frame.position_++] = highByte(data.word);
        frame.buffer_[frame.position_++] = lowByte(data.word);
        frame.buffer_[Frame::Position::Length] += 2;
    }
    else
        Log::error() << F("Data truncated") << Log::endl();
    return frame;
}

//! Append a dword to this Frame.
//! @param frame    The frame
//! @param data     DWord to be appended
//! @return         Itself
Frame& operator<<(Frame& frame, const Uint32 &data)
{
    if(frame.position_ < Frame::FRAME_BUFFER_SIZE - 3)
    {
        frame.buffer_[frame.position_++] = static_cast<uint8_t>(data.dword >> 24);
        frame.buffer_[frame.position_++] = static_cast<uint8_t>(data.dword >> 16);
        frame.buffer_[frame.position_++] = static_cast<uint8_t>(data.dword >> 8);
        frame.buffer_[frame.position_++] = static_cast<uint8_t>(data.dword & 0xFF);
        frame.buffer_[Frame::Position::Length] += 4;
    }
    else
        Log::error() << F("Data truncated") << Log::endl();
    return frame;
}

//! Append a Register to this Frame.
//! @param frame    The frame
//! @param reg      Register to be appended
//! @return         Itself
Frame& operator<<(Frame& frame, Register reg)
{
    frame << Uint8(reg);
    return frame;
}

//! Append a Page to this Frame.
//! @param frame    The frame
//! @param page     Page to be appended
//! @return         Itself
Frame& operator<<(Frame& frame, Page page)
{
    frame << Uint8(page);
    return frame;
}

//! Append a Variable to this Frame.
//! @param frame    The frame
//! @param var      Variable to be appended
//! @return         Itself
Frame& operator<<(Frame& frame, Variable var)
{
    frame << Uint16(var);
    return frame;
}

//! Append a string to this Frame.
//! @param frame    The frame
//! @param s        String to be appended
//! @return         Itself
Frame& operator<<(Frame& frame, const char* s)
{
    auto l = strlen(s);
    size_t length = frame.position_ + l < Frame::FRAME_BUFFER_SIZE ? l : Frame::FRAME_BUFFER_SIZE - frame.position_;
    memcpy(frame.buffer_ + frame.position_, s, length);
    frame.position_ += length;
    frame.buffer_[Frame::Position::Length] += length;
    return frame;
}

//! Send this Frame to the LCD display.
//! @param logging  Enable logging in DEBUG release
bool Frame::send(bool logging)
{
#ifdef ADVi3PP_LOG_ALL_FRAMES
    logging = true;
#endif

    if(logging)
    {
        Log::log() << F("<=S= 0x") << get_length() << F(" bytes, cmd = 0x") << static_cast<uint8_t>(get_command());
#ifdef ADVi3PP_LOG_FRAMES
        Log::log() << F(" ");
        Log::dump(buffer_, get_length() + 3);
#else
        Log::log() << Log::endl();
#endif
    }
    size_t size = 3 + buffer_[Position::Length];
    return Serial2.write(buffer_, size) == size; // Header, length and data
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
//! @param length       Number of bytes to be available before returning
void Frame::wait_for_data(uint8_t length)
{
    while (Serial2.available() < length)
        delay(50);
}

//! Check if data (bytes) are available.
//! @param bytes    The amount of bytes
//! @return         True if the amount of bytes is available
bool Frame::available(uint8_t bytes)
{
    return Serial2.available() >= bytes;
}

//! Receive data from the LCD display.
bool Frame::receive(bool log)
{
    // Format of the frame:
    // header | length | command | data
    // -------|--------|---------|------
    //      2 |      1 |       1 |    N  bytes
    //  5A A5 |     06 |      83 |  ...

#ifdef ADVi3PP_LOG_ALL_FRAMES
    log = true;
#endif

    uint8_t header0 = 0;
    for(size_t index = 0; index < MAX_GARBAGE_BYTES; ++index)
    {
        wait_for_data(1);
        header0 = static_cast<uint8_t>(Serial2.read());
        if(header0 == HEADER_BYTE_0)
            break;
        Log::error() << F("Garbage read: ") << header0 << Log::endl();
    }
    if(header0 != HEADER_BYTE_0)
    {
        Log::error() << F("Not able to locate frame in data received, abort receiving") << Log::endl();
        return false;
    }

    wait_for_data(2);
	auto header1 = static_cast<uint8_t>(Serial2.read());
    if(header1 != HEADER_BYTE_1)
    {
        Log::error() << F("Invalid header when receiving a Frame: ") << header0 << ", " << header1 << Log::endl();
        return false;
    }

    auto length = static_cast<uint8_t>(Serial2.read());

    buffer_[0] = HEADER_BYTE_0;
    buffer_[1] = HEADER_BYTE_1;

    if(length >= FRAME_BUFFER_SIZE - 3)
    {
        Log::error() << F("Data to be received is too big for the Frame buffer so skip it") << Log::endl();
        return false;
    }

    buffer_[2] = length;

    wait_for_data(length);
    auto read = Serial2.readBytes(buffer_ + 3, length);
    if(read != length)
    {
        Log::error() << F("Invalid amount of bytes received: ") << read << F(" instead of ") << length << Log::endl();
        return false;
    }

#ifdef ADVi3PP_LOG_FRAMES
    if(log)
    {
        Log::log() << F("=R=> ") << length << F(" bytes. ");
        Log::dump(buffer_, length + 3);
    }
#endif

    position_ = Position::Command;
    return true;
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

#ifdef ADVi3PP_UNIT_TEST
//! Return the raw data. This is used by unit testing.
const uint8_t* Frame::get_data() const
{
    return buffer_;
}
#endif

//! Extract the next byte from this input Frame.
//! @param frame    The Frame
//! @param data     Next byte extracted from this Frame
//! @return         Itself
Frame& operator>>(Frame& frame, Uint8& data)
{
    if(frame.position_ >= 3 + frame.get_length())
    {
        Log::log() << F("Try to read a byte after the end of data") << Log::endl();
        return frame;
    }

    data.byte = frame.buffer_[frame.position_++];
    return frame;
}

//! Extract the next word from this input Frame.
//! @param frame    The Frame
//! @param data     Next word extracted from this Frame
//! @return         Itself
Frame& operator>>(Frame& frame, Uint16& data)
{
    if(frame.position_ >= 3 + frame.get_length() - 1)
    {
        Log::log() << F("Try to read a word after the end of data") << Log::endl();
        return frame;
    }

    Uint8 msb, lsb;
    frame >> msb >> lsb;
    data.word = lsb.byte + 256 * msb.byte;
    return frame;
}

//! Extract the next dword from this input Frame.
//! @param frame    The Frame
//! @param data     Next dword extracted from this Frame
//! @return         Itself
Frame& operator>>(Frame& frame, Uint32& data)
{
    if(frame.position_ >= 3 + frame.get_length() - 3)
    {
        Log::log() << F("Try to read a dword after the end of data") << Log::endl();
        return frame;
    }

    Uint8 b0, b1, b2, b3;
    frame >> b0 >> b1 >> b2 >> b3;
    data.dword = (((static_cast<uint32_t>(b0.byte) * 256 + b0.byte) * 256 + b1.byte) * 256 + b2.byte) * 256 + b3.byte;
    return frame;
}

//! Extract the Action from this input Frame.
//! @param frame    The Frame
//! @param action   Action extracted from this Frame
//! @return         Itself
Frame& operator>>(Frame& frame, Action& action)
{
    Uint16 value;
    frame >> value;
    action = static_cast<Action>(value.word);
    return frame;
}

//! Extract the Command from this input Frame.
//! @param frame    The Frame
//! @param command  Command extracted
//! @return         Itself
Frame& operator>>(Frame& frame, Command& command)
{
    Uint8 value;
    frame >> value;
    command = static_cast<Command>(value.byte);
    return frame;
}

//! Extract a Register from this Frame
//! @param frame    The Frame
//! @param reg      Register extracted
//! @return         Itself
Frame& operator>>(Frame& frame, Register& reg)
{
    Uint8 value;
    frame >> value;
    reg = static_cast<Register>(value.byte);
    return frame;
}

//! Extract a Variable from this Frame
//! @param frame    The Frame
//! @param var      Variable extracted
//! @return         Itself
Frame& operator>>(Frame& frame, Variable& var)
{
    Uint16 value;
    frame >> value;
    var = static_cast<Variable>(value.word);
    return frame;
}

WriteRegisterDataRequest::WriteRegisterDataRequest(Register reg)
: Frame(Command::WriteRegisterData)
{
    *this << reg;
}

ReadRegisterDataRequest::ReadRegisterDataRequest(Register reg, uint8_t nb_bytes)
: Frame{Command::ReadRegisterData}
{
    *this << reg << Uint8{nb_bytes};
}

Register ReadRegisterDataRequest::get_register() const
{
    return static_cast<Register>(buffer_[Position::Register]);
}

uint8_t ReadRegisterDataRequest::get_nb_bytes() const
{
    return buffer_[Position::NbBytes];
}

bool ReadRegisterDataResponse::receive(Register reg, uint8_t nb_bytes, bool log)
{
    if(!Frame::receive(log))
        return false;

    Command command{}; Register frame_reg{}; Uint8 frame_nb_bytes;
    *this >> command >> frame_reg >> frame_nb_bytes;
    if(command != Command::ReadRegisterData)
    {
        Log::error() << F("Command in response (") << static_cast<uint8_t>(command) << F(") does not correspond to request (") << static_cast<uint8_t>(Command::ReadRegisterData) << F(")") << Log::endl();
        return false;
    }
    if(frame_reg != reg)
    {
        Log::error() << F("Register in response (") << static_cast<uint8_t>(frame_reg) << F(") does not correspond to request (") << static_cast<uint8_t>(reg) << F(")") << Log::endl();
        return false;
    }
    if(frame_nb_bytes.byte != nb_bytes)
    {
        Log::error() << F("Data length in response (") << frame_nb_bytes.byte << F(") does not correspond to request (") << nb_bytes << F(")") << Log::endl();
        return false;
    }

    return true;
}

bool ReadRegisterDataResponse::receive(const ReadRegisterDataRequest& request, bool log)
{
    return receive(request.get_register(), request.get_nb_bytes(), log);
}

ReadRegister::ReadRegister(Register reg, uint8_t nb_bytes)
: request{reg, nb_bytes}
{
}

bool ReadRegister::send_and_receive(bool log)
{
    if(!request.send(log))
        return false;
    return receive(request, log);
}


WriteRamDataRequest::WriteRamDataRequest(Variable var)
: Frame{Command::WriteRamData}
{
    *this << var;
}

void WriteRamDataRequest::reset(Variable var)
{
    Frame::reset(get_command());
    *this << var;
}

ReadRamDataRequest::ReadRamDataRequest(Variable var, uint8_t nb_words)
: Frame{Command::ReadRamData}
{
    auto h = static_cast<uint8_t>(static_cast<uint16_t>(var) / 256);
    auto l = static_cast<uint8_t>(static_cast<uint16_t>(var) % 256);
    *this << Uint8{h} << Uint8{l} << Uint8{nb_words};
}

Variable ReadRamDataRequest::get_variable() const
{
    auto h = buffer_[Position::Variable];
    auto l = buffer_[Position::Variable + 1];
    return static_cast<Variable>(h * 256 + l);
}

uint8_t ReadRamDataRequest::get_nb_words() const
{
    return buffer_[Position::NbWords];
}


bool ReadRamDataResponse::receive(Variable var, uint8_t nb_words)
{
    // Format of the frame:
    // header | length | command | variable | nb words | value
    // -------|--------|---------|----------|----------|------
    //      2 |      1 |       1 |        2 |        1 |     2   bytes
    //  5A A5 |     06 |      83 |     0460 |       01 | 01 50

    if(!Frame::receive())
        return false;
    Command command{}; Variable frame_var{}; Uint8 frame_nb_words;
    *this >> command >> frame_var >> frame_nb_words;
    if(command != Command::ReadRamData)
    {
        Log::error() << F("Command in response (") << static_cast<uint8_t>(command) << F(") does not correspond to request (") << static_cast<uint8_t>(Command::ReadRamData) << F(")") << Log::endl();
        return false;
    }
    if(frame_var != var)
    {
        Log::error() << F("Register in response (") << static_cast<uint16_t>(frame_var) << ") does not correspond to request (" << static_cast<uint16_t>(var) << F(")") << Log::endl();
        return false;
    }
    if(frame_nb_words.byte != nb_words)
    {
        Log::error() << F("Data length in response (") << frame_nb_words.byte << ") does not correspond to request (" << nb_words << ")" << Log::endl();
        return false;
    }

    return true;
}

bool ReadRamDataResponse::receive(const ReadRamDataRequest& request)
{
    return receive(request.get_variable(), request.get_nb_words());
}

ReadRamData::ReadRamData(Variable var, uint8_t nb_words)
: request{var, nb_words}
{
}

bool ReadRamData::send_and_receive()
{
    if(!request.send())
        return false;
    return receive(request);
}


WriteCurveDataRequest::WriteCurveDataRequest(uint8_t channels)
: Frame{Command::WriteCurveData}
{
    *this << Uint8{channels};
}

}
