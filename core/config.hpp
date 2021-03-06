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

   configuration file,
   parsing config file rdpproxy.ini

*/


#ifndef _REDEMPTION_CORE_CONFIG_HPP_
#define _REDEMPTION_CORE_CONFIG_HPP_

#include <dirent.h>
#include <stdio.h>

#include "log.hpp"

#include <istream>
#include <string>
#include <stdint.h>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

using namespace std;

static inline bool bool_from_string(string str)
{
    return (boost::iequals(string("1"),str))
        || (boost::iequals(string("yes"),str))
        || (boost::iequals(string("on"),str))
        || (boost::iequals(string("true"),str));
}

static inline bool bool_from_cstr(const char * str)
{
    return (0 == strcasecmp("1",str))
        || (0 == strcasecmp("yes",str))
        || (0 == strcasecmp("on",str))
        || (0 == strcasecmp("true",str));
}

static inline unsigned level_from_string(const char * str)
{ // low = 0, medium = 1, high = 2
    unsigned res = 0;
    if (0 == strcmp("medium", str)) { res = 1; }
    else if (0 == strcmp("high", str)) { res = 2; }
    return res;
}

static inline bool check_name(const char * str)
{
    return ((strlen(str) > 0) && (strlen(str) < 250));
}

static inline bool check_ask(const char * str)
{
    return (0 == strcmp(str, "ask"));
}

static inline void ask_string(const char * str, char buffer[], bool & flag)
{
    flag = check_ask(str);
    if (!flag){
        strncpy(buffer, str, strlen(str));
        buffer[strlen(str)] = 0;
    }
    else {
        buffer[0] = 0;
    }
}


struct IniAccounts {
    char accountname[255];
    char username[255]; // should use string
    char password[255]; // should use string
};

struct Inifile {
    struct Inifile_globals {
        bool capture_png;
        bool capture_wrm;
        bool capture_flv;
        bool capture_ocr;
        bool capture_chunk;
        bool capture_drawable;
        char movie_path[512];
        char codec_id[512];
        char video_quality[512];
        char auth_user[512];
        char host[512];
        char target_device[512];
        char target_user[512];
        char auth_channel[512];
        
        bool bitmap_cache;       // default true
        bool bitmap_compression; // default true
        int port;                // default 3389
        int encryptionLevel;   // 0=low, 1=medium, 2=high
        bool autologin;      // true if we should bypass login box and go directly
                             // to server with credentials provided by rdp client
                             // obviously, to do so we need some target address
                             // the used solution is to provide a user name
                             // in the form user@host
                             // if autologin mode is set and that we provide
                             // an @host target value autologin we be used
                             // if we do not provide @host, we are directed to
                             // login box as usual.
        bool autologin_useauth; // the above user command line is incomplete
                                // the full form is user@host:authuser
                                // if autologin and autologin_useauth are on
                                // the authuser will be used with provided
                                // password for authentication by auth module
                                // that will return the real credential.
                                // Otherwise the proxy will try a direct
                                // connection to host with user account
                                // and provided password.
        char authip[255];
        int authport;
        unsigned authversion;
        bool nomouse;
        bool notimestamp;
        bool autovalidate;      // dialog autovalidation for test
        char dynamic_conf_path[1024]; // directory where to look for dynamic configuration files

        unsigned capture_flags; // 1 PNG capture, 2 WRM
        unsigned png_interval;  // time between 2 png captures (in 1/10 seconds)
        unsigned frame_interval;  // time between 2 frame captures (in 1/100 seconds)
        unsigned break_interval;  // time between 2 wrm movies (in seconds)
        uint64_t flv_break_interval; // time between 2 flv movies captures (in seconds)
        unsigned flv_frame_interval; 
        unsigned ocr_interval;

        unsigned png_limit;     // number of png captures to keep

        int l_bitrate;         // bitrate for low quality
        int l_framerate;       // framerate for low quality
        int l_height;          // height for low quality
        int l_width;           // width for low quality
        int l_qscale;          // qscale (parameter given to ffmpeg) for low quality

        // Same for medium quality
        int m_bitrate;
        int m_framerate;
        int m_height;
        int m_width;
        int m_qscale;

        // Same for high quality
        int h_bitrate;
        int h_framerate;
        int h_height;
        int h_width;
        int h_qscale;

        // keepalive and no traffic auto deconnexion
        int max_tick;
        int keepalive_grace_delay;

        char replay_path[1024];
        bool internal_domain;

        struct {
            uint32_t x224;
            uint32_t mcs;
            uint32_t sec;
            uint32_t rdp;
            uint32_t primary_orders;
            uint32_t secondary_orders;
            uint32_t bitmap;
            uint32_t capture;
            uint32_t auth;
            uint32_t session;
            uint32_t front;
            uint32_t mod_rdp;
            uint32_t mod_vnc;
            uint32_t mod_int;
            uint32_t mod_xup;
            uint32_t widget;
            uint32_t input;
        } debug;

    } globals;

    struct IniAccounts account;

    Inifile() {
        std::stringstream oss("");
        this->init();
        this->cparse(oss);
    }

    Inifile(const char * filename) {
        this->init();
        this->cparse(filename);
    }


    Inifile(istream & Inifile_stream) {
        this->init();
        this->cparse(Inifile_stream);
    }

    void init(){
            this->globals.capture_flags = 1; // 1 png, 2 wrm, 4 flv, 8 ocr
            this->globals.capture_wrm = true;
            this->globals.capture_png = true;
            this->globals.capture_flv = false;
            this->globals.capture_ocr = false;
            this->globals.capture_chunk = false;
            this->globals.capture_drawable = false;
            this->globals.bitmap_cache = true;
            this->globals.bitmap_compression = true;
            this->globals.port = 3389;
            this->globals.nomouse = false;
            this->globals.notimestamp = false;
            this->globals.encryptionLevel = level_from_string("low");
            this->globals.autologin = false;
            strcpy(this->globals.authip, "127.0.0.1");
            this->globals.authport = 3350;
            this->globals.authversion = 2;
            this->globals.autovalidate = false;
            strcpy(this->globals.dynamic_conf_path, "/tmp/rdpproxy/");
            strcpy(this->globals.codec_id, "flv");
            TODO("this could be some kind of enumeration")
            strcpy(this->globals.video_quality, "medium");


            this->globals.png_interval = 3000;
            this->globals.ocr_interval = 100; // 1 every second
            this->globals.frame_interval = 40;
            this->globals.break_interval = 600;
            this->globals.flv_break_interval = 600000000l;
            this->globals.flv_frame_interval = 1000000L; 

            this->globals.png_limit = 3;
            this->globals.l_bitrate   = 20000;
            this->globals.l_framerate = 1;
            this->globals.l_height    = 480;
            this->globals.l_width     = 640;
            this->globals.l_qscale    = 25;
            this->globals.m_bitrate   = 40000;
            this->globals.m_framerate = 1;
            this->globals.m_height    = 768;
            this->globals.m_width     = 1024;
            this->globals.m_qscale    = 15;
            this->globals.h_bitrate   = 200000;
            this->globals.h_framerate = 5;
            this->globals.h_height    = 1024;
            this->globals.h_width     = 1280;
            this->globals.h_qscale    = 15;
            this->globals.max_tick    = 30;
            this->globals.keepalive_grace_delay = 30;
            strcpy(this->globals.replay_path, "/tmp/");
            this->globals.internal_domain = false;
            this->globals.debug.x224              = 0;
            this->globals.debug.mcs               = 0;
            this->globals.debug.sec               = 0;
            this->globals.debug.rdp               = 0;
            this->globals.debug.primary_orders    = 0;
            this->globals.debug.secondary_orders  = 0;
            this->globals.debug.bitmap            = 0;
            this->globals.debug.capture           = 0;
            this->globals.debug.auth              = 0;
            this->globals.debug.session           = 0;
            this->globals.debug.front             = 0;
            this->globals.debug.mod_rdp           = 0;
            this->globals.debug.mod_vnc           = 0;
            this->globals.debug.mod_int           = 0;
            this->globals.debug.mod_xup           = 0;
            this->globals.debug.widget            = 0;
            this->globals.debug.input             = 0;

            memcpy(this->globals.auth_channel, "\0\0\0\0\0\0\0\0", 8);
            strcpy(this->account.accountname, "");
            strcpy(this->account.username, "");
            strcpy(this->account.password, "");
    };

    void cparse(istream & ifs){
        const size_t maxlen = 256;
        char line[maxlen];
        char context[128] = {0};
        bool truncated = false;
        while (ifs.good()){
            ifs.getline(line, maxlen);
            if (ifs.fail() && ifs.gcount() == (maxlen-1)){
                if (!truncated){
                    LOG(LOG_INFO, "Line too long in configuration file");
                    hexdump(line, maxlen-1);
                }
                ifs.clear();
                truncated = true;
                continue;
            }
            if (truncated){
                truncated = false;
                continue;
            }
            this->parseline(line, context);
        };
    }



    void parseline(const char * line, char * context)
    {
        char key[128];
        char value[128];

        const char * startkey = line;
        for (; *startkey ; startkey++) {
            if (!isspace(*startkey)){
                if (*startkey == '['){
                    const char * startcontext = startkey + 1;
                    const char * endcontext = strchr(startcontext, ']');
                    if (endcontext){
                        memcpy(context, startcontext, endcontext - startcontext);
                        context[endcontext - startcontext] = 0;
                    }
                    return;
                }
                break;
            }
        }
        const char * endkey = strchr(startkey, '=');
        if (endkey && endkey != startkey){
            const char * sep = endkey;
            for (--endkey; endkey >= startkey ; endkey--) {
                if (!isspace(*endkey)){
                    memcpy(key, startkey, endkey - startkey + 1);
                    key[endkey - startkey + 1] = 0;

                    const char * startvalue = sep + 1;
                    for ( ; *startvalue ; startvalue++) {
                        if (!isspace(*startvalue)){
                            break;
                        }
                    }
                    const char * endvalue;
                    for (endvalue = startvalue; *endvalue ; endvalue++) {
                        if (isspace(*endvalue) || *endvalue == '#'){
                            break;
                        }
                    }
                    memcpy(value, startvalue, endvalue - startvalue + 1);
                    value[endvalue - startvalue + 1] = 0;
                    this->setglobal(key, value, context);
                    break;
                }
            }
        }
    }

    void setglobal(const char * key, const char * value, const char * context)
    {
        if (0 == strcmp(context, "globals")){
            if (0 == strcmp(key, "bitmap_cache")){
                this->globals.bitmap_cache = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "bitmap_compression")){
                this->globals.bitmap_compression = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "port")){
                this->globals.port = atol(value);
            }
            else if (0 == strcmp(key, "nomouse")){
                this->globals.nomouse = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "notimestamp")){
                this->globals.notimestamp = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "encryptionLevel")){
                this->globals.encryptionLevel = level_from_string(value);
            }
            else if (0 == strcmp(key, "autologin")){
                this->globals.autologin = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "authip")){
                strcpy(this->globals.authip, value);
            }
            else if (0 == strcmp(key, "authport")){
                this->globals.authport = atol(value);
            }
            else if (0 == strcmp(key, "authversion")){
                this->globals.authversion = atol(value);
            }
            else if (0 == strcmp(key, "autovalidate")){
                this->globals.autovalidate = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "max_tick")){
                this->globals.max_tick    = atol(value);
            }
            else if (0 == strcmp(key, "keepalive_grace_delay")){
                this->globals.keepalive_grace_delay = atol(value);
            }
            else if (0 == strcmp(key, "internal_domain")){
                this->globals.internal_domain = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "dynamic_conf_path")){
                strcpy(this->globals.dynamic_conf_path, value);
            }
            else if (0 == strcmp(key, "auth_channel")){
                strncpy(this->globals.auth_channel, value, 8);
                this->globals.auth_channel[7] = 0;
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "video")){ 
            if (0 == strcmp(key, "capture_flags")){
                this->globals.capture_flags   = atol(value);
                this->globals.capture_png = 0 != (this->globals.capture_flags & 1);
                this->globals.capture_wrm = 0 != (this->globals.capture_flags & 2);
                this->globals.capture_flv = 0 != (this->globals.capture_flags & 4);
                this->globals.capture_ocr = 0 != (this->globals.capture_flags & 8);
            }
            else if (0 == strcmp(key, "ocr_interval")){
                this->globals.ocr_interval   = atol(value);
            }
            else if (0 == strcmp(key, "png_interval")){
                this->globals.png_interval   = atol(value);
            }
            else if (0 == strcmp(key, "frame_interval")){
                this->globals.frame_interval   = atol(value);
            }
            else if (0 == strcmp(key, "break_interval")){
                this->globals.break_interval   = atol(value);
            }
            else if (0 == strcmp(key, "png_limit")){
                this->globals.png_limit   = atol(value);
            }
            else if (0 == strcmp(key, "replay_path")){
                strcpy(this->globals.replay_path, value);
            }
            else if (0 == strcmp(key, "l_bitrate")){
                this->globals.l_bitrate   = atol(value);
            }
            else if (0 == strcmp(key, "l_framerate")){
                this->globals.l_framerate = atol(value);
            }
            else if (0 == strcmp(key, "l_height")){
                this->globals.l_height    = atol(value);
            }
            else if (0 == strcmp(key, "l_width")){
                this->globals.l_width     = atol(value);
            }
            else if (0 == strcmp(key, "l_qscale")){
                this->globals.l_qscale    = atol(value);
            }
            else if (0 == strcmp(key, "m_bitrate")){
                this->globals.m_bitrate   = atol(value);
            }
            else if (0 == strcmp(key, "m_framerate")){
                this->globals.m_framerate = atol(value);
            }
            else if (0 == strcmp(key, "m_height")){
                this->globals.m_height    = atol(value);
            }
            else if (0 == strcmp(key, "m_width")){
                this->globals.m_width     = atol(value);
            }
            else if (0 == strcmp(key, "m_qscale")){
                this->globals.m_qscale    = atol(value);
            }
            else if (0 == strcmp(key, "h_bitrate")){
                this->globals.h_bitrate   = atol(value);
            }
            else if (0 == strcmp(key, "h_framerate")){
                this->globals.h_framerate = atol(value);
            }
            else if (0 == strcmp(key, "h_height")){
                this->globals.h_height    = atol(value);
            }
            else if (0 == strcmp(key, "h_width")){
                this->globals.h_width     = atol(value);
            }
            else if (0 == strcmp(key, "h_qscale")){
                this->globals.h_qscale    = atol(value);
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "debug")){ 
            if (0 == strcmp(key, "x224")){
                this->globals.debug.x224              = atol(value);
            }
            else if (0 == strcmp(key, "mcs")){
                this->globals.debug.mcs               = atol(value);
            }
            else if (0 == strcmp(key, "sec")){
                this->globals.debug.sec               = atol(value);
            }
            else if (0 == strcmp(key, "rdp")){
                this->globals.debug.rdp               = atol(value);
            }
            else if (0 == strcmp(key, "primary_orders")){
                this->globals.debug.primary_orders    = atol(value);
            }
            else if (0 == strcmp(key, "secondary_orders")){
                this->globals.debug.secondary_orders  = atol(value);
            }
            else if (0 == strcmp(key, "bitmap")){
                this->globals.debug.bitmap            = atol(value);
            }
            else if (0 == strcmp(key, "capture")){
                this->globals.debug.capture           = atol(value);
            }
            else if (0 == strcmp(key, "auth")){
                this->globals.debug.auth              = atol(value);
            }
            else if (0 == strcmp(key, "session")){
                this->globals.debug.session           = atol(value);
            }
            else if (0 == strcmp(key, "front")){
                this->globals.debug.front             = atol(value);
            }
            else if (0 == strcmp(key, "mod_rdp")){
                this->globals.debug.mod_rdp           = atol(value);
            }
            else if (0 == strcmp(key, "mod_vnc")){
                this->globals.debug.mod_vnc           = atol(value);
            }
            else if (0 == strcmp(key, "mod_int")){
                this->globals.debug.mod_int           = atol(value);
            }
            else if (0 == strcmp(key, "mod_xup")){
                this->globals.debug.mod_xup           = atol(value);
            }
            else if (0 == strcmp(key, "widget")){
                this->globals.debug.widget            = atol(value);
            }
            else if (0 == strcmp(key, "input")){
                this->globals.debug.input            = atol(value);
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else {
            LOG(LOG_ERR, "unknown section [%s]", context);
        }
    }


    void cparse(const char * filename){
        ifstream inifile(filename);
        this->cparse(inifile);
    }



};

#endif
