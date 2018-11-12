//
// Created by Sébastien Andrivet on 2018/09/16.
//

#ifndef UNIT_TESTS_AVRLIB_H
#define UNIT_TESTS_AVRLIB_H

extern "C"
{
char* itoa(int value, char* str, int base);
char* utoa(unsigned int value, char* str, int base);
char* ltoa(long value, char* str, int base);
char* ultoa(unsigned long value, char* str, int base);
char* dtostrf(double val, signed char width, unsigned char prec, char* s);
};

#endif //UNIT_TESTS_AVRLIB_H
