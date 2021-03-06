/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#ifndef _REDEMPTION_UTILS_ZLIB_HPP_
#define _REDEMPTION_UTILS_ZLIB_HPP_

#include <zlib.h>

class ZRaiiDeflateEnd
{
    z_stream& zstrm;

public:
    ZRaiiDeflateEnd(z_stream& zstream)
    : zstrm(zstream)
    {}

    ~ZRaiiDeflateEnd()
    {
        deflateEnd(&this->zstrm);
    }
};

class ZRaiiInflateEnd
{
    z_stream& zstrm;

public:
    ZRaiiInflateEnd(z_stream& zstream)
    : zstrm(zstream)
    {}

    ~ZRaiiInflateEnd()
    {
        inflateEnd(&this->zstrm);
    }
};

#endif
