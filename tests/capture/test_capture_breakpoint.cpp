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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestWrmCapture
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "capture.hpp"

BOOST_AUTO_TEST_CASE(TestSplittedCapture)
{
    {
        // Timestamps are applied only when flushing
        struct timeval now;
        now.tv_usec = 0;
        now.tv_sec = 1000;

        Rect scr(0, 0, 800, 600);
        
        Inifile ini;
        ini.globals.frame_interval = 100; // one timestamp every second
        ini.globals.break_interval = 3;   // one WRM file every 5 seconds

        ini.globals.png_limit = 10; // one snapshot by second
        ini.globals.png_interval = 10; // one snapshot by second

        ini.globals.capture_wrm = true;
        Capture capture(now, scr.cx, scr.cy, "./", "capture", ini);

        capture.draw(RDPOpaqueRect(scr, GREEN), scr);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, false, false);

        capture.draw(RDPOpaqueRect(Rect(1, 50, 700, 30), BLUE), scr);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, false, false);

        capture.draw(RDPOpaqueRect(Rect(2, 100, 700, 30), WHITE), scr);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, false, false);

        // ------------------------------ BREAKPOINT ------------------------------

        capture.draw(RDPOpaqueRect(Rect(3, 150, 700, 30), RED), scr);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, false, false);

        capture.draw(RDPOpaqueRect(Rect(4, 200, 700, 30), BLACK), scr);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, false, false);

        capture.draw(RDPOpaqueRect(Rect(5, 250, 700, 30), PINK), scr);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, false, false);

        // ------------------------------ BREAKPOINT ------------------------------

        capture.draw(RDPOpaqueRect(Rect(6, 300, 700, 30), WABGREEN), scr);
        now.tv_sec++;
        capture.snapshot(now, 0, 0, false, false);

        capture.flush(); // to close last wrm
        
        BOOST_CHECK_EQUAL((unsigned)3051, (unsigned)capture.png_sequence->filesize(0));
        capture.png_sequence->unlink(0);
        BOOST_CHECK_EQUAL((unsigned)3082, (unsigned)capture.png_sequence->filesize(1));
        capture.png_sequence->unlink(1);
        BOOST_CHECK_EQUAL((unsigned)3102, (unsigned)capture.png_sequence->filesize(2));
        capture.png_sequence->unlink(2);
        BOOST_CHECK_EQUAL((unsigned)3109, (unsigned)capture.png_sequence->filesize(3));
        capture.png_sequence->unlink(3);
        BOOST_CHECK_EQUAL((unsigned)3136, (unsigned)capture.png_sequence->filesize(4));
        capture.png_sequence->unlink(4);
        BOOST_CHECK_EQUAL((unsigned)3145, (unsigned)capture.png_sequence->filesize(5));
        capture.png_sequence->unlink(5);
        BOOST_CHECK_EQUAL((unsigned)3176, (unsigned)capture.png_sequence->filesize(6));
        capture.png_sequence->unlink(6);

        BOOST_CHECK_EQUAL((unsigned)1622, (unsigned)capture.wrm_sequence->filesize(0));
        capture.wrm_sequence->unlink(0);
        BOOST_CHECK_EQUAL((unsigned)3392, (unsigned)capture.wrm_sequence->filesize(1));
        capture.wrm_sequence->unlink(1);
        BOOST_CHECK_EQUAL((unsigned)3371, (unsigned)capture.wrm_sequence->filesize(2));
        capture.wrm_sequence->unlink(2);
        // The destruction of capture object will finalize the metafile content
    }
    // from here we do not have access to the file sequence used for the metafile inside capture
    // so we recreate a sequence with the same name. Not really nice, but it will do
    FileSequence meta_seq("path file pid extension", "./", "capture", "mwrm");
    BOOST_CHECK_EQUAL((unsigned)125, (unsigned)meta_seq.filesize(0));
    meta_seq.unlink(0);
}


