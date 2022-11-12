
namespace ADVi3pp {

// --------------------------------------------------------------------
// Dgus
// --------------------------------------------------------------------

#ifdef ADV_UNIT_TESTS
template<size_t S>
void Dgus::reset(const uint8_t (&buffer)[S]) {
  reset();
  Serial2.reset(buffer);
}
#endif

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
  assert(data_size_ == 0);
  data_size_ = data_size + sizeof(Param) + 1;
  data_written_ = 1; // For the Command (1 byte)
#endif

  return dgus.write_header(cmd, sizeof(Param), data_size) && write_parameter();
}

template<typename Param, Command cmd>
inline OutFrame<Param, cmd>::~OutFrame() {
  Log::frame() << Log::endl();
#ifdef ADVi3PP_DEBUG
  if(data_written_ != data_size_) {
    Log::error() << F("OutFrame") << data_size_ << F("expected") << data_written_ << F("written.") << Log::endl();
    debug_break();
  }
#endif
}

template<typename Param, Command cmd>
inline bool OutFrame<Param, cmd>::write_byte_data(uint8_t byte) const {
  if(!dgus.write_byte(byte))
    return false;
  check_overflow(1);
  return true;
}

template<typename Param, Command cmd>
inline bool OutFrame<Param, cmd>::write_bytes_data(const uint8_t *bytes, size_t length) const {
  if(!dgus.write_bytes(bytes, length))
    return false;
  check_overflow(length);
  return true;
}

template<typename Param, Command cmd>
inline bool OutFrame<Param, cmd>::write_bytes_data(const char *bytes, size_t length) const {
  if(!dgus.write_bytes(bytes, length))
    return false;
  check_overflow(length);
  return true;
}

template<typename Param, Command cmd>
inline bool OutFrame<Param, cmd>::write_word_data(uint16_t word) const {
    if(!dgus.write_word(word))
        return false;
    check_overflow(2);
    return true;
}

template<typename Param, Command cmd>
inline bool OutFrame<Param, cmd>::write_words_data(const uint16_t *words, size_t length) const {
  if(!dgus.write_words(words, length))
    return false;
  check_overflow(2 * length);
  return true;
}

template<typename Param, Command cmd>
inline bool OutFrame<Param, cmd>::write_text(const char* text, size_t text_length, size_t total_length) const {
  if(!dgus.write_text(text, text_length, total_length))
    return false;
  check_overflow(total_length);
  return true;
}

template<typename Param, Command cmd>
inline bool OutFrame<Param, cmd>::write_centered_text(const char* text, size_t text_length, size_t total_length) const {
  if(!dgus.write_centered_text(text, text_length, total_length))
    return false;
  check_overflow(total_length);
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
bool WriteOutFrame<Param, cmd>::write_byte(uint8_t value) {
  return Parent::write_header(1) && Parent::write_byte_data(value);
}

template<typename Param, Command cmd>
bool WriteOutFrame<Param, cmd>::write_word(uint16_t value) {
  return Parent::write_header(2) && Parent::write_word_data(value);
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

template<size_t N>
bool WriteRamRequest::write_text(const ADVString<N>& data) {
  return Parent::write_header(N) && Parent::write_text(data.get(), data.length(), N);
}

template<size_t N>
bool WriteRamRequest::write_centered_text(const ADVString<N>& data) {
  return Parent::write_header(N) && Parent::write_centered_text(data.get(), data.length(), N);
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
  if(!dgus.receive(cmd, blocking && mode == ReceiveMode::Known) || !read_parameter())
    return false;
  nb_data_expected_ = dgus.read_byte();
  return true;
}

template<typename Param, Command cmd, ReceiveMode mode>
uint8_t InFrame<Param, cmd, mode>::read_byte() {
  nb_data_read_ += 1;
  return dgus.read_byte();
}

template<typename Param, Command cmd, ReceiveMode mode>
uint16_t InFrame<Param, cmd, mode>::read_word() {
  nb_data_read_ += 2 / sizeof(Param);
  auto byte0 = dgus.read_byte();
  auto byte1 = dgus.read_byte();
  return adv::word_from_bytes(byte0, byte1);
}

template<typename Param, Command cmd, ReceiveMode mode>
int16_t InFrame<Param, cmd, mode>::read_signed_word() {
  return static_cast<int16_t>(read_word());
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
  if(!dgus.wait_for_data(1, mode == ReceiveMode::Known))
    return false;
  parameter_ = static_cast<Param>(dgus.read_byte());
  return true;
}

template<typename Param, Command cmd, ReceiveMode mode>
bool InFrame<Param, cmd, mode>::read_word_parameter() {
  if(!dgus.wait_for_data(2, mode == ReceiveMode::Known))
    return false;
  auto byte0 = dgus.read_byte();
  auto byte1 = dgus.read_byte();
  parameter_ = static_cast<Param>(adv::word_from_bytes(byte0, byte1));
  return true;
}

template<typename Param, Command cmd, ReceiveMode mode>
bool InFrame<Param, cmd, mode>::check_byte_parameter() const {
  if(!dgus.wait_for_data(1, mode == ReceiveMode::Known))
    return false;
  auto byte = dgus.read_byte();
  auto parameter = static_cast<Param>(byte);
  if(parameter != parameter_) {
    dgus.push_back(byte);
    return false;
  }
  return true;
}

template<typename Param, Command cmd, ReceiveMode mode>
bool InFrame<Param, cmd, mode>::check_word_parameter() const {
  if(!dgus.wait_for_data(2, mode == ReceiveMode::Known))
    return false;
  auto byte0 = dgus.read_byte();
  auto byte1 = dgus.read_byte();
  auto parameter = static_cast<Param>(adv::word_from_bytes(byte0, byte1));
  if(parameter != parameter_) {
    dgus.push_back(byte1);
    dgus.push_back(byte0);
    return false;
  }
  return true;
}

// --------------------------------------------------------------------
// OutInFrame
// --------------------------------------------------------------------

template<typename Param, Command cmd, ReceiveMode mode>
bool OutInFrame<Param, cmd, mode>::send_receive(uint8_t nb_elements) {
  if(!ReadOutFrame<Param, cmd>{Parent::parameter_}.write(nb_elements))
    return false;
  return Parent::receive();
}

// --------------------------------------------------------------------

}
