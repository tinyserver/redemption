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

   Unit test to RDP VirtualChannel object
   Using lib boost functions for testing
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"
#include "RDP/capabilities.hpp"

BOOST_AUTO_TEST_CASE(TestCapabilityVirtualChannelEmit)
{
    VirtualChannelCaps virtualchannel_caps;
    virtualchannel_caps.flags = VCCAPS_COMPR_CS_8K;
    virtualchannel_caps.VCChunkSize = 56897;

    BOOST_CHECK_EQUAL(virtualchannel_caps.capabilityType, (uint16_t)CAPSTYPE_VIRTUALCHANNEL);
    BOOST_CHECK_EQUAL(virtualchannel_caps.len, (uint16_t)CAPLEN_VIRTUALCHANNEL);
    BOOST_CHECK_EQUAL(virtualchannel_caps.flags, (uint32_t) 2);
    BOOST_CHECK_EQUAL(virtualchannel_caps.VCChunkSize, (uint32_t) 56897);

    BStream stream(1024);
    virtualchannel_caps.emit(stream);
    stream.mark_end();
    stream.p = stream.data;

    VirtualChannelCaps virtualchannel_caps2;

    BOOST_CHECK_EQUAL(virtualchannel_caps2.capabilityType, (uint16_t)CAPSTYPE_VIRTUALCHANNEL);
    BOOST_CHECK_EQUAL(virtualchannel_caps2.len, (uint16_t)CAPLEN_VIRTUALCHANNEL);

    BOOST_CHECK_EQUAL((uint16_t)CAPSTYPE_VIRTUALCHANNEL, stream.in_uint16_le());
    BOOST_CHECK_EQUAL((uint16_t)CAPLEN_VIRTUALCHANNEL, stream.in_uint16_le());
    virtualchannel_caps2.recv(stream, CAPLEN_VIRTUALCHANNEL);

    BOOST_CHECK_EQUAL(virtualchannel_caps2.flags, (uint32_t) 2);
    BOOST_CHECK_EQUAL(virtualchannel_caps2.VCChunkSize, (uint32_t) 56897);
}
