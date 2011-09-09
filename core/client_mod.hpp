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

   module object. Some kind of interface between core and modules

*/
#if !defined(__MOD_HPP__)
#define __MOD_HPP__

#include <string.h>
#include <stdio.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <locale.h>

#include "client_info.hpp"
#include "font.hpp"
#include "cache.hpp"
#include "front.hpp"
#include "mainloop.hpp"
#include "bitmap_cache.hpp"
#include "wait_obj.hpp"
#include "keymap.hpp"
#include "callback.hpp"

struct client_mod : public Callback {
    int (& keys)[256];
    int & key_flags;
    Keymap * &keymap;
    Rect clip;
    int current_pointer;
    RDPPen pen;
    bool clipboard_enable;
    bool pointer_displayed;
    Front & front;
    int sck;
    char ip_source[256];
    int rdp_compression;
    int bitmap_cache_persist_enable;
    BGRPalette palette332;
    BGRPalette mod_palette;
    BGRPalette memblt_mod_palette;
    bool mod_palette_setted;

    uint8_t mod_bpp;
    uint8_t socket;

    wait_obj * event;
    int signal;
    bool palette_sent;
    bool palette_memblt_sent[6];
    struct Capture * capture;

    client_mod(int (& keys)[256], int & key_flags, Keymap * &keymap, Front & front)
        : keys(keys),
          key_flags(key_flags),
          keymap(keymap),
          front(front),
          mod_palette_setted(0),
          mod_bpp(24),
          signal(0),
          palette_sent(false),
          capture(0)
    {
        for (size_t i = 0; i < 6 ; i++){
            this->palette_memblt_sent[i] = false;
        }
        this->current_pointer = 0;
        this->clip = Rect(0,0,4096,2048);
        this->pointer_displayed = false;

        init_palette332(this->palette332);
    }

    virtual ~client_mod()
    {
        if (this->capture){
            delete this->capture;
        }
    }


    int callback(int msg, long param1, long param2, long param3, long param4)
    {
        int rv = 0;
        switch (msg) {
        case 0: /* RDP_INPUT_SYNCHRONIZE */
//            LOG(LOG_INFO, "callback RDP_INPUT_SYNCHRONIZE");
            /* happens when client gets focus and sends key modifier info */
            this->key_flags = param1;
            // why do we not keep device flags ?
            this->mod_event(17, param1, param3, param1, param3);
            break;
        case RDP_INPUT_SCANCODE:
            this->scancode(param1, param2, param3, param4, this->key_flags, *this->keymap, this->keys);
            break;
        case 0x8001: /* RDP_INPUT_MOUSE */
            rv = this->input_mouse(param3, param1, param2);
            break;
        case WM_SCREENUPDATE:
//            LOG(LOG_INFO, "callback SCREENUPDATE");
            this->invalidate(Rect(param1, param2, param3, param4));
            break;
        case WM_CHANNELDATA:
//            LOG(LOG_INFO, "callback CHANNELDATA");
            rv = this->mod_event(WM_CHANNELDATA, param1, param2, param3, param4);
            break;
        default:
            LOG(LOG_INFO, "callback unsupported msg %u", msg);
            break;
        }
        return rv;
    }

    virtual void send_to_mod_channel(const McsChannelItem & channel, uint8_t * data, size_t size, size_t length, uint32_t flags)
    {
    }

    void set_mod_palette(const BGRPalette & palette)
    {
        this->mod_palette_setted = true;
        for (unsigned i = 0; i < 256 ; i++){
            this->mod_palette[i] = palette[i];
            this->memblt_mod_palette[i] = RGBtoBGR(palette[i]);
        }
    }

    const BGRColor convert(const BGRColor color) const
    {
        if (this->get_front_bpp() == 8 && this->mod_bpp == 8){
//            return ((color >> 5) & 7) |((color << 1) & 0x31)|((color<<6)&0xc0);
//            this->mod_palette[color]
            return color;
        }
        else{
            const BGRColor color24 = color_decode(color, this->mod_bpp, this->mod_palette);
            return color_encode(color24, this->get_front_bpp());
        }
    }

    const BGRColor convert_opaque(const BGRColor color) const
    {
        if (this->get_front_bpp() == 8 && this->mod_bpp == 8){
//            LOG(LOG_INFO, "convert_opaque: front=%u back=%u setted=%u color=%u palette=%.06x", this->get_front_bpp(), this->mod_bpp, this->mod_palette_setted, color, this->mod_palette[color]);
//            return ((color >> 5) & 7) |((color << 1) & 0x31)|((color<<6)&0xc0);
//            this->mod_palette[color]
            return color;
        }
        else
        if (this->mod_bpp == 16 || this->mod_bpp == 15 || this->mod_bpp == 8){
            const BGRColor color24 = color_decode_opaquerect(
                        color, this->mod_bpp, this->mod_palette);
            return  color_encode(color24, this->get_front_bpp());
        }
        else {
            const BGRColor color24 = color_decode(color, this->mod_bpp, this->mod_palette);
            return color_encode(color24, this->get_front_bpp());
        }
    }

    const BGRColor convert24_opaque(const BGRColor color) const
    {
        if (this->mod_bpp == 16 || this->mod_bpp == 15){
            const BGRColor color24 = color_decode_opaquerect(
                        color, this->mod_bpp, this->mod_palette);
            return  color_encode(color24, 24);
        }
        else if (this->mod_bpp == 8) {
            const BGRColor color24 = color_decode(color, this->mod_bpp, this->mod_palette);
            return RGBtoBGR(color_encode(color24, 24));
        }
        else {
            const BGRColor color24 = color_decode(color, this->mod_bpp, this->mod_palette);
            return color_encode(color24, 24);
        }
    }

    const BGRColor convert24(const BGRColor color) const
    {
        const BGRColor color24 = color_decode(color, this->mod_bpp, this->mod_palette);
        return color_encode(color24, 24);
    }

    uint32_t convert_to_black(uint32_t color)
    {
        return 0; // convert(color);
    }

    void start_capture(int width, int height, bool flag, char * path,
                const char * codec_id, const char * quality, int timezone)
    {
        this->front.timezone = timezone;
        if (flag){
            this->stop_capture();
            this->capture = new Capture(width, height, 24, path, codec_id, quality);
        }
    }

    void stop_capture()
    {
        if (this->capture){
            delete this->capture;
            this->capture = 0;
        }
    }

    void periodic_snapshot(bool pointer_is_displayed)
    {
        if (this->capture){
            this->capture->snapshot(this->front.mouse_x, this->front.mouse_y,
                    pointer_is_displayed|this->front.nomouse, this->front.notimestamp, this->front.timezone);
        }
    }

    /* client functions */
    virtual int mod_event(int msg, long param1, long param2, long param3, long param4) = 0;

    // mod_signal should be run when client socket received some data (mod_event is set)
    // In other words: when some data came from "server" and should be taken care of
    // mod_signal returns not 0 (return status) when the module finished
    // (connection to remote or internal server closed)
    // and returns 0 as long as the connection with server is still active.
    virtual int mod_signal(void) = 0;

    virtual void scancode(long param1, long param2, long param3, long param4, int & key_flags, Keymap & keymap, int keys[]){
        param1 = param1 % 128;
        int msg = WM_KEYUP;
        keys[param1] = 1 | param3;
        if ((param3 & KBD_FLAG_UP) == 0) { /* 0x8000 */
            /* key down */
            msg = WM_KEYDOWN;
            switch (param1) {
            case 58:
                key_flags ^= 4;
                break; /* caps lock */
            case 69:
                key_flags ^= 2;
                break; /* num lock */
            case 70:
                key_flags ^= 1;
                break; /* scroll lock */
            default:
                ;
            }
        }
        if (&keymap != NULL)
        {
            struct key_info* ki = keymap.get_key_info_from_scan_code(
                            param3,
                            param1,
                            keys,
                            key_flags);
            if (ki != 0) {
                this->mod_event(msg, ki->chr, ki->sym, param1, param3);
            }
        }
        if (msg == WM_KEYUP){
            keys[param1] = 0;
        }
    }

    int server_begin_update() {
        this->front.begin_update();
        return 0;
    }

    int server_end_update(){
        this->front.end_update();
        return 0;
    }


    const ClientInfo & get_client_info() const {
        return this->front.get_client_info();
    }

    int get_front_bpp() const {
        return this->get_client_info().bpp;
    }

    int get_front_width() const {
        return this->get_client_info().width;
    }

    int get_front_height() const {
        return this->get_client_info().height;
    }

    virtual void front_resize() {
    }

    const Rect get_front_rect(){
        return Rect(0, 0, this->get_front_width(), get_front_height());
    }

    void server_resize(int width, int height, int bpp)
    {
        const ClientInfo & client_info = this->get_client_info();

        if (client_info.width != width
        || client_info.height != height
        || client_info.bpp != bpp) {
            /* older client can't resize */
            if (client_info.build <= 419) {
                LOG(LOG_ERR, "Resizing is not available on older RDP clients");
                return;
            }
            this->palette_sent = false;
            for (size_t i = 0; i < 6 ; i++){
                this->palette_memblt_sent[i] = false;
            }
            LOG(LOG_INFO, "// Resizing client to : %d x %d x %d\n", width, height, bpp);

            this->front.set_client_info(width, height, bpp);
            this->front_resize();
            this->front.reset();
        }

        this->server_reset_clip();
    }


    #warning this function is written in a quite insane way, so don't use it, and rewrite it in a saner way.
    #warning also merge with the similar code in widget.
    #warning implementation of the server_draw_text function below is totally broken, especially data. MS-RDPEGDI See 2.2.2.2.1.1.2.13 GlyphIndex (GLYPHINDEX_ORDER)

    void server_draw_text(uint16_t x, uint16_t y, const char * text, uint32_t fgcolor, uint32_t bgcolor)
    {
        this->send_global_palette();

        // add text to glyph cache
        #warning use mbsrtowcs instead
        int len = mbstowcs(0, text, 0);
        wchar_t* wstr = new wchar_t[len + 2];
        mbstowcs(wstr, text, len + 1);
        int total_width = 0;
        int total_height = 0;
        uint8_t *data = new uint8_t[len * 4];
        memset(data, 0, len * 4);
        int f = 0;
        int c = 0;
        int distance_from_previous_fragment = 0;
        for (int index = 0; index < len; index++) {
            FontChar* font_item = this->front.font.font_items[wstr[index]];
            #warning avoid passing parameters by reference to get results
            switch (this->front.cache.add_glyph(font_item, f, c))
            {
                case Cache::GLYPH_ADDED_TO_CACHE:
                    this->glyph_cache(*font_item, f, c);
                break;
                default:
                break;
            }
            data[index * 2] = c;
            data[index * 2 + 1] = distance_from_previous_fragment;
            distance_from_previous_fragment = font_item->incby;
            total_width += font_item->incby;
            total_height = std::max(total_height, font_item->height);
        }

        #warning there seems to be some strange behavior with bk rect (and op ?). Same problem as usual, we have a rectangle but we don't know if boundaries (right, bottom) are included or not. Check actual behavior with rdesktop and with mstsc client. Nevertheless we shouldn't have to add 1 here.
        const Rect bk(x, y, total_width, total_height);

         RDPGlyphIndex glyphindex(
            f, // cache_id
            0x03, // fl_accel
            0x0, // ui_charinc
            1, // f_op_redundant,
            bgcolor, // bgcolor
            fgcolor, // fgcolor
            bk, // bk
            Rect(), // op
            // brush
            RDPBrush(0, 0, 3, 0xaa,
                (const uint8_t *)"\xaa\x55\xaa\x55\xaa\x55\xaa\x55"),
            x,  // glyph_x
            y + total_height, // glyph_y
            len * 2, // data_len in bytes
            data // data
        );

        this->glyph_index(glyphindex);

        delete [] wstr;
        delete [] data;
    }

    void opaque_rect(const RDPOpaqueRect & cmd)
    {
        if (!clip.isempty() && !clip.intersect(cmd.rect).isempty()){

            this->send_global_palette();

            RDPOpaqueRect new_cmd = cmd;
            new_cmd.color = this->convert_opaque(cmd.color);
            this->front.orders->send(new_cmd, this->clip);

            if (this->capture){
                RDPOpaqueRect new_cmd24 = cmd;
                new_cmd24.color = this->convert24_opaque(cmd.color);
                this->capture->opaque_rect(new_cmd24, this->clip);
            }
        }
    }

    void scr_blt(const RDPScrBlt & cmd)
    {
        if (!this->clip.isempty()
        && !this->clip.intersect(cmd.rect).isempty()){
            this->front.orders->send(cmd, clip);

            if (this->capture){
                this->capture->scr_blt(cmd, clip);
            }
        }
    }

    void dest_blt(const RDPDestBlt & cmd)
    {
        if (!this->clip.isempty()
        && !this->clip.intersect(cmd.rect).isempty()){
            this->front.orders->send(cmd, this->clip);
            if (this->capture){
                this->capture->dest_blt(cmd, this->clip);
            }
        }
    }

    void pat_blt(const RDPPatBlt & cmd)
    {
        if (!this->clip.isempty() && !this->clip.intersect(cmd.rect).isempty()){

            this->send_global_palette();

            RDPPatBlt new_cmd = cmd;
            new_cmd.back_color = this->convert(cmd.back_color);
            new_cmd.fore_color = this->convert(cmd.fore_color);

            if (new_cmd.brush.style == 3){
                if (this->get_client_info().brush_cache_code == 1) {
                    uint8_t pattern[8];
                    pattern[0] = new_cmd.brush.hatch;
                    memcpy(pattern+1, new_cmd.brush.extra, 7);
                    int cache_idx = 0;
                    if (BRUSH_TO_SEND == this->front.cache.add_brush(pattern, cache_idx)){
                        this->brush_cache(cache_idx);
                    }
                    new_cmd.brush.hatch = cache_idx;
                    new_cmd.brush.style = 0x81;
                }
            }
            this->front.orders->send(new_cmd, this->clip);

            if (this->capture){
                RDPPatBlt new_cmd24 = cmd;
                new_cmd24.back_color = this->convert24(cmd.back_color);
                new_cmd24.fore_color = this->convert24(cmd.fore_color);

                this->capture->pat_blt(new_cmd24, this->clip);
            }
        }
    }

    void mem_blt(const RDPMemBlt & memblt, Bitmap & bitmap, const BGRPalette & palette)
    {
        uint8_t palette_id = ((memblt.cache_id >> 4) >= 6)?0:(memblt.cache_id >> 4);

        if (this->get_front_bpp() == 8){
            #warning there may be some performance issue when client is 8 bits, palette is sent way too often
            this->palette_sent = false;
            this->send_global_palette();
            if (!this->palette_memblt_sent[palette_id]) {
                this->color_cache(palette, palette_id);
                this->palette_memblt_sent[palette_id] = true;
            }
            this->palette_sent = false;
        }

        const Rect & dst = memblt.rect;
        const int srcx = memblt.srcx;
        const int srcy = memblt.srcy;
        const uint16_t width = bitmap.cx;
        const uint16_t height = bitmap.cy;
        const Rect src_r(srcx, srcy, width, height);
        const uint8_t * src_data = bitmap.data_co(this->get_front_bpp());
        const uint8_t rop = memblt.rop;

        for (int y = 0; y < dst.cy ; y += 32) {
            int cy = std::min(32, dst.cy - y);
            for (int x = 0; x < dst.cx ; x += 32) {
                int cx = std::min(32, dst.cx - x);
                const Rect tile(x, y, cx, cy);
                if (!this->clip.intersect(tile.offset(dst.x, dst.y)).isempty()){
                    #warning transmit a bitmap to add_bitmap instead of individual components
                     uint32_t cache_ref = this->front.bmp_cache->add_bitmap(
                                                src_r.cx, src_r.cy,
                                                src_data,
                                                tile.offset(src_r.x, src_r.y),
                                                this->get_front_bpp(),
                                                bitmap.original_palette);

                    uint8_t send_type = (cache_ref >> 24);
                    uint8_t cache_id  = (cache_ref >> 16);
                    uint16_t cache_idx = (cache_ref & 0xFFFF);

                    if (send_type == BITMAP_ADDED_TO_CACHE){
                        this->bitmap_cache(cache_id, cache_idx);
                    }

                    const RDPMemBlt cmd(cache_id + palette_id*16, tile.offset(dst.x, dst.y), rop, 0, 0, cache_idx);
//                    cmd.log(LOG_INFO, this->clip);

                    if (!this->clip.isempty()
                    && !this->clip.intersect(cmd.rect).isempty()){
                        this->front.orders->send(cmd, this->clip);
                        #warning capture should have it's own reference to bmp_cache
                        if (this->capture){
                            this->capture->mem_blt(cmd, *this->front.bmp_cache, this->clip);
                        }
                    }
                }
            }
        }
    }

    #warning move out server_set_pen
    int server_set_pen(int style, int width)
    {
        this->pen.style = style;
        this->pen.width = width;
        return 0;
    }

    void line_to(const RDPLineTo & cmd)
    {
        const uint16_t minx = std::min(cmd.startx, cmd.endx);
        const uint16_t miny = std::min(cmd.starty, cmd.endy);
        const Rect rect(minx, miny,
                        std::max(cmd.startx, cmd.endx)-minx+1,
                        std::max(cmd.starty, cmd.endy)-miny+1);

        if (!this->clip.isempty() && !this->clip.intersect(rect).isempty()){

            RDPLineTo new_cmd = cmd;
            new_cmd.back_color = this->convert(cmd.back_color);
            new_cmd.pen.color = this->convert(cmd.pen.color);

            this->front.orders->send(new_cmd, clip);

            if (this->capture){
                RDPLineTo new_cmd24 = cmd;
                new_cmd24.back_color = this->convert24(cmd.back_color);
                new_cmd24.pen.color = this->convert24(cmd.pen.color);

                this->capture->line_to(new_cmd24, clip);
            }
        }
    }

    void glyph_index(const RDPGlyphIndex & cmd)
    {
        if (!this->clip.isempty() && !this->clip.intersect(cmd.bk).isempty()){
            this->send_global_palette();

            RDPGlyphIndex new_cmd = cmd;
            new_cmd.back_color = this->convert_opaque(cmd.back_color);
            new_cmd.fore_color = this->convert_opaque(cmd.fore_color);

            if (new_cmd.brush.style == 3){
                if (this->get_client_info().brush_cache_code == 1) {
                    uint8_t pattern[8];
                    pattern[0] = new_cmd.brush.hatch;
                    memcpy(pattern+1, new_cmd.brush.extra, 7);
                    int cache_idx = 0;
                    if (BRUSH_TO_SEND == this->front.cache.add_brush(pattern, cache_idx)){
                        this->brush_cache(cache_idx);
                    }
                    new_cmd.brush.hatch = cache_idx;
                    new_cmd.brush.style = 0x81;
                }
            }

            this->front.orders->send(new_cmd, this->clip);

            if (this->capture){
                RDPGlyphIndex new_cmd24 = cmd;
                new_cmd24.back_color = this->convert24_opaque(cmd.back_color);
                new_cmd24.fore_color = this->convert24_opaque(cmd.fore_color);

                this->capture->glyph_index(new_cmd24, this->clip);
            }
        }
    }

    void brush_cache(const int index)
    {
        RDPBrushCache cmd(index, 1, 8, 8, 0x81,
            sizeof(this->front.cache.brush_items[index].pattern),
            this->front.cache.brush_items[index].pattern);
        this->front.orders->send(cmd);
    }


    void color_cache(const BGRPalette & palette, uint8_t cacheIndex)
    {
        RDPColCache cmd(cacheIndex, palette);
        this->front.orders->send(cmd);
    }

    void bitmap_cache(const uint8_t cache_id, const uint16_t cache_idx)
    {
        BitmapCacheItem * entry =  this->front.bmp_cache->get_item(cache_id, cache_idx);

        RDPBmpCache cmd(this->get_front_bpp(), entry->pbmp, cache_id, cache_idx, &(this->get_client_info()));
        this->front.orders->send(cmd);

        if (this->capture){
            this->capture->bitmap_cache(cmd);
        }

    }

    void glyph_cache(const FontChar & font_char, int font_index, int char_index)
    {
        RDPGlyphCache cmd(font_index, 1, char_index, font_char.offset, font_char.baseline, font_char.width, font_char.height, font_char.data);
        this->front.orders->send(cmd);
    }

    #warning this should become BITMAP UPDATE, we should be able to send bitmaps either through orders and cache or through BITMAP UPDATE
    void bitmap_update(Bitmap & bitmap, const Rect & dst, int srcx, int srcy)
    {
        const uint16_t width = bitmap.cx;
        const uint16_t height = bitmap.cy;
        const Rect src_r(srcx, srcy, width, height);
        const int palette_id = 0;
        const uint8_t * src_data = bitmap.data_co(this->get_front_bpp());
        const uint8_t rop = 0xCC;

        for (int y = 0; y < dst.cy ; y += 32) {
            int cy = std::min(32, dst.cy - y);
            for (int x = 0; x < dst.cx ; x += 32) {
                int cx = std::min(32, dst.cx - x);
                const Rect tile(x, y, cx, cy);
                if (!this->clip.intersect(tile.offset(dst.x, dst.y)).isempty()){
                     uint32_t cache_ref = this->front.bmp_cache->add_bitmap(
                                                src_r.cx, src_r.cy,
                                                src_data,
                                                tile.offset(src_r.x, src_r.y),
                                                this->get_front_bpp(),
                                                bitmap.original_palette);

                    uint8_t send_type = (cache_ref >> 24);
                    uint8_t cache_id  = (cache_ref >> 16);
                    uint16_t cache_idx = (cache_ref & 0xFFFF);

                    if (send_type == BITMAP_ADDED_TO_CACHE){
                        this->bitmap_cache(cache_id, cache_idx);
                    }

                    const RDPMemBlt cmd(cache_id, tile.offset(dst.x, dst.y), rop, 0, 0, cache_idx);
                    if (!this->clip.isempty()
                    && !this->clip.intersect(cmd.rect).isempty()){
                        if (this->get_front_bpp() == 8){
                            if (!this->palette_memblt_sent[palette_id]) {
                                if (bitmap.original_bpp == 8){
                                    this->color_cache(this->mod_palette, palette_id);
                                }
                                else {
                                    this->color_cache(this->palette332, palette_id);
                                }
                                this->palette_memblt_sent[palette_id] = true;
                            }
                            this->palette_sent = false;
                        }
                        this->front.orders->send(cmd, this->clip);
                        #warning capture should have it's own reference to bmp_cache
                        if (this->capture){
                            this->capture->mem_blt(cmd, *this->front.bmp_cache, this->clip);
                        }
                    }
                }
            }
        }
    }

    void set_pointer(int cache_idx)
    {
        this->front.set_pointer(cache_idx);
        this->current_pointer = cache_idx;
    }

    void send_global_palette()
    {
        if (!this->palette_sent && (this->get_front_bpp() == 8)){
            if (this->mod_bpp == 8){
                this->front.send_global_palette(this->memblt_mod_palette);
            }
            else {
                this->front.send_global_palette(this->palette332);
            }
            this->palette_sent = true;
        }
    }

    void server_set_pointer(int x, int y, uint8_t* data, uint8_t* mask)
    {
        int cacheidx = 0;
        switch (this->front.cache.add_pointer(data, mask, x, y, cacheidx)){
        case POINTER_TO_SEND:
            this->front.send_pointer(cacheidx, data, mask, x, y);
        break;
        default:
        case POINTER_ALLREADY_SENT:
            this->set_pointer(cacheidx);
        break;
        }
    }

    virtual void invalidate(const Rect & r)
    {
        if (!r.isempty()) {
            this->mod_event(WM_INVALIDATE,
                ((r.x & 0xffff) << 16) | (r.y & 0xffff),
                ((r.cx & 0xffff) << 16) | (r.cy & 0xffff),
                0, 0);
        }
    }

    int server_is_term()
    {
        return g_is_term();
    }

    void server_set_clip(const Rect & rect)
    {
        this->clip = rect;
    }

    void server_reset_clip()
    {
        this->clip = this->get_front_rect();
    }

    void server_add_char(int font, int character,
                    int offset, int baseline,
                    int width, int height, const uint8_t* data)
    {
        struct FontChar fi(offset, baseline, width, height, 0);
        memcpy(fi.data, data, fi.datasize());
        this->glyph_cache(fi, font, character);
    }

    int server_get_channel_id(char* name)
    {
        return this->front.get_channel_id(name);
    }

    void server_send_to_channel_mod(const McsChannelItem & channel,
                           uint8_t* data, int data_len,
                           int total_data_len, int flags)
    {
        this->front.send_to_channel(channel, data, data_len, total_data_len, flags);
    }

    bool get_pointer_displayed() {
        return this->pointer_displayed;
    }

    void set_pointer_display() {
        this->pointer_displayed = true;
    }

    int input_mouse(int device_flags, int x, int y)
    {
        if (device_flags & MOUSE_FLAG_MOVE) { /* 0x0800 */
            this->mod_event(WM_MOUSEMOVE, x, y, 0, 0);
            this->front.mouse_x = x;
            this->front.mouse_y = y;

        }
        if (device_flags & MOUSE_FLAG_BUTTON1) { /* 0x1000 */
            this->mod_event(
                WM_LBUTTONUP + ((device_flags & MOUSE_FLAG_DOWN) >> 15),
                x, y, 0, 0);
        }
        if (device_flags & MOUSE_FLAG_BUTTON2) { /* 0x2000 */
            this->mod_event(
                WM_RBUTTONUP + ((device_flags & MOUSE_FLAG_DOWN) >> 15),
                x, y, 0, 0);
        }
        if (device_flags & MOUSE_FLAG_BUTTON3) { /* 0x4000 */
            this->mod_event(
                WM_BUTTON3UP + ((device_flags & MOUSE_FLAG_DOWN) >> 15),
                x, y, 0, 0);
        }
        if (device_flags == MOUSE_FLAG_BUTTON4 || /* 0x0280 */ device_flags == 0x0278) {
            this->mod_event(WM_BUTTON4DOWN, x, y, 0, 0);
            this->mod_event(WM_BUTTON4UP, x, y, 0, 0);
        }
        if (device_flags == MOUSE_FLAG_BUTTON5 || /* 0x0380 */ device_flags == 0x0388) {
            this->mod_event(WM_BUTTON5DOWN, x, y, 0, 0);
            this->mod_event(WM_BUTTON5UP, x, y, 0, 0);
        }
        return 0;
    }

};

#endif
