/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARIO *ICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

   New Outfile RedTransport class

*/

#ifndef _REDEMPTION_LIBS_RIO_OUTFILE_H_
#define _REDEMPTION_LIBS_RIO_OUTFILE_H_

#include "rio_constants.h"

struct RIOOutfile {
    int fd;
};

extern "C" {
    /* This method does not allocate space for object itself, 
        but initialize it's properties
        and allocate and initialize it's subfields if necessary
    */
    inline RIO_ERROR rio_m_RIOOutfile_constructor(RIOOutfile * self, int fd)
    {
        self->fd = fd;
        return RIO_ERROR_OK;
    }

    /* This method deallocate any space used for subfields if any
    */
    inline RIO_ERROR rio_m_RIOOutfile_destructor(RIOOutfile * self)
    {
        return RIO_ERROR_OK;
    }

    /* This method close ressource without calling destructor
       Any subsequent call should return an error
    */
    inline void rio_m_RIOOutfile_close(RIOOutfile * self)
    {
        close(self->fd);
    }

    /* This method receive len bytes of data into buffer
       target buffer *MUST* be large enough to contains len data
       returns len actually received (may be 0),
       or negative value to signal some error.
       If an error occurs after reading some data, the return buffer
       has been changed but an error is returned anyway
       and an error returned on subsequent call.
    */
    inline ssize_t rio_m_RIOOutfile_recv(RIOOutfile * self, void * data, size_t len)
    {
         return -RIO_ERROR_SEND_ONLY;
    }

    /* This method send len bytes of data from buffer to current transport
       buffer must actually contains the amount of data requested to send.
       returns len actually sent (may be 0),
       or negative value to signal some error.
       If an error occurs after sending some data the amount sent will be returned
       and an error returned on subsequent call.
    */
    inline ssize_t rio_m_RIOOutfile_send(RIOOutfile * self, const void * data, size_t len)
    {
        ssize_t ret = 0;
        size_t remaining_len = len;
        size_t total_sent = 0;
        while (remaining_len) {
            ret = ::write(self->fd, (uint8_t*)data + total_sent, remaining_len);
            if (ret <= 0){
                if (errno == EINTR){ continue; }
                switch (errno){
                    case EAGAIN:
                        return -RIO_ERROR_EAGAIN;
                    case EBADF:
                        return -RIO_ERROR_EBADF;
                    case EDESTADDRREQ:
                        return -RIO_ERROR_EDESTADDRREQ;
                    case EFAULT:
                        return -RIO_ERROR_EFAULT;
                    case EFBIG:
                        return -RIO_ERROR_EFBIG;
                    case EINVAL:
                        return -RIO_ERROR_EINVAL;
                    case EIO:
                        return -RIO_ERROR_EIO;
                    case ENOSPC:
                        return -RIO_ERROR_ENOSPC;
                    case EPIPE:
                        return -RIO_ERROR_EPIPE;
                    case EISDIR:
                        return -RIO_ERROR_EISDIR;
                    default:
                        return -RIO_ERROR_POSIX;
                }
            }
            remaining_len -= ret;
            total_sent += ret;
        }
        return total_sent;
    }

};

#endif
