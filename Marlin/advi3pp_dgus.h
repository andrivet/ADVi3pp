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
#ifndef ADV_I3_PLUS_PLUS_DGUS_H
#define ADV_I3_PLUS_PLUS_DGUS_H

#ifndef ADVi3PP_UNIT_TEST
#include "Marlin.h"
#endif
#include "duration_t.h"
#include "ADVstring.h"

namespace advi3pp {


//! List of commands and their values (DGUS Mini)
enum class Command: uint8_t
{
    WriteRegisterData       = 0x80, // 128
    ReadRegisterData        = 0x81, // 129
    WriteRamData            = 0x82, // 130
    ReadRamData             = 0x83, // 131
    WriteCurveData          = 0x84  // 132
};

//! List of registers and their values (DGUS Mini)
enum class Register: uint8_t
{
    Version                 = 0x00,
    Brightness              = 0x01,
    BuzzerBeepingTime       = 0x02,
    PictureID               = 0x03,
    TouchPanelFlag          = 0x05,
    TouchPanelStatus        = 0x06,
    TouchPanelPosition      = 0x07,
    TouchPanelEnable        = 0x0B,
    RunTime                 = 0x0C,
    R0                      = 0x10,
    R1                      = 0x11,
    R2                      = 0x12,
    R3                      = 0x13,
    R4                      = 0x14,
    R5                      = 0x15,
    R6                      = 0x16,
    R7                      = 0x17,
    R8                      = 0x18,
    R9                      = 0x19,
    RA                      = 0x1A,
    RtcComAdj               = 0x1F,
    RtcNow                  = 0x20,
    EnLibOP                 = 0x40,
    LibOPMode               = 0x41,
    LibID                   = 0x42,
    LibAddress              = 0x43,
    VP                      = 0x46,
    OPLength                = 0x48,
    Timer0                  = 0x4A,
    Timer1                  = 0x4C,
    Timer2                  = 0x4D,
    Timer3                  = 0x4E,
    KeyCode                 = 0x4F,
    TrendlineClear          = 0xEB,
    ResetTrigger            = 0xEE
};

enum class Variable: uint16_t;
enum class Action: uint16_t;
enum class KeyValue: uint16_t;
enum class Page: uint8_t;

// --------------------------------------------------------------------
// Uint8
// --------------------------------------------------------------------

//! An unsigned 8 bits value.
struct Uint8
{
    uint8_t byte{}; //!< The actual value
    constexpr explicit Uint8(uint8_t value = 0) : byte{value} {}
    constexpr explicit Uint8(Register reg) : byte{static_cast<uint8_t>(reg)} {}
    constexpr explicit Uint8(Page page) : byte{static_cast<uint8_t>(page)} {}
};

//! An unsigned 8 bits literal such as: 0_u8.
constexpr Uint8  operator "" _u8(unsigned long long int byte)  { return Uint8(static_cast<uint8_t>(byte)); }

// --------------------------------------------------------------------
// Uint16
// --------------------------------------------------------------------

//! An unsigned 16 bits value.
struct Uint16
{
    uint16_t word; //!< The actual value
    constexpr explicit Uint16(uint16_t value = 0) : word{value} {}
    constexpr explicit Uint16(int16_t value) : word{static_cast<uint16_t>(value)} {}
    constexpr explicit Uint16(long value) : word{static_cast<uint16_t>(value)} {}
    constexpr explicit Uint16(double value) : word{static_cast<uint16_t>(value)} {}
	constexpr explicit Uint16(bool value) : word{static_cast<uint16_t>(value)} {}
    constexpr explicit Uint16(Variable var) : word{static_cast<uint16_t>(var)} {}
};

//! An unsigned 16 bits literal such as: 0_u16.
constexpr Uint16 operator "" _u16(unsigned long long int word) { return Uint16(static_cast<uint16_t>(word)); }

// --------------------------------------------------------------------
// Uint32
// --------------------------------------------------------------------

//! An unsigned 32 bits value.
struct Uint32
{
    uint32_t dword; //!< The actual value
    constexpr explicit Uint32(uint32_t value = 0) : dword{value} {}
    constexpr explicit Uint32(int32_t value) : dword{static_cast<uint32_t>(value)} {}
    constexpr explicit Uint32(double value) : dword{static_cast<uint32_t>(value)} {}
};

//! An unsigned 32 bits literal such as: 0_u32.
constexpr Uint32 operator "" _u32(unsigned long long int dword) { return Uint32(static_cast<uint32_t>(dword)); }


// --------------------------------------------------------------------
// Frame
// --------------------------------------------------------------------

//! A frame to be send to the LCD or received from the LCD
struct Frame
{
    bool send(bool logging = true); // Logging is only used in DEBUG builds

    bool available(uint8_t bytes = 3);
    bool receive(bool logging = true); // Logging is only used in DEBUG builds
    Command get_command() const;
    size_t get_length() const;
    void reset();

    friend Frame& operator<<(Frame& frame, const Uint8& data);
    friend Frame& operator<<(Frame& frame, const Uint16& data);
    friend Frame& operator<<(Frame& frame, const Uint32& data);
    friend Frame& operator<<(Frame& frame, Page page);
    friend Frame& operator<<(Frame& frame, const char* s);
	template<size_t L> friend Frame& operator<<(Frame& frame, const ADVString<L>& data) { frame << data.get(); return frame; }

    friend Frame& operator>>(Frame& frame, Uint8& data);
    friend Frame& operator>>(Frame& frame, Uint16& data);
    friend Frame& operator>>(Frame& frame, Uint32& data);
    friend Frame& operator>>(Frame& frame, Action& action);
    friend Frame& operator>>(Frame& frame, Command& command);
    friend Frame& operator>>(Frame& frame, Register& reg);
    friend Frame& operator>>(Frame& frame, Variable& var);

#ifdef ADVi3PP_UNIT_TEST
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
    struct Position { enum { Header0 = 0, Header1 = 1, Length = 2, Command = 3, Data = 4, Register = 4, Variable = 4,
    		NbBytes = 5, NbWords = 6 }; };

    uint8_t buffer_[FRAME_BUFFER_SIZE] = {};
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
    bool receive(Register reg, uint8_t nb_bytes, bool log = true);  // Logging is only used in DEBUG builds
    bool receive(const ReadRegisterDataRequest& request, bool log = true);  // Logging is only used in DEBUG builds
};

// --------------------------------------------------------------------
// ReadRegister (Request and Response)
// --------------------------------------------------------------------

struct ReadRegister: ReadRegisterDataResponse
{
    ReadRegister(Register reg, uint8_t nb_bytes);
    bool send_and_receive(bool log = true); // Logging is only used in DEBUG builds

private:
    ReadRegisterDataRequest request;
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
// ReadRamData (Request and Response)
// --------------------------------------------------------------------

struct ReadRamData: ReadRamDataResponse
{
    ReadRamData(Variable var, uint8_t nb_words);
    bool send_and_receive();

private:
    ReadRamDataRequest request;
};


// --------------------------------------------------------------------
// WriteCurveDataRequest
// --------------------------------------------------------------------

struct WriteCurveDataRequest: Frame
{
    explicit WriteCurveDataRequest(uint8_t channels);
};

// --------------------------------------------------------------------

}

#endif //ADV_I3_PLUS_PLUS_DGUS_H
