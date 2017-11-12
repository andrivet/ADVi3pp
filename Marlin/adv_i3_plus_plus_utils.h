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
void Dump(const uint8_t* bytes, size_t size);
#define ADVi3PP_ERROR(expression) {Chars<100> message{"*** ERROR "}; message << expression; Serial.println(message.c_str());} while(false)
#define ADVi3PP_LOG(expression)   {Chars<100> message; message << expression; Serial.println(message.c_str());} while(false)
#define ADVi3PP_DUMP(bytes, size)  {Dump(bytes, size);}
#else
#define ADVi3PP_ERROR(expression) {} while(false)
#define ADVi3PP_LOG(expression)   {} while(false)
#define ADVi3PP_DUMP(bytes, size)  {} while(false)
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
// Chars
// --------------------------------------------------------------------

//! A fixed-size string of characters, truncating values when necessary.
template<size_t S = 26>
class Chars
{
public:
    Chars();
    explicit Chars(const char* name);
    explicit Chars(duration_t duration);

    Chars& operator=(const char* name);
    Chars& operator=(duration_t duration);

    const uint8_t* get() const { return buffer_; }
    constexpr size_t size() const { return S; }
    uint16_t length() const { return length_; }
    const char* c_str() const { return reinterpret_cast<const char*>(buffer_); }
    void vprintf(const char *fmt, va_list args);

    Chars& operator<<(const char* value);
    Chars& operator<<(uint16_t value);
    Chars& operator<<(Command command);
    Chars& operator<<(Register reg);
    Chars& operator<<(Variable var);

private:
    void fill_remaining();

private:
    static constexpr auto SIZE = S > 1 ? S : 1; // Avoid 0 size
    uint8_t buffer_[SIZE]; //!< Fixed-sized buffer holding the content of the Chars
    uint16_t length_; //!< Length of the Chars content
};

// --------------------------------------------------------------------
// Message
// --------------------------------------------------------------------

//! Message to be displayed on the LCD
using Message = Chars<26>;

// --------------------------------------------------------------------
// Frame
// --------------------------------------------------------------------

//! A frame to be send to the LCD or received from the LCD
struct Frame
{
    void send(bool logging = true); // Logging is only used in DEBUG builds
    Frame& operator<<(const Uint8& data);
    Frame& operator<<(const Uint16& data);
    template<size_t S> Frame& operator<<(const Chars<S>& name);
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
    Frame();
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
// IncomingFrame
// --------------------------------------------------------------------

struct IncomingFrame: Frame
{
    IncomingFrame(): Frame{} {}
};

// --------------------------------------------------------------------
// WriteRegisterDataRequest
// --------------------------------------------------------------------

struct WriteRegisterDataRequest: Frame
{
    explicit WriteRegisterDataRequest(Register reg);
};

// --------------------------------------------------------------------
// ReadRegisterDataRequest
// --------------------------------------------------------------------

struct ReadRegisterDataRequest: Frame
{
    ReadRegisterDataRequest(Register reg, uint8_t nb_bytes);
    Register get_register() const;
    uint8_t get_nb_bytes() const;
};

// --------------------------------------------------------------------
// ReadRegisterDataResponse
// --------------------------------------------------------------------

struct ReadRegisterDataResponse: Frame
{
    ReadRegisterDataResponse() = default;
    bool receive(Register reg, uint8_t nb_bytes);
    bool receive(const ReadRegisterDataRequest& request);
};

// --------------------------------------------------------------------
// WriteRamDataRequest
// --------------------------------------------------------------------

struct WriteRamDataRequest: Frame
{
    explicit WriteRamDataRequest(Variable var);
    void reset(Variable var);
};

// --------------------------------------------------------------------
// ReadRamDataRequest
// --------------------------------------------------------------------

struct ReadRamDataRequest: Frame
{
    ReadRamDataRequest(Variable var, uint8_t nb_words);
    Variable get_variable() const;
    uint8_t get_nb_words() const;
};

// --------------------------------------------------------------------
// ReadRamDataResponse
// --------------------------------------------------------------------

struct ReadRamDataResponse: Frame
{
    ReadRamDataResponse() = default;
    bool receive(Variable var, uint8_t nb_words);
    bool receive(const ReadRamDataRequest& request);
};

// --------------------------------------------------------------------
// WriteCurveDataRequest
// --------------------------------------------------------------------

struct WriteCurveDataRequest: Frame
{
    explicit WriteCurveDataRequest(uint8_t channels);
};


#include "adv_i3_plus_plus_utils.inline.h"


}}


#endif //ADV_I3_PLUS_PLUS_UTILS_H
