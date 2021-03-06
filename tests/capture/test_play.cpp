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
#include "staticcapture.hpp"
#include "nativecapture.hpp"
#include "FileToGraphic.hpp"

BOOST_AUTO_TEST_CASE(TestSample0WRM)
{
    const char * input_filename = "./tests/fixtures/sample0.wrm";
    InByFilenameTransport in_wrm_trans(input_filename);
    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(&in_wrm_trans, begin_capture, end_capture, false, 0);

    Inifile ini;
    ini.globals.debug.primary_orders = 0;
    ini.globals.debug.secondary_orders = 0;

    FileSequence png_sequence("path file pid count extension", "./", "first", "png");
    OutByFilenameSequenceTransport out_png_trans(png_sequence);
    ImageCapture png_recorder(out_png_trans, player.screen_rect.cx, player.screen_rect.cy);
        
    png_recorder.update_config(ini);
    player.add_consumer(&png_recorder);

    FileSequence wrm_sequence("path file pid count extension", "./", "first", "wrm");
    OutByFilenameSequenceTransport out_wrm_trans(wrm_sequence);
    ini.globals.frame_interval = 10;
    ini.globals.break_interval = 20;
    
    BmpCache bmp_cache(
        player.bmp_cache->bpp,
        player.bmp_cache->small_entries,
        player.bmp_cache->small_size,
        player.bmp_cache->medium_entries,
        player.bmp_cache->medium_size,
        player.bmp_cache->big_entries,
        player.bmp_cache->big_size);

    RDPDrawable drawable(player.screen_rect.cx, player.screen_rect.cy, true);
    NativeCapture wrm_recorder(
        player.record_now,
        out_wrm_trans,
        player.screen_rect.cx,
        player.screen_rect.cy,
        bmp_cache, &drawable, ini);

    wrm_recorder.update_config(ini);
    player.add_consumer(&wrm_recorder);

    BOOST_CHECK_EQUAL((unsigned)1352304810, (unsigned)player.record_now.tv_sec);
    player.play();
    
    png_recorder.flush();
    BOOST_CHECK_EQUAL((unsigned)1352304870, (unsigned)player.record_now.tv_sec);

    wrm_recorder.flush();
    BOOST_CHECK_EQUAL((unsigned)21280, (unsigned)png_sequence.filesize(0));
    png_sequence.unlink(0);

    BOOST_CHECK_EQUAL((unsigned)500675, (unsigned)wrm_sequence.filesize(0));
    wrm_sequence.unlink(0);
    BOOST_CHECK_EQUAL((unsigned)1265693, (unsigned)wrm_sequence.filesize(1));
    wrm_sequence.unlink(1);
    BOOST_CHECK_EQUAL((unsigned)360488, (unsigned)wrm_sequence.filesize(2));
    wrm_sequence.unlink(2);
}

BOOST_AUTO_TEST_CASE(TestSecondPart)
{
    const char * input_filename = "./tests/fixtures/sample1.wrm";
    InByFilenameTransport in_wrm_trans(input_filename);
    timeval begin_capture;
    begin_capture.tv_sec = 0; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = 0; end_capture.tv_usec = 0;
    FileToGraphic player(&in_wrm_trans, begin_capture, end_capture, false, 0);

    Inifile ini;
    ini.globals.debug.primary_orders = 0;
    ini.globals.debug.secondary_orders = 0;

    FileSequence png_sequence("path file pid count extension", "./", "second_part", "png");
    OutByFilenameSequenceTransport out_png_trans(png_sequence);
    ImageCapture png_recorder(out_png_trans, player.screen_rect.cx, player.screen_rect.cy);
        
    png_recorder.update_config(ini);
    player.add_consumer(&png_recorder);

    FileSequence wrm_sequence("path file pid count extension", "./", "second_part", "wrm");
    OutByFilenameSequenceTransport out_wrm_trans(wrm_sequence);
    ini.globals.frame_interval = 10;
    ini.globals.break_interval = 20;
    
    BmpCache bmp_cache(
        player.bmp_cache->bpp,
        player.bmp_cache->small_entries,
        player.bmp_cache->small_size,
        player.bmp_cache->medium_entries,
        player.bmp_cache->medium_size,
        player.bmp_cache->big_entries,
        player.bmp_cache->big_size);

    RDPDrawable drawable(player.screen_rect.cx, player.screen_rect.cy, true);
    NativeCapture wrm_recorder(
        player.record_now,
        out_wrm_trans,
        player.screen_rect.cx,
        player.screen_rect.cy,
        bmp_cache, &drawable, ini);

    wrm_recorder.update_config(ini);
    player.add_consumer(&wrm_recorder);

    BOOST_CHECK_EQUAL((unsigned)1352304870, (unsigned)player.record_now.tv_sec);
    player.play();
    BOOST_CHECK_EQUAL((unsigned)1352304928, (unsigned)player.record_now.tv_sec);

    png_recorder.flush();
    BOOST_CHECK_EQUAL((unsigned)47483, (unsigned)png_sequence.filesize(0));
    png_sequence.unlink(0);

    wrm_recorder.flush();
    BOOST_CHECK_EQUAL((unsigned)74803, (unsigned)wrm_sequence.filesize(0));
    wrm_sequence.unlink(0);
    BOOST_CHECK_EQUAL((unsigned)273774, (unsigned)wrm_sequence.filesize(1));
    wrm_sequence.unlink(1);
    BOOST_CHECK_EQUAL((unsigned)185108, (unsigned)wrm_sequence.filesize(2));
    wrm_sequence.unlink(2);
}

//TODO("need to add test for controlled replay between begin_time and end_time")
