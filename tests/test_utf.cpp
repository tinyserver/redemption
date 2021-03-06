/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test for Lightweight UTF library

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestLul
#include <boost/test/auto_unit_test.hpp>

#include "log.hpp"
#define LOGNULL

#include "utf.hpp"
//#include <unicode/ustring.h>
#include <stdio.h>


BOOST_AUTO_TEST_CASE(TestUTF8Len_2)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    BOOST_CHECK_EQUAL(11, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8Len)
{
    uint8_t source[] = { 'a', 0xC3, 0xA9, 0};

    BOOST_CHECK_EQUAL(2, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 20);
    BOOST_CHECK_EQUAL(11, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos_0)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 0);
    BOOST_CHECK_EQUAL(0, source[0]);
    BOOST_CHECK_EQUAL(0, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos_0_v2)
{
    uint8_t source[] = { 0xC3, 0xA9, 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 0);
    BOOST_CHECK_EQUAL(0, source[0]);
    BOOST_CHECK_EQUAL(0, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos_1)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 1);
    
    BOOST_CHECK_EQUAL('a', source[0]);
    BOOST_CHECK_EQUAL(0, source[1]);
    BOOST_CHECK_EQUAL(1, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos_1_v2)
{
    uint8_t source[] = { 0xC3, 0xA9, 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 1);
    
    BOOST_CHECK_EQUAL(0xC3, source[0]);
    BOOST_CHECK_EQUAL(0xA9, source[1]);
    BOOST_CHECK_EQUAL(0, source[2]);
    BOOST_CHECK_EQUAL(1, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos_2)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 2);
    
    BOOST_CHECK_EQUAL('a', source[0]);
    BOOST_CHECK_EQUAL('b', source[1]);
    BOOST_CHECK_EQUAL(0, source[2]);
    BOOST_CHECK_EQUAL(2, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos_2_v2)
{
    uint8_t source[] = { 0xC3, 0xA9, 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 2);
    
    BOOST_CHECK_EQUAL(0xC3, source[0]);
    BOOST_CHECK_EQUAL(0xA9, source[1]);
    BOOST_CHECK_EQUAL('a', source[2]);
    BOOST_CHECK_EQUAL(0, source[3]);
    BOOST_CHECK_EQUAL(2, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos_8)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 8);
    
    uint8_t expected_result[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0};
    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(8, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos_8_v2)
{
    uint8_t source[] = { 0xC3, 0xA9, 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 8);
    
    uint8_t expected_result[] = {0xC3, 0xA9, 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0};
    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(8, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos_9)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 9);
    
    uint8_t expected_result[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0};
    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(9, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8TruncateAtPos_9_v2)
{
    uint8_t source[] = { 0xC3, 0xA9, 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8TruncateAtPos(source, 9);
    
    uint8_t expected_result[] = {0xC3, 0xA9, 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0};
    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(9, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8InsertAtPos_0)
{
    uint8_t source[255] = { 0 };
    uint8_t to_insert[] = { 0xC3, 0xA9, 0};

    UTF8InsertAtPos(source, 3, to_insert, sizeof(source));

    BOOST_CHECK_EQUAL(0xC3, source[0]);
    BOOST_CHECK_EQUAL(0xA9, source[1]);
    BOOST_CHECK_EQUAL(0, source[2]);

    BOOST_CHECK_EQUAL(1, UTF8Len(source));
}


BOOST_AUTO_TEST_CASE(TestUTF8InsertAtPos_beyond_end)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
    uint8_t to_insert[] = { 0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z', 0};

    UTF8InsertAtPos(source, 20, to_insert, sizeof(source));
    
    uint8_t expected_result[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 
                                  0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z', 0
    };


    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));

    BOOST_CHECK_EQUAL(17, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8InsertAtPos_at_start)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
    uint8_t to_insert[] = { 0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z', 0};

    UTF8InsertAtPos(source, 0, to_insert, sizeof(source));
    
    uint8_t expected_result[] = { 0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z',
                                 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0
    };


    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(17, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8InsertAtPos_at_1)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
    uint8_t to_insert[] = { 0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z', 0};

    UTF8InsertAtPos(source, 1, to_insert, sizeof(source));
    
    uint8_t expected_result[] = {'a', 0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z',
                                 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0
    };


    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(17, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8InsertAtPos_at_8)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
    uint8_t to_insert[] = { 0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z', 0};

    UTF8InsertAtPos(source, 8, to_insert, sizeof(source));
    
    uint8_t expected_result[] = {'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9,
                                 0xC3, 0xA9, 'x', 0xC3, 0xA7, 0xC3, 0xA0, 'y', 'z',
                                 0xC3, 0xA7, 0xC3, 0xA0, '@', 0
    };


    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(17, UTF8Len(source));
}


BOOST_AUTO_TEST_CASE(TestUTF8InsertOneAtPos_at_8)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8InsertOneAtPos(source, 8, 0xE9, sizeof(source));
    
    uint8_t expected_result[] = {'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0,
                                 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0
    };


    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(12, UTF8Len(source));
}


BOOST_AUTO_TEST_CASE(TestUTF8RemoveOneAtPos0)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8RemoveOneAtPos(source, 0);
    
    uint8_t expected_result[] = {'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0 };

    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(10, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8RemoveOneAtPos1)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8RemoveOneAtPos(source, 1);
    
    uint8_t expected_result[] = {'a', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0 };

    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(10, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8RemoveOneAtPos7)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8RemoveOneAtPos(source, 7);
    
    uint8_t expected_result[] = {'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA7, 0xC3, 0xA0, '@', 0 };

    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(10, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8RemoveOneAtPos8)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8RemoveOneAtPos(source, 8);
    
    uint8_t expected_result[] = {'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA0, '@', 0 };

    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(10, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8RemoveOneAtPos9)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8RemoveOneAtPos(source, 9);
    
    uint8_t expected_result[] = {'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, '@', 0 };

    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(10, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8RemoveOneAtPos10)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8RemoveOneAtPos(source, 10);
    
    uint8_t expected_result[] = {'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, 0 };

    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(10, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8RemoveOneAtPos11)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8RemoveOneAtPos(source, 11);
    
    uint8_t expected_result[] = {'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0 };

    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(11, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8RemoveOneAtPos12)
{
    uint8_t source[255] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};

    UTF8RemoveOneAtPos(source, 12);
    
    uint8_t expected_result[] = {'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA0, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0 };

    BOOST_CHECK_EQUAL(0, memcmp(source, expected_result, sizeof(expected_result)));
    BOOST_CHECK_EQUAL(11, UTF8Len(source));
}

BOOST_AUTO_TEST_CASE(TestUTF8_UTF16)
{
    uint8_t source[] = { 'a', 'b', 'c', 'e', 'd', 'e', 'f', 0xC3, 0xA9, 0xC3, 0xA7, 0xC3, 0xA0, '@', 0};
    uint8_t expected_target[] = { 'a', 0, 'b', 0, 'c', 0, 'e', 0, 'd', 0,
                                  'e', 0, 'f', 0, 
                                  0xE9, 0 /* é */,
                                  0xE7, 0 /* ç */,
                                  0xE0, 0 /* à */,
                                  '@', 0, };
    const size_t target_length = sizeof(expected_target)/sizeof(expected_target[0]);
    uint8_t target[target_length];

    size_t nbbytes_utf16 = UTF8toUTF16(source, target, target_length);

    // Check result
    BOOST_CHECK_EQUAL(target_length, nbbytes_utf16);
    for (size_t q = 0 ; q < target_length ; q++){
        if (expected_target[q] != target[q]){
            printf("at %u: expected %u, got %u\n", (unsigned)q, expected_target[q] ,target[q]);
            BOOST_CHECK(false);
        }
    }

    uint8_t source_round_trip[15];

    size_t nbbytes_utf8 = UTF16toUTF8(target, nbbytes_utf16 / 2, source_round_trip, sizeof(source_round_trip));
    BOOST_CHECK_EQUAL(14, nbbytes_utf8);

}


BOOST_AUTO_TEST_CASE(TestUTF8toUnicode)
{
    uint8_t source[16] = "Red";
    uint32_t uni[16];

    // Check result
    BOOST_CHECK_EQUAL(3, UTF8toUnicode(source, uni, sizeof(uni)/sizeof(uni[0])));

}


BOOST_AUTO_TEST_CASE(TestUTF8Check_invalid_utf8)
{
    uint8_t source[] = {0xC3, 0xA9 /* é */, 0xC3 };
    size_t source_length = sizeof(source); // source_length is a buffer size, including trailing zero if any
    // returns number of valid UTF8 characters (source buffer unchanged, no trailing zero added after broken part)

    // Check result
    BOOST_CHECK_EQUAL(2, UTF8Check(source, source_length));

}

BOOST_AUTO_TEST_CASE(TestUTF8Check_valid_utf8_no_trailing_zero)
{
    uint8_t source[] = {0xC3, 0xA9 /* é */, 0xC3, 0xA9 };
    size_t source_length = sizeof(source);

    // Check result
    BOOST_CHECK_EQUAL(4, UTF8Check(source, source_length));

}

BOOST_AUTO_TEST_CASE(TestUTF8Check_valid_utf8_trailing_zero)
{
    uint8_t source[] = {0xC3, 0xA9 /* é */, 0xC3, 0xA9, 0, 'a' };
    size_t source_length = sizeof(source);

    // Check result
    BOOST_CHECK_EQUAL(5, UTF8Check(source, source_length));

}

