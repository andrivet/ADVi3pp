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
struct Log
{
    struct EndOfLine {};

    Log& operator<<(const String& data);
    Log& operator<<(uint8_t data);
    Log& operator<<(uint16_t data);
    Log& operator<<(double data);
    void operator<<(EndOfLine eol);

    inline static Log& log() { return logging_; }
    static Log& error();
    inline static EndOfLine endl() { return EndOfLine{}; }
    static void dump(const uint8_t* bytes, size_t size);

private:
    static Log logging_;
};

void __assert (const char *msg, const char *file, uint16_t line);
#define assert(E) (void)((E) || (__assert (#E, __FILE__, __LINE__), 0))

#else
struct Log
{
    struct EndOfLine {};

    inline Log& operator<<(const String& data) { return log(); }
    inline Log& operator<<(uint8_t data) { return log(); }
    inline Log& operator<<(uint16_t data) { return log(); }
    inline Log& operator<<(double data) { return log(); }
    inline void operator<<(EndOfLine eol) {};

    inline static Log& log() { return logging_; }
    inline static Log& error() { return log(); }
    inline static EndOfLine endl() { return EndOfLine{}; }
    inline static void dump(const uint8_t* bytes, size_t size) {}

private:
    static Log logging_;
};

#define assert(E) (void)(false)

#endif

inline String& operator<<(String& rhs, const __FlashStringHelper* lhs) { rhs += lhs; return rhs; }
inline String& operator<<(String& rhs, const String& lhs) { rhs += lhs; return rhs; }

String& operator<<(String& rhs, uint16_t lhs);
String& operator<<(String& rhs, Command lhs);
String& operator<<(String& rhs, Register lhs);
String& operator<<(String& rhs, Variable lhs);

// --------------------------------------------------------------------
// Stack - Simple Stack
// --------------------------------------------------------------------

template<typename T, size_t S>
struct Stack
{
    void push(T e);
    T pop();
    bool is_empty() const;
    bool contains(T e) const;

private:
    T elements_[S];
    size_t top_ = 0;
};

template<typename T, size_t S>
void Stack<T, S>::push(T e)
{
    assert(top_ <= S);
    elements_[top_++] = e;
};

template<typename T, size_t S>
T Stack<T, S>::pop()
{
    assert(!is_empty());
    return elements_[--top_];
};

template<typename T, size_t S>
bool Stack<T, S>::is_empty() const
{
    return top_ == 0;
};

template<typename T, size_t S>
bool Stack<T, S>::contains(T e) const
{
    for(size_t i = 0; i < top_; ++i)
        if(elements_[top_ - i - 1] == e)
            return true;
    return false;
};


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
// FixedSizeString
// --------------------------------------------------------------------

class Frame;

struct FixedSizeString
{
    FixedSizeString(const String& str, size_t size);
    explicit FixedSizeString(duration_t duration, size_t size);

    inline size_t length() const { return string_.length(); }

    friend Frame& operator<<(Frame& frame, const FixedSizeString& data);

private:
	void assign(const char* str, size_t size);

private:
    String string_;
};


// --------------------------------------------------------------------
// Frame
// --------------------------------------------------------------------

//! A frame to be send to the LCD or received from the LCD
struct Frame
{
    void send(bool logging = true); // Logging is only used in DEBUG builds

    bool available(uint8_t bytes = 3);
    bool receive();
    Command get_command() const;
    size_t get_length() const;
    void reset();

    friend Frame& operator<<(Frame& frame, const Uint8& data);
    friend Frame& operator<<(Frame& frame, const Uint16& data);
    friend Frame& operator<<(Frame& frame, const String& data);
    friend Frame& operator<<(Frame& frame, const FixedSizeString& data);
    friend Frame& operator<<(Frame& frame, Page page);

    friend Frame& operator>>(Frame& frame, Uint8& data);
    friend Frame& operator>>(Frame& frame, Uint16& data);
    friend Frame& operator>>(Frame& frame, Action& action);
    friend Frame& operator>>(Frame& frame, Command& command);
    friend Frame& operator>>(Frame& frame, Register& reg);
    friend Frame& operator>>(Frame& frame, Variable& var);

#ifdef UNIT_TEST
    const uint8_t* get_data() const;
#endif

protected:
    Frame();
    explicit Frame(Command command);
    void reset(Command command);
    friend Frame& operator<<(Frame& frame, Register reg);
    friend Frame& operator<<(Frame& frame, Variable var);

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


}}


#endif //ADV_I3_PLUS_PLUS_UTILS_H
