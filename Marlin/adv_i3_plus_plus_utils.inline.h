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
#ifndef MARLIN_ADV_I3_PLUS_PLUS_UTILS_INLINE_H
#define MARLIN_ADV_I3_PLUS_PLUS_UTILS_INLINE_H

// --------------------------------------------------------------------
// Chars
// --------------------------------------------------------------------

//! Construct an empty Chars.
//! @tparam S Maximum site of the name (including terminating 0 byte)
template<size_t S>
Chars<S>::Chars(): length_{0}
{
    memset(buffer_, 0, sizeof(buffer_));
}

//! Construct a Chars from a duration.
//! @tparam S Maximum site of the name (including terminating 0 byte)
//! @param duration Duration to be translated to a Chars
template<size_t S>
Chars<S>::Chars(duration_t duration)
{
    *this = duration;
}

//! Construct a Chars from a string of characters
//! @tparam S Maximum site of the name (including terminating 0 byte)
//! @param name String of characters to be copied into this Chars.
template<size_t S>
Chars<S>::Chars(const char* name)
{
    *this = name;
}

//! Fill the remaining space in the buffer with 0s
template<size_t S>
void Chars<S>::fill_remaining()
{
    if(length_ < SIZE - 1)
        memset(buffer_ + length_ + 1, 0, SIZE - length_ - 1);
}

//! Assign a new value to this Chars from a string of characters. It is truncated if it does not fit into the Chars.
//! @tparam S       The maximum size of the Chars
//! @param name     The value to be copied into this Chars
//! @return         Itself
template<size_t S>
Chars<S>& Chars<S>::operator=(const char* name)
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
//! @tparam S           The maximum size of the Chars
//! @param duration     The value to be copied (after transformation) into this Chars
//! @return             Itself
template<size_t S>
Chars<S>& Chars<S>::operator=(duration_t duration)
{
    auto char_buffer = reinterpret_cast<char*>(buffer_);
    duration.toString(char_buffer);
    length_ = strlen(char_buffer);
    fill_remaining();
    return *this;
}

//! Append a string of character to this Chars. It is truncated if it does not fit into the Chars.
//! @tparam S           The maximum size of the Chars
//! @param value        The value to be append to this Chars
//! @return             Itself
template<size_t S>
Chars<S>& Chars<S>::operator<<(const char* value)
{
    auto value_length = strlen(value);
    auto actual_value_length = value_length < SIZE - 1 - length_ ? value_length : SIZE - 1 - length_;

    if(actual_value_length > 0)
        memcpy(buffer_ + length_, value, actual_value_length);

    length_ += actual_value_length;

    buffer_[length_] = 0;
    fill_remaining(); // TODO: Is it required in this case?

    return *this;
}

//! Append a decimal value to this Chars. It is truncated if it does not fit into the Chars.
//! @tparam S           The maximum size of the Chars
//! @param value        The value to be append to this Chars (after transformation into a string)
//! @return             Itself
template<size_t S>
Chars<S>& Chars<S>::operator<<(uint16_t value)
{
    char buffer[32];
    sprintf(buffer, "%d", value);
    return (*this << buffer);
}

//! Append a Command to this Chars. It is truncated if it does not fit into the Chars.
//! @tparam S           The maximum size of the Chars
//! @param command      The command to be append to this Chars (after transformation into a string)
//! @return             Itself
template<size_t S>
Chars<S>& Chars<S>::operator<<(Command command)
{
    return (*this << static_cast<uint8_t>(command));
}

//! Append a Register to this Chars. It is truncated if it does not fit into the Chars.
//! @tparam S           The maximum size of the Chars
//! @param reg          The register to be append to this Chars (after transformation into a string)
//! @return             Itself
template<size_t S>
Chars<S>&  Chars<S>::operator<<(Register reg)
{
    return (*this << static_cast<uint8_t>(reg));
}

//! Append a Variable to this Chars. It is truncated if it does not fit into the Chars.
//! @tparam S           The maximum size of the Chars
//! @param var          The variable to be append to this Chars (after transformation into a string)
//! @return             Itself
template<size_t S>
Chars<S>&  Chars<S>::operator<<(Variable var)
{
    return (*this << static_cast<uint16_t>(var));
}

template<size_t S>
void Chars<S>::vprintf(const char * const fmt, va_list args)
{
    vsnprintf(reinterpret_cast<char*>(buffer_), SIZE, fmt, args);
    length_ = strlen(reinterpret_cast<const char*>(buffer_));
    fill_remaining();
}


// --------------------------------------------------------------------
// Frame
// --------------------------------------------------------------------

//! Append a Chars to this Frame.
//! @tparam S           The size of the Chars
//! @param name         The name to be append
//! @return             Itself
template<size_t S>
Frame& Frame::operator<<(const Chars<S>& name)
{
    size_t length = position_ + name.size() < FRAME_BUFFER_SIZE ? name.size() : FRAME_BUFFER_SIZE - position_;
    // TODO: Truncate
    memcpy(buffer_ + position_, name.get(), length);
    position_ += length;
    buffer_[Position::Length] += length;
    return *this;
}


#endif //MARLIN_ADV_I3_PLUS_PLUS_UTILS_INLINE_H
