/**
 * ADVi3++ Unit Tests
 *
 * Copyright (C) 2021 Sebastien Andrivet [https://github.com/andrivet/]
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

#define ADV_UNIT_TESTS
#define ADVi3PP_DEBUG

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include "../lib/dgus.h"

using namespace ADVi3pp;
using namespace Catch::Matchers;

using bytes = std::vector<uint8_t>;


SCENARIO("Write in RAM")
{
  GIVEN("WriteRamRequest frame")
  {
    dgus.reset();
    WriteRamRequest frame{Variable::TargetHotEnd};

    WHEN("When a 16-bit data is written")
    {
      frame.write_word(0x00D2);

      THEN("The Serial has written the expected content")
      {
        REQUIRE_THAT(Serial2.get_written_bytes(), Equals(bytes{
          0x5A, 0xA5, 0x05, 0x82, 0x00, 0x02, 0x00, 0xD2}));
      }
    }

    WHEN("When two 16-bit data are written")
    {
      adv::array<uint16_t, 2> data = {0xD0D1, 0xD2D3};
      frame.write_words(data);

      THEN("The Serial has the expected content")
      {
        REQUIRE_THAT(Serial2.get_written_bytes(), Equals(bytes{
          0x5A, 0xA5, 0x07, 0x82, 0x00, 0x02, 0xD0, 0xD1, 0xD2, 0xD3}));
      }
    }
  }
}

SCENARIO("Write text in RAM")
{
  GIVEN("WriteRamRequest frame")
  {
    dgus.reset();
    WriteRamRequest frame{Variable::Message};

    WHEN("When a text is written")
    {
      frame.write_text(ADVString<4>("Test"));

      THEN("The Serial has written the expected content")
      {
        REQUIRE_THAT(Serial2.get_written_bytes(), Equals(bytes{
          0x5A, 0xA5, 07, 0x82, 0x00, 0x10, 0x54, 0x65, 0x73, 0x74}));
      }
    }
    WHEN("When a long text is written")
    {
      frame.write_text(ADVString<44>("12345678901234567890123456789012345678901234"));

      THEN("The Serial has written the expected content")
      {
        REQUIRE_THAT(Serial2.get_written_bytes(), Equals(bytes{
          0x5a, 0xa5, 0x2F, 0x82, 0x00, 0x10, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
          0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32,
          0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
          0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34
        }));
      }
    }
    WHEN("When a centered text is written")
    {
      frame.write_centered_text(ADVString<8>("Test"));

      THEN("The Serial has written the expected content")
      {
        REQUIRE_THAT(Serial2.get_written_bytes(), Equals(bytes{
          0x5A, 0xA5, 11, 0x82, 0x00, 0x10, ' ', ' ', 'T', 'e', 's', 't', ' ', ' '}));
      }
    }

    WHEN("An empty test is written")
    {
        frame.write_text(ADVString<8>(""));

        THEN("The Serial has written the expected content")
        {
            REQUIRE_THAT(Serial2.get_written_bytes(), Equals(bytes{
                0x5A, 0xA5, 11, 0x82, 0x00, 0x10, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}));
        }
    }
  }
}

SCENARIO("Read from RAM")
{
  GIVEN("A ReadRamRequest frame")
  {
    dgus.reset();
    ReadRamRequest frame{Variable::Value1};

    WHEN("When 7 bytes are requested")
    {
      frame.write(7);

      THEN("The frame has the expected content")
      {
        REQUIRE_THAT(Serial2.get_written_bytes(), Equals(bytes{0x5A, 0xA5, 0x04, 0x83, 0x03, 0x01, 0x07}));
      }
    }
  }

  GIVEN("A ReadRamResponse simulated response")
  {
    dgus.reset({0x5A, 0xA5, 0x06, 0x83, 0x04, 0x60, 0x01, 0x01, 0x50});
    ReadRamResponse response{static_cast<Variable>(0x0460)};
    WHEN("Data are received")
    {
      REQUIRE(response.receive());

      THEN("The response has the expected content")
      {
        REQUIRE(response.get_nb_words() == 1);
        REQUIRE(response.read_word() == 0x0150);
      }
    }
  }
}

SCENARIO("Write to a register")
{
  GIVEN("A WriteRegisterRequest frame")
  {
    dgus.reset();
    WriteRegisterRequest frame{Register::PictureID};

    WHEN("When a byte data is written")
    {
      frame.write_byte(68);

      THEN("The Serial has the expected content")
      {
        REQUIRE_THAT(Serial2.get_written_bytes(), Equals(bytes{0x5A, 0xA5, 0x03, 0x80, 0x03, 0x044}));
      }
    }
  }
}

SCENARIO("Read a byte from a register")
{
  GIVEN("A ReadRegisterDataRequest frame")
  {
    dgus.reset();
    ReadRegisterRequest frame{Register::Version};

    WHEN("The command is sent")
    {
      frame.write(1); // 1 byte

      THEN("The Serial has the expected content")
      {
        REQUIRE_THAT(Serial2.get_written_bytes(), Equals(bytes{0x5A, 0xA5, 0x03, 0x81, 0x00, 0x01}));
      }
    }
  }

  GIVEN("A ReadRegisterResponse frame and a simulated response")
  {
    dgus.reset({0x5A, 0xA5, 0x04, 0x81, 0x00, 0x01, 0x22});
    ReadRegisterResponse response{Register::Version};

    WHEN("Data are received")
    {
      REQUIRE(response.receive());

      THEN("The response has the expected content")
      {
        REQUIRE(response.get_nb_bytes() == 1);
        REQUIRE(response.read_byte() == 0x22);
      }
    }
  }
}

SCENARIO("Read a word from a register")
{
  GIVEN("A ReadRegisterRequest frame")
  {
    dgus.reset();
    ReadRegisterRequest frame{Register::PictureID};

    WHEN("The command is sent")
    {
      frame.write(2); // 1 byte

      THEN("The Serial has the expected content")
      {
        REQUIRE_THAT(Serial2.get_written_bytes(), Equals(bytes{0x5A, 0xA5, 0x03, 0x81, 0x03, 0x02}));
      }
    }
  }

  GIVEN("A ReadRegisterResponse frame and a simulated response")
  {
    dgus.reset({0x5A, 0xA5, 0x05, 0x81, 0x03, 0x02, 0x00, 0x15});
    ReadRegisterResponse response{Register::PictureID};

    WHEN("Data are received")
    {
      REQUIRE(response.receive());

      THEN("The response has the expected content")
      {
        REQUIRE(response.get_nb_bytes() == 2);
        REQUIRE(response.read_word() == 0x15);
      }
    }
  }

  GIVEN("A wrong ReadRegisterResponse frame and a simulated response")
  {
    dgus.reset({0x5A, 0xA5, 0x05, 0x81, 0x03, 0x02, 0x00, 0x15});
    ReadRegisterResponse response{Register::Version};

    THEN("Data are not received")
    {
      REQUIRE(!response.receive());

      GIVEN("A right ReadRegisterResponse")
      {
        ReadRegisterResponse response2{Register::PictureID};

        THEN("Data are received")
        {
          REQUIRE(response2.receive());

          THEN("The response has the expected content")
          {
            REQUIRE(response2.read_word() == 0x15);
          }
        }
      }
    }
  }
}

SCENARIO("Send a receive a Register")
{
    GIVEN("A simulated response")
    {
        dgus.reset({0x5A, 0xA5, 0x04, 0x81, 0x12, 0x01, 0x55});

        WHEN("A read frame is sent")
        {
            ReadRegister read{Register::R2};
            REQUIRE(read.send_receive(1));

            THEN("The response has the right content")
            {
                REQUIRE(read.read_byte() == 0x55);
            }
        }
    }
}

SCENARIO("Read the right frame")
{
  GIVEN("A simulated response")
  {
    dgus.reset({0x5A, 0xA5, 0x05, 0x81, 0x03, 0x02, 0x00, 0x15});

    WHEN("Trying to read it as a read RAM response")
    {
      ReadRamResponse response{Variable::Value1};
      THEN("Its fails")
      {
        REQUIRE(!response.receive());

        WHEN("Trying to read it as a read register response")
        {
          ReadRegisterResponse response2{Register::PictureID};

          THEN("It succeeds")
          {
            REQUIRE(response2.receive());

            THEN("The response has the expected content")
            {
              REQUIRE(response2.get_nb_bytes() == 2);
              REQUIRE(response2.read_word() == 0x15);
            }
          }
        }
      }
    }
  }
}

SCENARIO("Read the right frame with the right parameter value")
{
  GIVEN("A simulated response")
  {
    dgus.reset({0x5A, 0xA5, 0x05, 0x81, 0x03, 0x02, 0x00, 0x15});

    WHEN("Trying to read it as a read RAM response")
    {
      ReadRamResponse response{Variable::Value1};
      THEN("Its fails")
      {
        REQUIRE(!response.receive());

        WHEN("Trying to read it as a read register response")
        {
          ReadRegisterResponse response2{Register::PictureID};

          THEN("It succeeds")
          {
            REQUIRE(response2.receive());

            THEN("The response has the expected content")
            {
              REQUIRE(response2.get_nb_bytes() == 2);
              REQUIRE(response2.read_word() == 0x15);
            }
          }
        }
      }
    }
  }
}

SCENARIO("Read an incoming frame")
{
  GIVEN("A simulated response")
  {
    dgus.reset({0x5A, 0xA5, 0x06, 0x83, 0x04, 0x00, 0x01, 0x00, 0x02});

    WHEN("Reading the frame")
    {
      ReadAction frame{};
      REQUIRE(frame.receive());

      THEN("It has the right values")
      {
        CHECK(frame.get_parameter() == Action::Controls);
        CHECK(frame.read_key_value() == KeyValue::Controls);
      }
    }
  }
}
