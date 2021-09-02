/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2021 Sebastien Andrivet [https://github.com/andrivet/]
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

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "string.h"
#include "../lib/ADVstd/array.h"
#include "../lib/ADVstd/endian.h"
#include "../lib/ADVstd/bitmasks.h"
#include "logging.h"

namespace ADVi3pp {

//! List of commands and their values (DGUS Mini)
enum class Command: uint8_t
{
    None                    = 0x00,
    WriteRegister           = 0x80, // 128
    ReadRegister            = 0x81, // 129
    WriteRam                = 0x82, // 130
    ReadRam                 = 0x83, // 131
    WriteCurve              = 0x84  // 132
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
enum class Page: uint16_t;
enum class ReceiveMode { Known, Unknown };

namespace
{
    const uint8_t HEADER_BYTE_0 = 0x5A;
    const uint8_t HEADER_BYTE_1 = 0xA5;
    const size_t MAX_GARBAGE_BYTES = 5;
}

// --------------------------------------------------------------------
// Dgus - DGUS LCD panel
// --------------------------------------------------------------------

struct Dgus
{
    void open();
    void setup();
    [[noreturn]] void forwarding_loop();

#ifdef ADV_UNIT_TESTS
    void reset();
    template<size_t S> void reset(const uint8_t (&buffer)[S]);
#endif

    bool write_header(Command cmd, uint8_t param_size, uint8_t data_size);
    bool wait_for_data(uint8_t size, bool blocking);
    bool receive(Command cmd, bool blocking);

    uint8_t read_byte();
    size_t read_bytes(uint8_t *buffer, size_t length);
    void push_back(uint8_t byte);

    bool write_byte(uint8_t byte);
    bool write_bytes(const uint8_t *bytes, size_t length);
    bool write_bytes(const char *bytes, size_t length);
    bool write_word(uint16_t word);
    bool write_words(const uint16_t *words, size_t length);
    bool write_text(const char* text, size_t text_length, size_t total_length);
    bool write_centered_text(const char* text, size_t text_length, size_t total_length);

private:
    void kill();
    bool receive_header();
    bool has_pushed_back();
    uint8_t get_pushed_back();

    static const size_t MAX_PUSH_BACK = 5;
    enum class State { Start = 0, Command = 1, Data = 2};

    State   state_ = State::Start;
    uint8_t length_ = 0;
    uint8_t read_ = 0;
    Command command_ = Command::None;
    uint8_t nb_pushed_back_ = 0;
    uint8_t pushed_back_[MAX_PUSH_BACK] = {};
};

extern Dgus dgus; // Singleton

// --------------------------------------------------------------------
// OutFrame - Frame sent to the LCD panel
// --------------------------------------------------------------------

template<typename Param, Command>
struct OutFrame
{
    ~OutFrame();

protected:
    explicit OutFrame(Param param): parameter_{param} {}
    bool write_header(uint8_t data_size);
    bool write_parameter() const;
    bool write_byte_data(uint8_t byte) const;
    bool write_bytes_data(const uint8_t *bytes, size_t length) const;
    bool write_bytes_data(const char *bytes, size_t length) const;
    bool write_word_data(uint16_t word) const;
    bool write_words_data(const uint16_t *words, size_t length) const;
    bool write_text(const char* text, size_t text_length, size_t total_length) const;
    bool write_centered_text(const char* text, size_t text_length, size_t total_length) const;

private:
    bool write_byte_parameter() const;
    bool write_word_parameter() const;
    void check_overflow(size_t size) const;

private:
    const Param parameter_{};

#ifdef ADVi3PP_DEBUG
    uint8_t data_size_ = 0;
    mutable uint8_t data_written_ = 0;
#endif
};

// --------------------------------------------------------------------
// ReadOutFrame
// --------------------------------------------------------------------

template<typename Param, Command cmd>
struct ReadOutFrame: OutFrame<Param, cmd>
{
  using Parent = OutFrame<Param, cmd>;
  explicit ReadOutFrame(Param param): Parent{param} {}
  bool write(uint8_t nb_bytes);
};

// --------------------------------------------------------------------
// WriteOutFrame
// --------------------------------------------------------------------

template<typename Param, Command cmd>
struct WriteOutFrame: OutFrame<Param, cmd>
{
protected:
    using Parent = OutFrame<Param, cmd>;
    explicit WriteOutFrame(Param param): Parent{param} {};

    bool write_byte(uint8_t value);
    bool write_word(uint16_t value);

    template<size_t N> bool write_bytes(const adv::array<uint8_t , N>& data);
    template<size_t N> bool write_words(const adv::array<uint16_t , N>& data);
};

// --------------------------------------------------------------------
// InFrame
// --------------------------------------------------------------------

template<typename Param, Command, ReceiveMode mode>
struct InFrame
{
  InFrame() = default;
  ~InFrame();

  bool receive();
  Param get_parameter() const;

protected:
  explicit InFrame(Param param) : parameter_{param} {}

  uint8_t get_nb_data() const;
  uint8_t read_byte();
  uint16_t read_word();

  bool read_parameter();
  bool read_byte_parameter();
  bool read_word_parameter();
  bool check_byte_parameter() const;
  bool check_word_parameter() const;

private:
  uint8_t nb_data_expected_{};
  uint8_t nb_data_read_{};

protected:
  Param parameter_{};
};


// --------------------------------------------------------------------
// OutInFrame
// --------------------------------------------------------------------

template<typename Param, Command cmd, ReceiveMode mode>
struct OutInFrame: InFrame<Param, cmd, mode>
{
  bool send_receive(uint8_t nb_bytes);

protected:
    using Parent = InFrame<Param, cmd, mode>;
    explicit OutInFrame(Param param): Parent{param} {}
};


// --------------------------------------------------------------------
// WriteRegisterRequest
// --------------------------------------------------------------------

struct WriteRegisterRequest: WriteOutFrame<Register, Command::WriteRegister>
{
  using Parent = WriteOutFrame<Register, Command::WriteRegister>;
  explicit WriteRegisterRequest(Register reg): Parent{reg} {}
  using Parent::write_byte;
  bool write_page(Page page) { return write_word(static_cast<uint16_t>(page)); }
};

// --------------------------------------------------------------------
// ReadRegisterRequest
// --------------------------------------------------------------------

struct ReadRegisterRequest: ReadOutFrame<Register, Command::ReadRegister>
{
    using Parent = ReadOutFrame<Register, Command::ReadRegister>;
    explicit ReadRegisterRequest(Register reg): Parent{reg} {}
};

// --------------------------------------------------------------------
// ReadRegisterDataResponse
// --------------------------------------------------------------------

struct ReadRegisterResponse: InFrame<Register, Command::ReadRegister, ReceiveMode::Known>
{
  using Parent = InFrame<Register, Command::ReadRegister, ReceiveMode::Known>;
  explicit ReadRegisterResponse(Register reg): Parent{reg} {}
  uint8_t get_nb_bytes() const { return get_nb_data(); }
  using Parent::read_byte;
  using Parent::read_word;
};

// --------------------------------------------------------------------
// ReadRegister (Request and Response)
// --------------------------------------------------------------------

struct ReadRegister: OutInFrame<Register, Command::ReadRegister, ReceiveMode::Known>
{
  using Parent = OutInFrame<Register, Command::ReadRegister, ReceiveMode::Known>;
  explicit ReadRegister(Register reg): Parent{reg} {}
  using Parent::read_byte;
  using Parent::read_word;
};

// --------------------------------------------------------------------
// WriteRamRequest
// --------------------------------------------------------------------

struct WriteRamRequest: WriteOutFrame<Variable, Command::WriteRam>
{
  using Parent = WriteOutFrame<Variable, Command::WriteRam>;
  explicit WriteRamRequest(Variable var): Parent{var} {}
  using Parent::write_word;
  using Parent::write_words;
  template<size_t N> bool write_text(const ADVString<N>& data);
  template<size_t N> bool write_centered_text(const ADVString<N>& data);
};

// --------------------------------------------------------------------
// ReadRamDataRequest
// --------------------------------------------------------------------

struct ReadRamRequest: ReadOutFrame<Variable, Command::ReadRam>
{
    using Parent = ReadOutFrame<Variable, Command::ReadRam>;
    explicit ReadRamRequest(Variable var): Parent{var} {}
};

// --------------------------------------------------------------------
// ReadRamDataResponse
// --------------------------------------------------------------------

struct ReadRamResponse: InFrame<Variable, Command::ReadRam, ReceiveMode::Known>
{
  using Parent = InFrame<Variable, Command::ReadRam, ReceiveMode::Known>;
  explicit ReadRamResponse(Variable var): Parent{var} {}
  uint16_t get_nb_words() const { return get_nb_data(); }
  using Parent::read_word;
};


// --------------------------------------------------------------------
// ReadAction
// --------------------------------------------------------------------

struct ReadAction: InFrame<Action, Command::ReadRam, ReceiveMode::Unknown>
{
    using Parent = InFrame<Action, Command::ReadRam, ReceiveMode::Unknown>;
    ReadAction(): Parent{} {}
    KeyValue read_key_value() { return static_cast<KeyValue>(read_word()); }
};

// --------------------------------------------------------------------
// ReadRam (Request and Response)
// --------------------------------------------------------------------

struct ReadRam: OutInFrame<Variable, Command::ReadRam, ReceiveMode::Known>
{
    using Parent = OutInFrame<Variable, Command::ReadRam, ReceiveMode::Known>;
    explicit ReadRam(Variable var): Parent{var} {}
    using Parent::read_word;
};


// --------------------------------------------------------------------
// WriteCurveDataRequest
// --------------------------------------------------------------------

struct WriteCurveRequest: WriteOutFrame<uint8_t, Command::WriteCurve>
{
    using Parent = WriteOutFrame<uint8_t, Command::WriteCurve>;
    explicit WriteCurveRequest(uint8_t channels): Parent{channels} {}
    using Parent::write_words;
};

}

#include "dgus.impl"
