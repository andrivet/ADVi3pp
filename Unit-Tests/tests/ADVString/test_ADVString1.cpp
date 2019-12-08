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
#include "ADVString.h"

using namespace advi3pp;

SCENARIO("ADVString can be set to strings", "[ADVString]")
{
    GIVEN("An empty ADVString<10>")
    {
        ADVString<10> s;
        REQUIRE(s.is_empty());
        REQUIRE(s.length() == 0);
        REQUIRE(s.has_changed());

        WHEN("It is assigned with a short string")
        {
            s = "123";
            THEN("It has the expected length")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 3);
                REQUIRE(s.has_changed());
            }
            THEN("It has the expected content")
            {
                REQUIRE(s[0] == '1'); REQUIRE(s[1] == '2'); REQUIRE(s[2] == '3');
            }
        }

        WHEN("It is assigned with a (too) long string")
        {
            s = "123456789012345";
            THEN("It is truncated and has the expected value")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 10);
                REQUIRE(s.has_changed());
            }
            THEN("It has the expected content")
            {
                REQUIRE(s[0] == '1'); REQUIRE(s[1] == '2'); REQUIRE(s[2] == '3'); REQUIRE(s[9] == '0');
            }
        }

        WHEN("It is assigned with a short ADVString")
        {
            ADVString<8> s2{"12345678"};
            s = s2;
            THEN("It has the expected length")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 8);
                REQUIRE(s.has_changed());
            }
            THEN("It has the expected content")
            {
                REQUIRE(s[0] == '1'); REQUIRE(s[1] == '2'); REQUIRE(s[2] == '3'); REQUIRE(s[7] == '8');
            }
        }

        WHEN("It is assigned with a big ADVString")
        {
            ADVString<15> s2{"123456789012345"};
            s = s2;
            THEN("It is truncated and has the expected value")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 10);
                REQUIRE(s[0] == '1'); REQUIRE(s[1] == '2'); REQUIRE(s[2] == '3'); REQUIRE(s[9] == '0');
                REQUIRE(s.has_changed());
            }
        }
    }
}

SCENARIO("Strings can be append to ADVString", "[ADVString]")
{
    GIVEN("A none-empty string")
    {
        ADVString<10> s{"12345678"};
        REQUIRE_FALSE(s.is_empty());
        REQUIRE(s.length() == 8);
        REQUIRE(s.has_changed());

        WHEN("A short string is appended")
        {
            s += "9";
            THEN("It has the expected value")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 9);
                REQUIRE(s[0] == '1'); REQUIRE(s[1] == '2'); REQUIRE(s[2] == '3'); REQUIRE(s[8] == '9');
                REQUIRE(s.has_changed());
            }
        }

        WHEN("A (too) long string is appended")
        {
            s += "9012345";
            THEN("It is truncated and has the expected value")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 10);
                REQUIRE(s[0] == '1'); REQUIRE(s[1] == '2'); REQUIRE(s[2] == '3'); REQUIRE(s[9] == '0');
                REQUIRE(s.has_changed());
            }
        }
    }
}

SCENARIO("ADVString<10> can be set to numbers", "[ADVString]")
{
    GIVEN("An empty ADVString<10>")
    {
        ADVString<10> s;
        REQUIRE(s.is_empty());
        REQUIRE(s.length() == 0);
        REQUIRE(s.has_changed());

        WHEN("It is set to a small positive int")
        {
            s.set(1234);
            THEN("Is has the expected value")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 4);
                REQUIRE(s[0] == '1'); REQUIRE(s[1] == '2'); REQUIRE(s[2] == '3'); REQUIRE(s[3] == '4');
            }
        }

        WHEN("It is set to a small negative int")
        {
            s.set(-1234);
            THEN("Is has the expected value")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 5);
                REQUIRE(s[0] == '-'); REQUIRE(s[1] == '1'); REQUIRE(s[2] == '2'); REQUIRE(s[3] == '3'); REQUIRE(s[4] == '4');
            }
        }

        WHEN("It is set to a big positive int")
        {
            s.set(65537);
            THEN("Is has the expected value")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 5);
                REQUIRE(s[0] == '6'); REQUIRE(s[1] == '5'); REQUIRE(s[2] == '5'); REQUIRE(s[3] == '3'); REQUIRE(s[4] == '7');
            }
        }

        WHEN("It is set to a big negative int")
        {
            s.set(-65537);
            THEN("Is has the expected value")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 6);
                REQUIRE(s[0] == '-'); REQUIRE(s[1] == '6'); REQUIRE(s[2] == '5');  REQUIRE(s[3] == '5'); REQUIRE(s[4] == '3');  REQUIRE(s[5] == '7');
            }
        }

        WHEN("It is set to a small hexadecimal int")
        {
            s.set(0x1234, Base::Hexadecimal);
            THEN("Is has the expected value")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 4);
                REQUIRE(s[0] == '1'); REQUIRE(s[1] == '2'); REQUIRE(s[2] == '3'); REQUIRE(s[3] == '4');
            }
        }

        WHEN("It is set to a big hexadecimal int")
        {
            s.set(65537, Base::Hexadecimal);
            THEN("Is has the expected value")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 5);
                REQUIRE(s[0] == '1'); REQUIRE(s[1] == '0'); REQUIRE(s[2] == '0'); REQUIRE(s[3] == '0'); REQUIRE(s[4] == '1');
            }
        }

        WHEN("It is set to a decimal")
        {
            s.set(1.234, 3); // 3 decimals
            THEN("Is has the expected value")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 5);
                REQUIRE(s[0] == '1'); REQUIRE(s[1] == '.'); REQUIRE(s[2] == '2'); REQUIRE(s[3] == '3'); REQUIRE(s[4] == '4');
            }
        }
    }
}

SCENARIO("ADVString<4> can be set to numbers", "[ADVString]")
{
    GIVEN("An empty ADVString<4>")
    {
        ADVString<4> s;
        REQUIRE(s.is_empty());
        REQUIRE(s.length() == 0);
        REQUIRE(s.has_changed());

        WHEN("It is set to a small positive int")
        {
            s.set(1234);
            THEN("Is has the expected value")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 4);
                REQUIRE(s[0] == '1'); REQUIRE(s[1] == '2'); REQUIRE(s[2] == '3'); REQUIRE(s[3] == '4');
            }
        }

        WHEN("It is set to a small negative int")
        {
            s.set(-1234);
            THEN("Is it truncated and has the expected value")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 4);
                REQUIRE(s[0] == '-'); REQUIRE(s[1] == '1'); REQUIRE(s[2] == '2'); REQUIRE(s[3] == '3');
            }
        }

        WHEN("It is set to a big positive int")
        {
            s.set(65537);
            THEN("Is has the expected value")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 4);
                REQUIRE(s[0] == '6'); REQUIRE(s[1] == '5'); REQUIRE(s[2] == '5'); REQUIRE(s[3] == '3');
            }
        }

        WHEN("It is set to a big negative int")
        {
            s.set(-65537);
            THEN("Is has the expected value")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 4);
                REQUIRE(s[0] == '-'); REQUIRE(s[1] == '6'); REQUIRE(s[2] == '5');  REQUIRE(s[3] == '5');
            }
        }

        WHEN("It is set to a big decimal")
        {
            s.set(1.23456789, 8); // 8 decimals
            THEN("Is has the expected value")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 4);
                REQUIRE(s[0] == '1'); REQUIRE(s[1] == '.'); REQUIRE(s[2] == '2'); REQUIRE(s[3] == '3');
            }
        }
    }
}

SCENARIO("Numbers can be append to ADVString", "[ADVString]")
{
    GIVEN("A none-empty string")
    {
        ADVString<12> s{"Value = "};
        REQUIRE_FALSE(s.is_empty());
        REQUIRE(s.length() == 8);
        REQUIRE(s.has_changed());

        WHEN("A short number is appended")
        {
            s.append(1234);
            THEN("It has the expected value")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 12);
                REQUIRE(s[0] == 'V'); REQUIRE(s[6] == '='); REQUIRE(s[10] == '3'); REQUIRE(s[11] == '4');
                REQUIRE(s.has_changed());
            }
        }

        WHEN("A long number is appended")
        {
            s.append(65537);
            THEN("It is truncated and has the expected value")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 12);
                REQUIRE(s[0] == 'V'); REQUIRE(s[6] == '='); REQUIRE(s[10] == '5'); REQUIRE(s[11] == '3');
                REQUIRE(s.has_changed());
            }
        }

        WHEN("A long decimal is appended")
        {
            s.append(1.23456789, 8);
            THEN("It is truncated and has the expected value")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 12);
                REQUIRE(s[0] == 'V'); REQUIRE(s[6] == '='); REQUIRE(s[8] == '1'); REQUIRE(s[9] == '.'); REQUIRE(s[10] == '2'); REQUIRE(s[11] == '3');
            }
        }
    }
}

SCENARIO("ADVStrings can concatenated", "[ADVString]")
{
    GIVEN("A empty string")
    {
        ADVString<10> s;
        REQUIRE(s.is_empty());
        REQUIRE(s.length() == 0);
        REQUIRE(s.has_changed());

        WHEN("Strings and numbers are concatenated")
        {
            s << 4 << '.' << 0 << '.' << 0;
            THEN("It has the expected value")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 5);
                REQUIRE(s[0] == '4'); REQUIRE(s[1] == '.'); REQUIRE(s[2] == '0'); REQUIRE(s[3] == '.'); REQUIRE(s[4] == '0');
            }
        }
    }
}

SCENARIO("ADVStrings can be aligned", "[ADVString]")
{
    GIVEN("A small string")
    {
        ADVString<16> s{"1234567890"};
        THEN("It has the expected length")
        {
            REQUIRE_FALSE(s.is_empty());
            REQUIRE(s.length() == 10);
            REQUIRE(s.has_changed());
        }

        WHEN("It is left aligned")
        {
            s.align(Alignment::Left);
            THEN("It has the expected content")
            {
                REQUIRE(s[0] == '1'); REQUIRE(s[1] == '2'); REQUIRE(s[2] == '3'); REQUIRE(s[3] == '4'); REQUIRE(s[4] == '5');
                REQUIRE(s[11] == ' '); REQUIRE(s[12] == ' '); REQUIRE(s[13] == ' '); REQUIRE(s[14] == ' '); REQUIRE(s[15] == ' ');
            }
        }

        WHEN("It is right aligned")
        {
            s.align(Alignment::Right);
            THEN("It has the expected content")
            {
                REQUIRE(s[0] == ' '); REQUIRE(s[1] == ' '); REQUIRE(s[2] == ' '); REQUIRE(s[3] == ' '); REQUIRE(s[4] == ' ');
                REQUIRE(s[11] == '6'); REQUIRE(s[12] == '7'); REQUIRE(s[13] == '8'); REQUIRE(s[14] == '9'); REQUIRE(s[15] == '0');
            }
        }

        WHEN("It is centered")
        {
            s.align(Alignment::Center);
            THEN("It has the expected content")
            {
                REQUIRE(s[0] == ' '); REQUIRE(s[1] == ' '); REQUIRE(s[2] == ' '); REQUIRE(s[3] == '1'); REQUIRE(s[4] == '2');
                REQUIRE(s[11] == '9'); REQUIRE(s[12] == '0'); REQUIRE(s[13] == ' '); REQUIRE(s[14] == ' '); REQUIRE(s[15] == ' ');
            }
        }
    }
}


SCENARIO("ADVStrings can be formatted", "[ADVString]")
{
    GIVEN("An empty string")
    {
        ADVString<16> s;
        WHEN("It is formatted with a simple expression")
        {
            s.format("Value = %i", 6);
            THEN("It has the expected length")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 9);
                REQUIRE(s.has_changed());
            }
            THEN("It has the expected content")
            {
                REQUIRE(s[0] == 'V'); REQUIRE(s[1] == 'a'); REQUIRE(s[2] == 'l'); REQUIRE(s[3] == 'u'); REQUIRE(s[4] == 'e');
                REQUIRE(s[5] == ' '); REQUIRE(s[6] == '='); REQUIRE(s[7] == ' '); REQUIRE(s[8] == '6');
            }
            WHEN("It is centered")
            {
                s.align(Alignment::Center);
                THEN("It has the expected length")
                {
                    REQUIRE_FALSE(s.is_empty());
                    REQUIRE(s.length() == 16);
                    REQUIRE(s.has_changed());
                }
                THEN("It has the expected content")
                {
                    REQUIRE(s[0] == ' '); REQUIRE(s[1] == ' '); REQUIRE(s[2] == ' ');
                    REQUIRE(s[3] == 'V'); REQUIRE(s[4] == 'a'); REQUIRE(s[5] == 'l'); REQUIRE(s[6] == 'u'); REQUIRE(s[7] == 'e');
                    REQUIRE(s[8] == ' '); REQUIRE(s[9] == '='); REQUIRE(s[10] == ' '); REQUIRE(s[11] == '6');
                    REQUIRE(s[12] == ' '); REQUIRE(s[13] == ' '); REQUIRE(s[14] == ' '); REQUIRE(s[15] == ' ');
                }
            }
        }
    }
}

SCENARIO("Operations on ADVString can be chained")
{
    GIVEN("An empty string")
    {
        ADVString<16> s;

        WHEN("Several operations are chained")
        {
            s.append(1234).append("ABCD").align(Alignment::Center);
            THEN("It has the expected length")
            {
                REQUIRE_FALSE(s.is_empty());
                REQUIRE(s.length() == 16);
                REQUIRE(s.has_changed());
            }
            THEN("It has the expected content")
            {
                REQUIRE(s[0] == ' '); REQUIRE(s[1] == ' '); REQUIRE(s[2] == ' '); REQUIRE(s[3] == ' ');
                REQUIRE(s[4] == '1'); REQUIRE(s[5] == '2'); REQUIRE(s[6] == '3'); REQUIRE(s[7] == '4');
                REQUIRE(s[8] == 'A'); REQUIRE(s[9] == 'B'); REQUIRE(s[10] == 'C'); REQUIRE(s[11] == 'D');
                REQUIRE(s[12] == ' '); REQUIRE(s[13] == ' '); REQUIRE(s[14] == ' '); REQUIRE(s[15] == ' ');
            }
        }
    }
}
