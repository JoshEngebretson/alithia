/*
 * Alithia Engine
 * Copyright (C) 2009-2010 Kostas Michalopoulos
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Kostas Michalopoulos <badsector@runtimeterror.com>
 */

#include "atest.h"

static int selx1, sely1, selx2, sely2;
static int drag_selection, has_selection;
static int dsel_x1, dsel_y1;
static int cur_x1, cur_y1;
static int points_to_floor;
static int camera_mode = 1;
static float last_cell_y;

static uicontrol_t* editoruiroot;
static uicontrol_t* texture_browser;
static uicontrol_t* texture_browser_ctl;
static float texture_browser_scroll;

screen_t* editorscreen;

void editor_raisefloor_modifier(int cx, int cy, cell_t* cell, void* data)
{
    int points = *((int*)data);
    if (cell->floorz + points < cell->ceilz)
        cell->floorz += points;
    else
        cell->floorz = cell->ceilz;
}

void editor_lowerfloor_modifier(int cx, int cy, cell_t* cell, void* data)
{
    int points = *((int*)data);
    cell->floorz -= points;
}

void editor_raiseceiling_modifier(int cx, int cy, cell_t* cell, void* data)
{
    int points = *((int*)data);
    cell->ceilz += points;
}

void editor_lowerceiling_modifier(int cx, int cy, cell_t* cell, void* data)
{
    int points = *((int*)data);
    if (cell->ceilz - points > cell->floorz)
        cell->ceilz -= points;
    else
        cell->ceilz = cell->floorz;
}

void editor_makecolumn_modifier(int cx, int cy, cell_t* cell, void* data)
{
    cell->floorz = cell->ceilz = (cell->floorz + cell->ceilz)*0.5;
}

void editor_adjustflooroffs_modifier(int cx, int cy, cell_t* cell, void* data)
{
    int32_t di = *((int32_t*)data);
    cell->zfoffs[di&0x0F] += di>>4;
}

void editor_adjustceilingoffs_modifier(int cx, int cy, cell_t* cell, void* data)
{
    int32_t di = *((int32_t*)data);
    cell->zcoffs[di&0x0F] += di>>4;
}

void editor_setcelltexture_modifier(int cx, int cy, cell_t* cell, void* data)
{
    setcelltexture_modifier_data_t* mdata = data;
    switch (mdata->part) {
    case 0: cell->toptex = mdata->tex; break;
    case 1: cell->uppertex = mdata->tex; break;
    case 2: cell->lowetex = mdata->tex; break;
    case 3: cell->bottomtex = mdata->tex; break;
    }
}

void editor_apply_cell_modifier(modifier_proc_t proc, void* data)
{
    int x, y;
    if (!has_selection) {
        proc(cur_x1, cur_y1, &cell[cur_y1*map_width + cur_x1], data);
        map_update_cell(cur_x1, cur_y1);
        return;
    }
    for (y=sely1; y<=sely2; y++)
        for (x=selx1; x<=selx2; x++)
            proc(x, y, &cell[y*map_width + x], data);
    for (y=sely1; y<=sely2; y++)
        for (x=selx1; x<=selx2; x++)
            map_update_cell(x, y);
}

void editor_set_selection(int x1, int y1, int x2, int y2)
{
    if (x1 > x2) {
        selx1 = x2;
        selx2 = x1;
    } else {
        selx1 = x1;
        selx2 = x2;
    }
    if (y1 > y2) {
        sely1 = y2;
        sely2 = y1;
    } else {
        sely1 = y1;
        sely2 = y2;
    }
}

void editor_move_towards(float ix, float iy, float iz)
{
    plx += ix;
    if (plx < 0) plx = 0;
    else if (plx > CELLSIZE*(map_width - 1)) plx = (map_width - 1)*CELLSIZE;
    ply += iy;
    plz += iz;
    if (plz < 0) plz = 0;
    else if (plz > CELLSIZE*(map_height - 1)) plz = (map_height - 1)*CELLSIZE;
}

static void key_down(SDL_Event ev)
{
    switch (ev.key.keysym.sym) {
    case SDLK_RETURN:
        camera_mode = !camera_mode;
        break;
    case SDLK_o:
        if (SDL_GetModState() & KMOD_CTRL)
            disable_occlusion = !disable_occlusion;
        break;
    case SDLK_e:
        screen_set(gamescreen);
        break;
    case SDLK_c:
        editor_apply_cell_modifier(editor_makecolumn_modifier, NULL);
        break;
    case SDLK_l:
        if (lmap_needs_update)
            lmap_update();
        break;
    default: ;
    }
}

static void editorscreen_update(float ms)
{
    if (key[SDLK_w]) {
        editor_move_towards(-sin(pla*PI/180.0)*ms*0.55, sin(pll*PI/180.0)*ms*0.55, -cos(pla*PI/180.0)*ms*0.55);
    }
    if (key[SDLK_s]) {
        editor_move_towards(sin(pla*PI/180.0)*ms*0.55, -sin(pll*PI/180.0)*ms*0.55, cos(pla*PI/180.0)*ms*0.55);
    }
    if (key[SDLK_a]) {
        editor_move_towards(sin((pla-90)*PI/180.0)*ms*0.55, 0, cos((pla-90)*PI/180.0)*ms*0.55);
    }
    if (key[SDLK_d]) {
        editor_move_towards(sin((pla+90)*PI/180.0)*ms*0.55, 0, cos((pla+90)*PI/180.0)*ms*0.55);
    }
}

static void editorscreen_sdl_event(SDL_Event ev)
{
    pickdata_t pd;

    switch (ev.type) {
    case SDL_MOUSEBUTTONDOWN:
        if (ev.button.button == 1) {
            drag_selection = 1;
            dsel_x1 = cur_x1;
            dsel_y1 = cur_y1;
            has_selection = 1;
        }
        if (ev.button.button == 4) { /* scroll wheel up */
            int offs = -1;
            if (key[SDLK_i]) offs = 0;
            else if (key[SDLK_o]) offs = 1;
            else if (key[SDLK_k]) offs = 2;
            else if (key[SDLK_j]) offs = 3;
            else {
                offs = 4;
                editor_apply_cell_modifier(points_to_floor ? editor_raisefloor_modifier : editor_raiseceiling_modifier, &offs);
                return;
            }
            offs |= (1<<4);
            editor_apply_cell_modifier(points_to_floor ? editor_adjustflooroffs_modifier : editor_adjustceilingoffs_modifier, &offs);
        } else if (ev.button.button == 5) { /* scroll wheel down */
            int offs = -1;
            if (key[SDLK_i]) offs = 0;
            else if (key[SDLK_o]) offs = 1;
            else if (key[SDLK_k]) offs = 2;
            else if (key[SDLK_j]) offs = 3;
            else {
                offs = 4;
                editor_apply_cell_modifier(points_to_floor ? editor_lowerfloor_modifier : editor_lowerceiling_modifier, &offs);
                return;
            }
            offs |= -(1<<4);
            editor_apply_cell_modifier(points_to_floor ? editor_adjustflooroffs_modifier : editor_adjustceilingoffs_modifier, &offs);
        }
        break;
    case SDL_MOUSEBUTTONUP:
        if (ev.button.button == 1) {
            drag_selection = 0;
            if (cur_x1 == dsel_x1 && cur_y1 == dsel_y1) {
                has_selection = 0;
            }
        }
        break;
    case SDL_MOUSEMOTION:
        if (camera_mode || button[2]) {
            pla -= ev.motion.xrel/10.0;
            pll -= ev.motion.yrel/10.0;
            if (pll > 90) pll = 90;
            if (pll < -90) pll = -90;
        }

        if (pick(&centeraya, &centerayb, &pd)) {
            switch (pd.result) {
            case PICK_WORLD:
                cur_x1 = ((int)pd.ip.x)/CELLSIZE;
                cur_y1 = ((int)pd.ip.z)/CELLSIZE;
                if (cur_x1 < 1) cur_x1 = 1;
                else if (cur_x1 > map_width - 1) cur_x1 = map_width - 1;
                if (cur_y1 < 1) cur_y1 = 1;
                else if (cur_y1 > map_height - 1) cur_y1 = map_height - 1;
                last_cell_y = pd.cell->floorz;
                break;
            }
        } else {
            plane_t p;
            vector_t ip;
            p.nx = 0;
            p.ny = 1;
            p.nz = 0;
            p.d = last_cell_y;
            if (ray_plane_intersection(&p, &centeraya, &centerayb, &ip)) {
                cur_x1 = ((int)ip.x)/CELLSIZE;
                cur_y1 = ((int)ip.z)/CELLSIZE;
                if (cur_x1 < 1) cur_x1 = 1;
                else if (cur_x1 > map_width - 1) cur_x1 = map_width - 1;
                if (cur_y1 < 1) cur_y1 = 1;
                else if (cur_y1 > map_height - 1) cur_y1 = map_height - 1;
            }
        }

        if (drag_selection) {
            editor_set_selection(dsel_x1, dsel_y1, cur_x1, cur_y1);
        }
        break;
    case SDL_KEYDOWN:
        key_down(ev);
        break;
    }
}


static void fill_cell(int cx, int cy, int floor)
{
    int i;
    float vx[4], vy[4], vz[4];
    cell_vertices(cell + (cy*map_width) + cx, cx, cy, vx, vy, vz, floor);
    for (i=0; i<4; i++)
        glVertex3f(vx[i], vy[i] + (floor?0.1:-0.1), vz[i]);
}

static void draw_cell_cursor(int cx, int cy, int floor)
{
    int i;
    float vx[4], vy[4], vz[4];
    cell_vertices(cell + (cy*map_width) + cx, cx, cy, vx, vy, vz, floor);
    for (i=0; i<4; i++) vy[i] += floor?0.1:-0.1;

    if (!(cx&15)) glColor3f(0.5, 1.0, 0.5);
    else glColor3f(0.6, 0.6, 0.7);
    glVertex3f(vx[0], vy[0], vz[0]);
    glVertex3f(vx[1], vy[1], vz[1]);

    if (!((cx + 1)&15)) glColor3f(0.5, 1.0, 0.5);
    else glColor3f(0.6, 0.6, 0.7);
    glVertex3f(vx[2], vy[2], vz[2]);
    glVertex3f(vx[3], vy[3], vz[3]);

    if (!((cy + 1)&15)) glColor3f(0.5, 1.0, 0.5);
    else glColor3f(0.6, 0.6, 0.7);
    glVertex3f(vx[1], vy[1], vz[1]);
    glVertex3f(vx[2], vy[2], vz[2]);

    if (!(cy&15)) glColor3f(0.5, 1.0, 0.5);
    else glColor3f(0.6, 0.6, 0.7);
    glVertex3f(vx[3], vy[3], vz[3]);
    glVertex3f(vx[0], vy[0], vz[0]);
}

static void draw_cell_grid(int x1, int y1, int x2, int y2, int floor)
{
    int x, y;
    if (x1 < 1) x1 = 1;
    if (x2 > map_width - 2) x2 = map_width - 2;
    if (y1 < 1) y1 = 1;
    if (y2 > map_height - 2) y2 = map_height - 2;
    for (y=y1; y<=y2; y++) {
        for (x=x1; x<=x2; x++)
            draw_cell_cursor(x, y, floor);
    }
}

static void fill_selection(int floor)
{
    int x, y, x1 = selx1, y1 = sely1, x2 = selx2, y2 = sely2;
    if (x1 < 1) x1 = 1;
    if (x2 > map_width - 2) x2 = map_width - 2;
    if (y1 < 1) y1 = 1;
    if (y2 > map_height - 2) y2 = map_height - 2;
    for (y=y1; y<=y2; y++) {
        for (x=x1; x<=x2; x++)
            fill_cell(x, y, floor);
    }
}

static void editorscreen_render(void)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE1);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);

    points_to_floor = centeraya.y > centerayb.y;

    /* Draw grid around cursor */
    glLineWidth(2);
    glBegin(GL_LINES);
    draw_cell_grid(cur_x1 - 4, cur_y1 - 4, cur_x1 + 4, cur_y1 + 4, points_to_floor);
    glEnd();
    glLineWidth(1);

    /* Draw cursor and selection */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
    glColor4f(1.0, 0.4, 0.4, 0.4);
    fill_cell(cur_x1, cur_y1, points_to_floor);
    if (has_selection) {
        glColor4f(1.0, 0.4, 1.0, 0.4);
        fill_selection(points_to_floor);
    }
    glEnd();

    glDisable(GL_BLEND);

    if (camera_mode) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glPointSize(4);
        glBegin(GL_POINTS);
        glVertex2f(0, 0);
        glEnd();
    }

    glPopClientAttrib();
    glPopAttrib();
}

static int editorscreen_proc(screen_t* scr, int msg, void* data)
{
    switch (msg) {
    case SMS_INIT:
        scr->draw_world = 1;
        scr->do_clear = 1;
        break;
    case SMS_UPDATE:
        editorscreen_update(*((float*)data));
        scr->draw_gui = !(button[2] || camera_mode);
        break;
    case SMS_SDL_EVENT:
        editorscreen_sdl_event(*((SDL_Event*)data));
        break;
    case SMS_RENDER:
        editorscreen_render();
        break;
    case SMS_ENTER:
        uiroot_set(editoruiroot);
        break;
    }
    return 0;
}

static void texture_browser_ctl_render(uicontrol_t* tb)
{
    float x, y, adv;
    size_t i;

    x = tb->x + 0.02;
    adv = tb->w + 0.04;
    glEnable(GL_TEXTURE_2D);
    for (i=0; i<texbank_items; i++) {
        y = tb->y + tb->h - 0.04*hw_ratio + texture_browser_scroll - i*(tb->w + 0.06);
        glColor4f(1, 1, 1, 1);
        glBindTexture(GL_TEXTURE_2D, texbank_item[i]->tex->name);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex2f(x, y);
        glTexCoord2f(0, 1);
        glVertex2f(x, y - adv);
        glTexCoord2f(1, 1);
        glVertex2f(x + tb->w - 0.04, y - adv);
        glTexCoord2f(1, 0);
        glVertex2f(x + tb->w - 0.04, y);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, 0);
        glColor4f(1, 0, 0, 1);
        glBegin(GL_LINES);
        glVertex2f(x, y - adv/4.0);
        glVertex2f(x + tb->w - 0.04, y - adv/4.0);
        glVertex2f(x, y - (adv/4.0)*2.0);
        glVertex2f(x + tb->w - 0.04, y - (adv/4.0)*2.0);
        glVertex2f(x, y - (adv/4.0)*3.0);
        glVertex2f(x + tb->w - 0.04, y - (adv/4.0)*3.0);
        glEnd();
        y -= (tb->w + 0.06);
    }
    glDisable(GL_TEXTURE_2D);
}

static int texture_browser_ctl_handle_event(uicontrol_t* ctl, SDL_Event* ev)
{
    float mx, my;
    uictl_mouse_position(ctl, &mx, &my);

    switch (ev->type) {
    case SDL_MOUSEBUTTONDOWN:
        if (ev->button.button == 1) {
            setcelltexture_modifier_data_t mdata;
            int idx = -(int)((my - ctl->y - ctl->h + 0.04*hw_ratio - texture_browser_scroll)/(ctl->w + 0.06));
            float y1, y2;
            if (idx < 0 || idx >= texbank_items) return 1;
            y1 = ctl->y + ctl->h - 0.04*hw_ratio + texture_browser_scroll - idx*(ctl->w + 0.06);
            y2 = ctl->y + ctl->h - 0.04*hw_ratio + texture_browser_scroll - (idx + 1)*(ctl->w + 0.06) + 0.02;
            if (my < y2) return 1;
            mdata.part = (int)(((my - y1)/(y2 - y1))/0.25f);
            mdata.tex = texbank_item[idx]->tex;
            editor_apply_cell_modifier(editor_setcelltexture_modifier, &mdata);
            return 1;
        } else if (ev->button.button == 4) {
            texture_browser_scroll -= 0.3;
            if (texture_browser_scroll < 0)
                texture_browser_scroll = 0;
            return 1;
        } else if (ev->button.button == 5) {
            texture_browser_scroll += 0.3;
            return 1;
        }
        return 0;
    }
    return 0;
}

static void texture_browser_layout(uicontrol_t* win)
{
    uictl_place(texture_browser_ctl, 0, 0, win->w, win->h - FONTSIZE*1.3);
}

static void create_texture_browser(void)
{
    texture_browser = uiwin_new(1.7, 0, 0.3, 2, "Texture Browser");
    uiwin_set_resizeable(texture_browser, 1);
    texture_browser_ctl = uictl_new(texture_browser);
    texture_browser_ctl->render = texture_browser_ctl_render;
    texture_browser_ctl->handle_event = texture_browser_ctl_handle_event;

    texture_browser->layout = texture_browser_layout;
    texture_browser_layout(texture_browser);
}

void editor_init(void)
{
    editorscreen = screen_new(editorscreen_proc, NULL);
    editoruiroot = uiroot_new();
    uiroot_set(editoruiroot);
    create_texture_browser();
}

void editor_shutdown(void)
{
    uictl_free(editoruiroot);
    screen_free(editorscreen);
}
