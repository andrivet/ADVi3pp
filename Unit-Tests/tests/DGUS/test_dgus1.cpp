/**
 * ADVi3++ Unit Tests
 *
 * Copyright (C) 2018 Sebastien Andrivet [https://github.com/andrivet/]
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

#include "catch.hpp"
#include "advi3pp_dgus.h"
#include "../../vendors/avr/serial.h"

using namespace advi3pp;

SCENARIO("Literals can be declared and have the expected value", "[literal]")
{
    GIVEN("Uint8 literals")
    {
        auto literal1 = 243_u8;
        auto literal2 = 0xAB_u8;

        THEN("They have the expected values")
        {
            REQUIRE(literal1.byte == 243);
            REQUIRE(literal2.byte == 0xAB);
        }
    }

    GIVEN("Uint16 literals")
    {
        auto literal1 = 24378_u16;
        auto literal2 = 0xABCD_u16;

        THEN("They have the expected values")
        {
            REQUIRE(literal1.word == 24378);
            REQUIRE(literal2.word == 0xABCD);
        }
    }
}

SCENARIO("Write in RAM", "[frame]")
{
    GIVEN("WriteRamDataRequest frame")
    {
        WriteRamDataRequest frame{Variable::TargetHotEnd};

        WHEN("When a 16-bit data is written")
        {
            frame << 0x00D2_u16;

            THEN("The frame has the expected command")
            {
                REQUIRE(frame.get_command() == Command::WriteRamData);
            }
            THEN("The frame has the expected length")
            {
                REQUIRE(frame.get_length() == 5);
            }
            THEN("The frame has the expected content")
            {
                REQUIRE(frame.get_data()[0] == 0x5A);
                REQUIRE(frame.get_data()[1] == 0xA5);
                REQUIRE(frame.get_data()[2] == 0x05);
                REQUIRE(frame.get_data()[3] == 0x82);
                REQUIRE(frame.get_data()[4] == 0x00);
                REQUIRE(frame.get_data()[5] == 0x02);
                REQUIRE(frame.get_data()[6] == 0x00);
                REQUIRE(frame.get_data()[7] == 0xD2);
            }
        }

        WHEN("When a 32-bit data is written")
        {
            frame << 0x00D2_u32;

            THEN("The frame has the expected command")
            {
                REQUIRE(frame.get_command() == Command::WriteRamData);
            }
            THEN("The frame has the expected length")
            {
                REQUIRE(frame.get_length() == 7);
            }
            THEN("The frame has the expected content")
            {
                REQUIRE(frame.get_data()[0] == 0x5A);
                REQUIRE(frame.get_data()[1] == 0xA5);
                REQUIRE(frame.get_data()[2] == 0x07);
                REQUIRE(frame.get_data()[3] == 0x82);
                REQUIRE(frame.get_data()[4] == 0x00);
                REQUIRE(frame.get_data()[5] == 0x02);
                REQUIRE(frame.get_data()[6] == 0x00);
                REQUIRE(frame.get_data()[7] == 0x00);
                REQUIRE(frame.get_data()[8] == 0x00);
                REQUIRE(frame.get_data()[9] == 0xD2);
            }
        }
    }
}

SCENARIO("Read from RAM")
{
    GIVEN("A ReadRamDataRequest frame")
    {
        ReadRamDataRequest frame{Variable::Value1, 7};

        THEN("The frame has the expected command")
        {
            REQUIRE(frame.get_command() == Command::ReadRamData);
        }
        THEN("The frame has the expected length")
        {
            REQUIRE(frame.get_length() == 4);
        }
        THEN("The frame has the expected content")
        {
            REQUIRE(frame.get_data()[0] == 0x5A);
            REQUIRE(frame.get_data()[1] == 0xA5);
            REQUIRE(frame.get_data()[2] == 0x04);
            REQUIRE(frame.get_data()[3] == 0x83);
            REQUIRE(frame.get_data()[4] == 0x03);
            REQUIRE(frame.get_data()[5] == 0x01);
            REQUIRE(frame.get_data()[6] == 0x07);
        }
    }

    GIVEN("A ReadRamDataRequest frame and a simulated response")
    {
        ReadRamDataResponse frame;
        WHEN("Data are received")
        {
            static const uint8_t serial_data[] = {0x5A, 0xA5, 0x06, 0x83, 0x04, 0x60, 0x01, 0x01, 0x50};
            Serial2.set_serial_data(serial_data);
            frame.receive(static_cast<Variable>(0x0460), 1);

            THEN("The frame has the expected command")
            {
                REQUIRE(frame.get_command() == Command::ReadRamData);
            }
            THEN("The frame has the expected length")
            {
                REQUIRE(frame.get_length() == 6);
            }
            THEN("The frame has the expected content")
            {
                REQUIRE(frame.get_data()[0] == 0x5A);
                REQUIRE(frame.get_data()[1] == 0xA5);
                REQUIRE(frame.get_data()[2] == 0x06);
                REQUIRE(frame.get_data()[3] == 0x83);
                REQUIRE(frame.get_data()[4] == 0x04);
                REQUIRE(frame.get_data()[5] == 0x60);
                REQUIRE(frame.get_data()[6] == 0x01);
                REQUIRE(frame.get_data()[7] == 0x01);
                REQUIRE(frame.get_data()[8] == 0x50);
            }
            THEN("The payload can be read")
            {
                Uint16 data;
                frame >> data;
                REQUIRE(data.word == 0x0150);
            }
        }
    }
}

SCENARIO("Write to a register")
{
    GIVEN("A WriteRamDataRequest frame")
    {
        WriteRegisterDataRequest frame{Register::PictureID};

        WHEN("When a byte data is written")
        {
            frame << 68_u8;

            THEN("The frame has the expected command")
            {
            REQUIRE(frame.get_command() == Command::WriteRegisterData);
            }
            THEN("The frame has the expected length")
            {
                REQUIRE(frame.get_length() == 3);
            }
            THEN("The frame has the expected content")
            {
                REQUIRE(frame.get_data()[0] == 0x5A);
                REQUIRE(frame.get_data()[1] == 0xA5);
                REQUIRE(frame.get_data()[2] == 0x03);
                REQUIRE(frame.get_data()[3] == 0x80);
                REQUIRE(frame.get_data()[4] == 0x03);
                REQUIRE(frame.get_data()[5] == 0x44);
            }
        }
    }
}

SCENARIO("Read a byte from a register")
{
    GIVEN("A ReadRegisterDataRequest frame")
    {
        ReadRegisterDataRequest frame{Register::Version, 1};

        THEN("The frame has the expected command")
        {
            REQUIRE(frame.get_command() == Command::ReadRegisterData);
        }
        THEN("The frame has the expected length")
        {
            REQUIRE(frame.get_length() == 3);
        }
        THEN("The frame has the expected register")
        {
            REQUIRE(frame.get_register() == Register::Version);
        }
        THEN("The frame has the expected number of payload bytes")
        {
            REQUIRE(frame.get_nb_bytes() == 1);
        }
        THEN("The frame has the expected content")
        {
            REQUIRE(frame.get_data()[0] == 0x5A);
            REQUIRE(frame.get_data()[1] == 0xA5);
            REQUIRE(frame.get_data()[2] == 0x03);
            REQUIRE(frame.get_data()[3] == 0x81);
            REQUIRE(frame.get_data()[4] == 0x00);
            REQUIRE(frame.get_data()[5] == 0x01);
        }
    }

    GIVEN("A ReadRegisterDataRequest frame and a simulated response")
    {
        ReadRegisterDataResponse frame;
        WHEN("Data are received")
        {
            static const uint8_t serial_data[] = {0x5A, 0xA5, 0x04, 0x81, 0x00, 0x01, 0x22};
            Serial2.set_serial_data(serial_data);
            frame.receive(Register::Version, 1);

            THEN("The frame has the expected command")
            {
                REQUIRE(frame.get_command() == Command::ReadRegisterData);
            }
            THEN("The frame has the expected length")
            {
                REQUIRE(frame.get_length() == 4);
            }
            THEN("The frame has the expected content")
            {
                REQUIRE(frame.get_data()[0] == 0x5A);
                REQUIRE(frame.get_data()[1] == 0xA5);
                REQUIRE(frame.get_data()[2] == 0x04);
                REQUIRE(frame.get_data()[3] == 0x81);
                REQUIRE(frame.get_data()[4] == 0x00);
                REQUIRE(frame.get_data()[5] == 0x01);
                REQUIRE(frame.get_data()[6] == 0x22);
            }
            THEN("The payload can be read")
            {
                Uint8 version;
                frame >> version;
                REQUIRE(version.byte == 0x22);
            }
        }
    }
}

SCENARIO("Read a word from a register")
{
    GIVEN("A ReadRegisterDataRequest frame")
    {
        ReadRegisterDataRequest frame{Register::PictureID, 2};

        THEN("The frame has the expected command")
        {
            REQUIRE(frame.get_command() == Command::ReadRegisterData);
        }
        THEN("The frame has the expected length")
        {
            REQUIRE(frame.get_length() == 3);
        }
        THEN("The frame has the expected register")
        {
            REQUIRE(frame.get_register() == Register::PictureID);
        }
        THEN("The frame has the expected number of payload bytes")
        {
            REQUIRE(frame.get_nb_bytes() == 2);
        }
        THEN("The frame has the expected content")
        {
            REQUIRE(frame.get_data()[0] == 0x5A);
            REQUIRE(frame.get_data()[1] == 0xA5);
            REQUIRE(frame.get_data()[2] == 0x03);
            REQUIRE(frame.get_data()[3] == 0x81);
            REQUIRE(frame.get_data()[4] == 0x03);
            REQUIRE(frame.get_data()[5] == 0x02);
        }
    }

    GIVEN("A ReadRegisterDataRequest frame and a simulated response")
    {
        ReadRegisterDataResponse frame;
        WHEN("Data are received")
        {
            static const uint8_t serial_data[] = {0x5A, 0xA5, 0x05, 0x81, 0x03, 0x02, 0x00, 0x15};
            Serial2.set_serial_data(serial_data);
            frame.receive(Register::PictureID, 2);

            THEN("The frame has the expected command")
            {
                REQUIRE(frame.get_command() == Command::ReadRegisterData);
            }
            THEN("The frame has the expected length")
            {
                REQUIRE(frame.get_length() == 5);
            }
            THEN("The frame has the expected content")
            {
                REQUIRE(frame.get_data()[0] == 0x5A);
                REQUIRE(frame.get_data()[1] == 0xA5);
                REQUIRE(frame.get_data()[2] == 0x05);
                REQUIRE(frame.get_data()[3] == 0x81);
                REQUIRE(frame.get_data()[4] == 0x03);
                REQUIRE(frame.get_data()[5] == 0x02);
                REQUIRE(frame.get_data()[6] == 0x00);
                REQUIRE(frame.get_data()[7] == 0x15);
            }
            THEN("The payload can be read")
            {
                Uint16 page;
                frame >> page;
                REQUIRE(page.word == 0x15);
            }
        }
    }
}
