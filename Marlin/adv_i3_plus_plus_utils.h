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
#ifndef ADV_I3_PLUS_PLUS_UTILS_H
#define ADV_I3_PLUS_PLUS_UTILS_H

#ifndef NO_MARLIN
#include "Marlin.h"

#if ENABLED(PRINTCOUNTER)
#include "duration_t.h"
#endif
#endif

namespace advi3pp { inline namespace internals {

enum class Register: uint8_t;
enum class Variable: uint16_t;
enum class Command: uint8_t;
enum class Action: uint16_t;

// --------------------------------------------------------------------
// Logging
// --------------------------------------------------------------------

#ifdef DEBUG
#define ADVi3PP_ERROR(expresssion) {Name<100> message; message << expresssion; Serial.println(message.c_str());}
#define ADVi3PP_LOG(expresssion)   {Name<100> message; message << expresssion; Serial.println(message.c_str());}
#else
#define ADVi3PP_ERROR(expresssion) {}
#define ADVi3PP_LOG(expresssion)   {}
#endif

// --------------------------------------------------------------------
// Uint8
// --------------------------------------------------------------------

//! An unsigned 8 bits value.
struct Uint8
{
    uint8_t byte; //!< The actual value
    constexpr explicit Uint8(uint8_t value = 0) : byte{value} {}
    constexpr explicit Uint8(Register reg) : byte{static_cast<uint8_t>(reg)} {}
    constexpr explicit Uint8(Page page) : byte{static_cast<uint8_t>(page)} {}
};

// --------------------------------------------------------------------
// Uint16
// --------------------------------------------------------------------

//! An unsigned 16 bit. value.
struct Uint16
{
    uint16_t word; //!< The actual value
    constexpr explicit Uint16(uint16_t value = 0) : word{value} {}
    constexpr explicit Uint16(int16_t value) : word{static_cast<uint16_t>(value)} {}
    constexpr explicit Uint16(long value) : word{static_cast<uint16_t>(value)} {}
    constexpr explicit Uint16(double value) : word{static_cast<uint16_t>(value)} {}
    constexpr explicit Uint16(Variable var) : word{static_cast<uint16_t>(var)} {}
};

//! An unsigned 8 bits literal such as: 0_u8.
constexpr Uint8  operator "" _u8(unsigned long long int byte)  { return Uint8(static_cast<uint8_t>(byte)); }
//! An unsigned 8 bits literal such as: 0_u8.
constexpr Uint16 operator "" _u16(unsigned long long int word) { return Uint16(static_cast<uint16_t>(word)); }

// --------------------------------------------------------------------
// Name
// --------------------------------------------------------------------

//! A fixed-size string of characters, truncating values when necessary.
template<size_t S = 26>
class Name
{
public:
    Name();
    explicit Name(const char* name);
    explicit Name(duration_t duration);

    Name& operator=(const char* name);
    Name& operator=(duration_t duration);

    const uint8_t* get() const { return buffer_; }
    constexpr size_t size() const { return S; }
    uint16_t length() const { return length_; }
    const char* c_str() const { return reinterpret_cast<const char*>(buffer_); }

    Name& operator<<(const char* value);
    Name& operator<<(uint16_t value);
    Name& operator<<(Command command);
    Name& operator<<(Register reg);
    Name& operator<<(Variable var);

private:
    void fill_remaining();

private:
    static constexpr auto SIZE = S > 1 ? S : 1; // Avoid 0 size
    uint8_t buffer_[SIZE]; //!< Fixed-sized buffer holding the content of the Name
    uint16_t length_; //!< Length of the Name content
};

// --------------------------------------------------------------------
// Frame
// --------------------------------------------------------------------
//! A frame to be send to the LCD or received from the LCD
struct Frame
{
    Frame();

    void send();
    Frame& operator<<(const Uint8& data);
    Frame& operator<<(const Uint16& data);
    template<size_t S> Frame& operator<<(const Name<S>& name);
    Frame& operator<<(Page page);

    bool available(uint8_t bytes = 3);
    bool receive();
    Command get_command() const;
    size_t get_length() const;
    Frame& operator>>(Uint8& data);
    Frame& operator>>(Uint16& data);
    Frame& operator>>(Action& action);
    Frame& operator>>(Command& command);
    Frame& operator>>(Register& reg);
    Frame& operator>>(Variable& var);

    void reset();

#ifdef UNIT_TEST
    const uint8_t* get_data() const;
#endif

protected:
    explicit Frame(Command command);
    void reset(Command command);
    Frame& operator<<(Register reg);
    Frame& operator<<(Variable var);

private:
    void wait_for_data(uint8_t length);

protected:
    static const size_t FRAME_BUFFER_SIZE = 255;
    static const uint8_t HEADER_BYTE_0 = 0x5A;
    static const uint8_t HEADER_BYTE_1 = 0xA5;
    struct Position { enum { Header0 = 0, Header1 = 1, Length = 2, Command = 3, Data = 4, Register = 4, Variable = 4, NbBytes = 5, NbWords = 6 }; };

    uint8_t buffer_[FRAME_BUFFER_SIZE];
    uint8_t position_ = 0;
};


// --------------------------------------------------------------------
// Frame subclasses
// --------------------------------------------------------------------

struct WriteRegisterDataRequest: Frame
{
    explicit WriteRegisterDataRequest(Register reg);
};

struct ReadRegisterDataRequest: Frame
{
    ReadRegisterDataRequest(Register reg, uint8_t nb_bytes);
    Register get_register() const;
    uint8_t get_nb_bytes() const;
};

struct ReadRegisterDataResponse: Frame
{
    ReadRegisterDataResponse() = default;
    bool receive(Register reg, uint8_t nb_bytes);
    bool receive(const ReadRegisterDataRequest& request);
};

struct WriteRamDataRequest: Frame
{
    explicit WriteRamDataRequest(Variable var);
    void reset(Variable var);
};

struct ReadRamDataRequest: Frame
{
    ReadRamDataRequest(Variable var, uint8_t nb_words);
};

struct ReadRamDataResponse: Frame
{
    ReadRamDataResponse() = default;
    bool receive(Variable var, uint8_t nb_words);
    bool receive(const ReadRamDataRequest& request);
};

struct WriteCurveDataRequest: Frame
{
    explicit WriteCurveDataRequest(uint8_t channels);
};


// --------------------------------------------------------------------
// Name
// --------------------------------------------------------------------

//! Construct an empty Name.
//! @tparam S Maximum site of the name (including terminating 0 byte)
template<size_t S>
Name<S>::Name(): length_{0}
{
    memset(buffer_, 0, sizeof(buffer_));
}

//! Construct a Name from a duration.
//! @tparam S Maximum site of the name (including terminating 0 byte)
//! @param duration Duration to be translated to a Name
template<size_t S>
Name<S>::Name(duration_t duration)
{
    *this = duration;
}

//! Constrcut a Name from a string of characters
//! @tparam S Maximum site of the name (including terminating 0 byte)
//! @param name String of characters to be copied into this Name.
template<size_t S>
Name<S>::Name(const char* name)
{
    *this = name;
}

//! Fill the remaining space in the buffer with 0s
template<size_t S>
void Name<S>::fill_remaining()
{
    if(length_ < SIZE - 1)
        memset(buffer_ + length_ + 1, 0, SIZE - length_ - 1);
}

//! Assign a new value to this Name from a string of characters. It is truncated if it does not fit into the Name.
//! @tparam S       The maximum size of the Name
//! @param name     The value to be copied into this Name
//! @return         Itself
template<size_t S>
Name<S>& Name<S>::operator=(const char* name)
{
    auto length = strlen(name);
    length_ = length < SIZE - 1 ? length : SIZE - 1;

    if(length_ > 0)
        memcpy(buffer_, name, length_);

    buffer_[length_] = 0;
    fill_remaining();

    return *this;
}

//! Assign a new value to this name from a duration
//! @tparam S           The maximum size of the Name
//! @param duration     The value to be copied (after transformation) into this Name
//! @return             Itself
template<size_t S>
Name<S>& Name<S>::operator=(duration_t duration)
{
    auto char_buffer = reinterpret_cast<char*>(buffer_);
    duration.toString(char_buffer);
    length_ = strlen(char_buffer);
    fill_remaining();
    return *this;
}

//! Append a string of character to this Name. It is truncated if it does not fit into the Name.
//! @tparam S           The maximum size of the Name
//! @param value        The value to be append to this Name
//! @return             Itself
template<size_t S>
Name<S>& Name<S>::operator<<(const char* value)
{
    auto value_length = strlen(value);
    auto actual_value_length = value_length < SIZE - 1 - length_ ? value_length : SIZE - 1 - length_;

    if(actual_value_length > 0)
        memcpy(buffer_ + length_, value, actual_value_length);

    length_ += actual_value_length;

    buffer_[length_] = 0;
    fill_remaining();

    return *this;
}

//! Append a decimal value to this Name. It is truncated if it does not fit into the Name.
//! @tparam S           The maximum size of the Name
//! @param value        The value to be append to this Name (after transformation into a string)
//! @return             Itself
template<size_t S>
Name<S>& Name<S>::operator<<(uint16_t value)
{
    char buffer[32];
    sprintf(buffer, "%d", value);
    return (*this << buffer);
}

//! Append a Command to this Name. It is truncated if it does not fit into the Name.
//! @tparam S           The maximum size of the Name
//! @param command      The command to be append to this Name (after transformation into a string)
//! @return             Itself
template<size_t S>
Name<S>& Name<S>::operator<<(Command command)
{
    return (*this << static_cast<uint8_t>(command));
}

//! Append a Register to this Name. It is truncated if it does not fit into the Name.
//! @tparam S           The maximum size of the Name
//! @param reg          The register to be append to this Name (after transformation into a string)
//! @return             Itself
template<size_t S>
Name<S>&  Name<S>::operator<<(Register reg)
{
    return (*this << static_cast<uint8_t>(reg));
}

//! Append a Variable to this Name. It is truncated if it does not fit into the Name.
//! @tparam S           The maximum size of the Name
//! @param var          The variable to be append to this Name (after transformation into a string)
//! @return             Itself
template<size_t S>
Name<S>&  Name<S>::operator<<(Variable var)
{
    return (*this << static_cast<uint16_t>(var));
}


// --------------------------------------------------------------------
// Frame
// --------------------------------------------------------------------

//! Append a Name to this Frame.
//! @tparam S           The size of the Name
//! @param name         The name to be append
//! @return             Itself
template<size_t S>
Frame& Frame::operator<<(const Name<S>& name)
{
    size_t length = position_ + name.size() < FRAME_BUFFER_SIZE ? name.size() : FRAME_BUFFER_SIZE - position_;
    // TODO: Truncate
    memcpy(buffer_ + position_, name.get(), length);
    position_ += length;
    buffer_[Position::Length] += length;
    return *this;
}


}}


#endif //ADV_I3_PLUS_PLUS_UTILS_H
