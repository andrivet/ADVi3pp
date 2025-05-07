/**
 * ADVi3++ Firmware For Wanhao Duplicator i3 Plus (based on Marlin 2)
 *
 * Copyright (C) 2017-2025 Sebastien Andrivet [https://github.com/andrivet/]
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
namespace ADVi3pp {

  // --------------------------------------------------------------------
  // OutFrame
  // --------------------------------------------------------------------

  #ifdef ADVi3PP_DEBUG
  template<typename Param, Command cmd>
  inline void OutFrame<Param, cmd>::check_overflow(size_t size) const {
    uint8_t total = data_written_ + size;
    if(total > data_size_) {
      Log::error() << F("Overflow") << data_size_ << F("expected") << total << F("written.") << Log::endl();
      debug_break();
    }
    data_written_ += size;
  }
  #else
  template<typename Param, Command cmd>
  inline void OutFrame<Param, cmd>::check_overflow(size_t size) const {}
  #endif

  template<typename Param, Command cmd>
  bool OutFrame<Param, cmd>::write_header(uint8_t data_size)
  {
  #ifdef ADVi3PP_DEBUG
    if(data_size_ != 0) {
      Log::error() << F("OutFrame 0 expected") << data_size << F("found.");
      debug_break();
    }
    data_size_ = data_size + sizeof(Param) + 1;
    data_written_ = 1; // For the Command (1 byte)
  #endif

    return Dgus::write_header(cmd, sizeof(Param), data_size) && write_parameter();
  }

  template<typename Param, Command cmd>
  inline OutFrame<Param, cmd>::~OutFrame() {
    Log::verbose() << Log::endl();
  #ifdef ADVi3PP_DEBUG
    if(data_written_ != data_size_) {
      Log::error() << F("OutFrame") << data_size_ << F("expected") << data_written_ << F("written.") << Log::endl();
      debug_break();
    }
  #endif
  }

  template<typename Param, Command cmd>
  inline bool OutFrame<Param, cmd>::write_byte_data(uint8_t byte) const {
    if(!Dgus::write_byte(byte)) return false;
    check_overflow(1);
    return true;
  }

  template<typename Param, Command cmd>
  inline bool OutFrame<Param, cmd>::write_bytes_data(const uint8_t *bytes, size_t length) const {
    if(!Dgus::write_bytes(bytes, length)) return false;
    check_overflow(length);
    return true;
  }

  template<typename Param, Command cmd>
  inline bool OutFrame<Param, cmd>::write_bytes_data(const char *bytes, size_t length) const {
    if(!Dgus::write_bytes(bytes, length)) return false;
    check_overflow(length);
    return true;
  }

  template<typename Param, Command cmd>
  inline bool OutFrame<Param, cmd>::write_word_data(uint16_t word) const {
    if(!Dgus::write_word(word)) return false;
    check_overflow(2);
    return true;
  }

  template<typename Param, Command cmd>
  inline bool OutFrame<Param, cmd>::write_words_data(const uint16_t *words, size_t length) const {
    if(!Dgus::write_words(words, length)) return false;
    check_overflow(2 * length);
    return true;
  }

  template<typename Param, Command cmd>
  inline bool OutFrame<Param, cmd>::write_text(const char* text, size_t text_length, size_t field_length) const {
    if(!Dgus::write_text(text, text_length, field_length)) return false;
    check_overflow(field_length);
    return true;
  }

  template<typename Param, Command cmd>
  inline bool OutFrame<Param, cmd>::write_text(const FlashChar *text, size_t text_length, size_t field_length) const {
    if(!Dgus::write_text(text, text_length, field_length)) return false;
    check_overflow(field_length);
    return true;
  }

  template<typename Param, Command cmd>
  inline bool OutFrame<Param, cmd>::write_centered_text(const char* text, size_t text_length, size_t field_length) const {
    if(!Dgus::write_centered_text(text, text_length, field_length)) return false;
    check_overflow(field_length);
    return true;
  }

  template<typename Param, Command cmd>
  inline bool OutFrame<Param, cmd>::write_centered_text(const FlashChar* text, size_t text_length, size_t field_length) const {
    if(!Dgus::write_centered_text(text, text_length, field_length)) return false;
    check_overflow(field_length);
    return true;
  }

  template<typename Param, Command cmd>
  inline bool OutFrame<Param, cmd>::write_parameter() const {
    return sizeof(Param) == 1 ? write_byte_parameter() : write_word_parameter();
  }

  template<typename Param, Command cmd>
  inline bool OutFrame<Param, cmd>::write_byte_parameter() const {
    return write_byte_data(static_cast<uint8_t>(parameter_));
  }

  template<typename Param, Command cmd>
  inline bool OutFrame<Param, cmd>::write_word_parameter() const {
    return write_word_data(static_cast<uint16_t>(parameter_));
  }

  // --------------------------------------------------------------------
  // ReadOutFrame
  // --------------------------------------------------------------------

  template<typename Param, Command cmd>
  bool ReadOutFrame<Param, cmd>::write(uint8_t nb_elements) {
    return Parent::write_header(1) && Parent::write_byte_data(nb_elements);
  }

  // --------------------------------------------------------------------
  // WriteOutFrame
  // --------------------------------------------------------------------

  template<typename Param, Command cmd>
  template<typename T> bool WriteOutFrame<Param, cmd>::write_byte(T value) {
    return Parent::write_header(1) && Parent::write_byte_data(static_cast<uint8_t>(value));
  }

  template<typename Param, Command cmd>
  template<typename T> bool WriteOutFrame<Param, cmd>::write_word(T value) {
    return Parent::write_header(2) && Parent::write_word_data(static_cast<uint16_t>(value));
  }

  template<typename Param, Command cmd>
  bool WriteOutFrame<Param, cmd>::write_bytes_data(const uint8_t *first, size_t size) {
    return Parent::write_header(size) && Parent::write_bytes_data(first, size);
  }

  template<typename Param, Command cmd>
  bool WriteOutFrame<Param, cmd>::write_words_data(const uint16_t *first, size_t size) {
    return Parent::write_header(size * 2) && Parent::write_words_data(first, size);
  }

  template<typename Param, Command cmd>
  template<typename... T>
  bool WriteOutFrame<Param, cmd>::write_bytes(T... args) {
    const auto size = sizeof...(args);
    const adv::array<uint8_t, size> data = {static_cast<uint8_t>(args)...};
    return write_bytes_data(data.data(), size);
  }

  template<typename Param, Command cmd>
  template<typename... T>
  bool WriteOutFrame<Param, cmd>::write_words(T... args) {
    const auto size = sizeof...(args);
    const adv::array<uint16_t, size> data = {static_cast<uint16_t>(args)...};
    return write_words_data(data.data(), size);
  }

  // --------------------------------------------------------------------
  // WriteRamRequest
  // --------------------------------------------------------------------

  inline bool WriteRamRequest::write_text(const char *data, size_t field_length) {
    return Parent::write_header(field_length) && Parent::write_text(data, strlen(data), field_length);
  }

  inline bool WriteRamRequest::write_text(const FlashChar *data, size_t field_length) {
    return Parent::write_header(field_length) && Parent::write_text(data, strlen_P(from_flash(data)), field_length);
  }

  inline bool WriteRamRequest::write_centered_text(const char *data, size_t field_length) {
    return Parent::write_header(field_length) && Parent::write_centered_text(data, strlen(data), field_length);
  }

  inline bool WriteRamRequest::write_centered_text(const FlashChar *data, size_t field_length) {
    return Parent::write_header(field_length) && Parent::write_centered_text(data, strlen_P(from_flash(data)), field_length);
  }

  // --------------------------------------------------------------------
  // InFrame
  // --------------------------------------------------------------------

  template<typename Param, Command cmd, ReceiveMode mode>
  InFrame<Param, cmd, mode>::~InFrame() {
    if(nb_data_expected_ != nb_data_read_) {
      Log::error() << F("InFrame") << nb_data_expected_ << F("expected") << nb_data_read_ << F("read.") << Log::endl();
      debug_break();
    }
  }

  template<typename Param, Command cmd, ReceiveMode mode>
  bool InFrame<Param, cmd, mode>::receive(bool blocking) {
    if(!Dgus::receive(cmd, blocking && mode == ReceiveMode::Known) || !read_parameter())
      return false;
    nb_data_expected_ = Dgus::read_byte();
    return true;
  }

  template<typename Param, Command cmd, ReceiveMode mode>
  uint8_t InFrame<Param, cmd, mode>::read_byte() {
    nb_data_read_ += 1;
    return Dgus::read_byte();
  }

  template<typename Param, Command cmd, ReceiveMode mode>
  uint16_t InFrame<Param, cmd, mode>::read_uint() {
    nb_data_read_ += 2 / sizeof(Param);
    auto byte0 = Dgus::read_byte();
    auto byte1 = Dgus::read_byte();
    return adv::word_from_bytes(byte0, byte1);
  }

  template<typename Param, Command cmd, ReceiveMode mode>
  int16_t InFrame<Param, cmd, mode>::read_int() {
    return static_cast<int16_t>(read_uint());
  }

  template<typename Param, Command cmd, ReceiveMode mode>
  bool InFrame<Param, cmd, mode>::read_bool() {
    return read_uint() != 0;
  }

  template<typename Param, Command cmd, ReceiveMode mode>
  template<typename T, typename adv::enable_if<adv::is_enum<T>::value, int>::type> T InFrame<Param, cmd, mode>::read_enum() {
    return static_cast<T>(read_uint());
  }

  template<typename Param, Command cmd, ReceiveMode mode>
  Param InFrame<Param, cmd, mode>::get_parameter() const {
    return parameter_;
  }

  template<typename Param, Command cmd, ReceiveMode mode>
  uint8_t InFrame<Param, cmd, mode>::get_nb_data() const {
    return nb_data_expected_;
  }

  template<typename Param, Command cmd, ReceiveMode mode>
  bool InFrame<Param, cmd, mode>::read_parameter() {
    if(mode == ReceiveMode::Known)
      return sizeof(Param) == 1 ? check_byte_parameter() : check_word_parameter();
    else
      return sizeof(Param) == 1 ? read_byte_parameter() : read_word_parameter();
  }

  template<typename Param, Command cmd, ReceiveMode mode>
  bool InFrame<Param, cmd, mode>::read_byte_parameter() {
    if(!Dgus::wait_for_data(1, mode == ReceiveMode::Known))
      return false;
    parameter_ = static_cast<Param>(Dgus::read_byte());
    return true;
  }

  template<typename Param, Command cmd, ReceiveMode mode>
  bool InFrame<Param, cmd, mode>::read_word_parameter() {
    if(!Dgus::wait_for_data(2, mode == ReceiveMode::Known))
      return false;
    auto byte0 = Dgus::read_byte();
    auto byte1 = Dgus::read_byte();
    parameter_ = static_cast<Param>(adv::word_from_bytes(byte0, byte1));
    return true;
  }

  template<typename Param, Command cmd, ReceiveMode mode>
  bool InFrame<Param, cmd, mode>::check_byte_parameter() const {
    if(!Dgus::wait_for_data(1, mode == ReceiveMode::Known))
      return false;
    auto byte = Dgus::read_byte();
    auto parameter = static_cast<Param>(byte);
    if(parameter != parameter_) {
      Dgus::push_back(byte);
      return false;
    }
    return true;
  }

  template<typename Param, Command cmd, ReceiveMode mode>
  bool InFrame<Param, cmd, mode>::check_word_parameter() const {
    if(!Dgus::wait_for_data(2, mode == ReceiveMode::Known))
      return false;
    auto byte0 = Dgus::read_byte();
    auto byte1 = Dgus::read_byte();
    auto parameter = static_cast<Param>(adv::word_from_bytes(byte0, byte1));
    if(parameter != parameter_) {
      Dgus::push_back(byte1);
      Dgus::push_back(byte0);
      return false;
    }
    return true;
  }

  // --------------------------------------------------------------------
  // OutInFrame
  // --------------------------------------------------------------------

  template<typename Param, Command cmd, ReceiveMode mode>
  bool OutInFrame<Param, cmd, mode>::send_receive(uint8_t nb_elements) {
    if(!ReadOutFrame<Param, cmd>{Parent::parameter_}.write(nb_elements) || !Parent::receive()) {
      Log::error() << F("Receiving Frame") << Log::endl();
      return false;
    }
    return true;
  }

  // --------------------------------------------------------------------

}
