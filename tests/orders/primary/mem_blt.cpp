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
   Author(s): Christophe Grosjean

   Unit test to RDP Orders coder/decoder
   Using lib boost functions for testing
*/


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestOrderMemBlt
#include <boost/test/auto_unit_test.hpp>
#include <algorithm>

#include "RDP/orders/RDPOrdersCommon.hpp"
#include "RDP/orders/RDPOrdersPrimaryMemBlt.hpp"

#include "./test_orders.hpp"

BOOST_AUTO_TEST_CASE(TestMemBlt)
{
    using namespace RDP;

    {
        BStream stream(1000);

        uint16_t cache_id = 1;
        uint16_t cache_idx = 15;

        RDPOrderCommon state_common(0, Rect(311, 0, 800, 600));
        RDPMemBlt state_memblt(cache_id, Rect(310, 390, 10, 10), 0xFF, 0, 0, cache_idx);

        RDPOrderCommon newcommon(MEMBLT, Rect(311, 0, 800, 600));
        RDPMemBlt(cache_id, Rect(300, 400, 50, 60), 0xFF, 3, 4, cache_idx).emit(stream, newcommon, state_common, state_memblt);

        uint8_t datas[9] = {
            SMALL | CHANGE | STANDARD | BOUNDS | DELTA | LASTBOUNDS,
            MEMBLT,
            0xde,   // x, y, cx, cy, srcx, srcy coordinates changed
            -10,    // x = -10 -> 300
            +10,    // y = 390 +10 = 400
            +40,    // cx = 10 + 40 -> 50
            +50,    // cy = 10 + 50 -> 60
              3,    // srcx = 0 + 3 = 3
              4,    // srcy = 0 + 4 = 4
        };
        check_datas(stream.p-stream.data, stream.data, 9, datas, "MemBlt 1");

        stream.mark_end(); stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)MEMBLT, common_cmd.order);


        RDPMemBlt cmd(cache_id, Rect(310, 390, 10, 10), 0xFF, 0, 0, cache_idx);


        cmd.receive(stream, header);

        check<RDPMemBlt>(common_cmd, cmd,
            RDPOrderCommon(MEMBLT, Rect(311, 0, 800, 600)),
            RDPMemBlt(cache_id, Rect(300, 400, 50, 60), 0xFF, 3, 4, cache_idx),
            "MemBlt 1");
    }

    {
        BStream stream(1000);

        uint16_t cache_id = 1;
        uint16_t cache_idx = 15;

        RDPOrderCommon state_common(0, Rect(311, 0, 800, 600));
        RDPMemBlt state_memblt(0, Rect(), 0, 0, 0, 0);

        RDPOrderCommon newcommon(MEMBLT, Rect(311, 0, 800, 600));
        RDPMemBlt(cache_id, Rect(300, 400, 50, 60), 0xFF, 3, 4, cache_idx).emit(stream, newcommon, state_common, state_memblt);

        uint8_t datas[21] = {
            CHANGE | STANDARD | BOUNDS | LASTBOUNDS,
            MEMBLT,
            0xff, 0x01,  // cache_id, x, y, cx, cy, rop, srcx, srcy, cache_idx changed
            0x01, 0x00,  // cache_id
            0x2C, 0x01,  // x = 300
            0x90, 0x01,  // y = 400
            0x32, 0x00,  // cx = 50
            0x3c, 0x00,  // cy = 60
            0xFF,        // rop
            0x03, 0x00,  // srcx = 3
            0x04, 0x00,  // srcy = 4
            0x0F, 0      // cache_idx
        };
        check_datas(stream.p-stream.data, stream.data, 21, datas, "MemBlt 2");

        stream.mark_end(); stream.p = stream.data;

        RDPOrderCommon common_cmd = state_common;
        uint8_t control = stream.in_uint8();
        BOOST_CHECK_EQUAL(true, !!(control & STANDARD));
        RDPPrimaryOrderHeader header = common_cmd.receive(stream, control);

        BOOST_CHECK_EQUAL((uint8_t)MEMBLT, common_cmd.order);


        RDPMemBlt cmd(0, Rect(), 0, 0, 0, 0);


        cmd.receive(stream, header);

        check<RDPMemBlt>(common_cmd, cmd,
            RDPOrderCommon(MEMBLT, Rect(311, 0, 800, 600)),
            RDPMemBlt(cache_id, Rect(300, 400, 50, 60), 0xFF, 3, 4, cache_idx),
            "MemBlt 2");
    }

}
