/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Fixed point math class.
 *
 * @copyright Chronoshift is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "fixed.h"
#include "minmax.h"
#include "stringex.h"
#include <ctype.h>
#include <stdio.h>

const fixed fixed::_0_1(0, 1);
const fixed fixed::_1_1(1, 1);
const fixed fixed::_1_2(1, 2);
const fixed fixed::_1_3(1, 3);
const fixed fixed::_1_4(1, 4);
const fixed fixed::_3_4(3, 4);
const fixed fixed::_2_3(2, 3);

fixed::fixed(int a, int b)
{
    if (b) {
        m_number.word = (a << 8) / b;
    } else {
        m_number.word = 0;
    }
}

fixed::fixed(const char *string)
{
    const char *str_read = string;

    if (string) {
        // this while loop finds first number while for loop finds
        // one after last?
        while (isspace(*str_read) != 0) {
            ++str_read;
        }

        // Find something that isn't a number?
        const char *i;

        for (i = str_read; isdigit(*i) != 0; ++i) {
            ;
        }

        if (*i == '%') {
            m_number.word = (atoi(str_read) << 8) / 100;
        } else {
            m_number.word = 0;
            if (string && *string != '.') {
                m_number.ch.hi = atoi(string);
            }

            // finds '.' if it exists, nulls char *if not to indicate no
            // decimal
            while (*str_read != '.') {
                if (*str_read) {
                    ++str_read;
                    char cur_char = *str_read;
                    if (*str_read == '.') {
                        break;
                    }
                    ++str_read;

                    if (cur_char) {
                        continue;
                    }
                }
                str_read = nullptr;
                break;
            }

            str_read = str_read ? str_read + 1 : str_read;

            if (str_read) {
                int divisor = 1;
                int fraction = atoi(str_read);

                while (isdigit(*str_read) != 0) {
                    ++str_read;
                    divisor *= 10;
                }

                m_number.ch.low = (fraction << 8) / divisor;
            }
        }
    } else {
        m_number.word = 0;
    }
}

int fixed::To_ASCII(char *string, int size) const
{
    char char_buff[32];

    if (!string) {
        return 0;
    }

    int whole = m_number.ch.hi;
    int fraction = (100 * m_number.ch.low) >> 8;

    if (fraction > 0) {
        snprintf(char_buff, sizeof(char_buff), "%d.%02d", whole, fraction);

        char *last_char = &char_buff[(int)strlen(char_buff) - 1];

        while (*last_char == '0') {
            *last_char = '\0';
            --last_char;
        }
    } else {
        // sprintf( char_buff, "%d", whole );
        sprintf(char_buff, "%d.0", whole);
    }

    if (size == -1) {
        size = (int)strlen(char_buff) + 1;
    }

    strlcpy((char *)string, char_buff, size);

    return Min((int)strlen(char_buff), (size - 1));
}

char *const fixed::As_ASCII() const
{
    static char _buffer[32];

    To_ASCII(_buffer, sizeof(_buffer));

    return _buffer;
}
