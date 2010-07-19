/*
 * Alithia Engine
 * Copyright (C) 2009 Kostas Michalopoulos
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

#ifndef __GUI_H_INCLUDED__
#define __GUI_H_INCLUDED__

#define FONTSIZE 0.05

#define CB_DESTROYED 0
#define CB_ACTIVATED 1
#define CB_CHANGED 2

typedef struct _uicontrol_t
{
    struct _uicontrol_t* parent;
    list_t* children;
    char* text;
    int textlen;
    float x, y;
    float w, h;
    float wpref, hpref;
    void* layoutdata;
    void* ctldata;
    unsigned int ctlflags;
    void* userdata;

    void (*render)(struct _uicontrol_t* ctl);
    void (*render_children)(struct _uicontrol_t* ctl);
    void (*update)(struct _uicontrol_t* ctl);
    void (*layout)(struct _uicontrol_t* ctl);
    void (*dispose)(struct _uicontrol_t* ctl);
    void (*callback)(struct _uicontrol_t* ctl, int cbtype, void* cbdata);
    int (*handle_event)(struct _uicontrol_t* ctl, SDL_Event* ev);
    int (*block_child_at)(struct _uicontrol_t* ctl, float x, float y);
} uicontrol_t;

extern uicontrol_t* uiroot;

/* GUI funcs */
void gui_init(void);
void gui_render(void);
void gui_shutdown(void);
int gui_handle_event(SDL_Event* ev);
void gui_capture(uicontrol_t* ctl);
void gui_focus(uicontrol_t* ctl);

/* Drawing helpers */
void gui_color(float r, float g, float b, float a);
void gui_bar(float x, float y, float w, float h);

/* Generic controls functions */
uicontrol_t* uictl_new(uicontrol_t* parent);
void uictl_free(uicontrol_t* ctl);
void uictl_render(uicontrol_t* ctl);
void uictl_render_children(uicontrol_t* ctl);
void uictl_update(uicontrol_t* ctl);
void uictl_layout(uicontrol_t* ctl);
void uictl_callback(uicontrol_t* ctl, int cbtype, void* cbdata);
void uictl_place(uicontrol_t* ctl, float x, float y, float w, float h);
void uictl_add(uicontrol_t* parent, uicontrol_t* child);
void uictl_remove(uicontrol_t* parent, uicontrol_t* child);
void uictl_set_text(uicontrol_t* ctl, const char* text);
const char* uictl_get_text(uicontrol_t* ctl);
uicontrol_t* uictl_child_at(uicontrol_t* ctl, float x, float y);
void uictl_screen_to_local(uicontrol_t* ctl, float* x, float* y);
void uictl_local_to_screen(uicontrol_t* ctl, float* x, float* y);
void uictl_mouse_position(uicontrol_t* ctl, float* x, float* y);

/* Root */
uicontrol_t* uiroot_new(void);
uicontrol_t* uiroot_set(uicontrol_t* newroot);

/* Top-level windows */
uicontrol_t* uiwin_new(float x, float y, float w, float h, const char* title);
void uiwin_raise(uicontrol_t* win);
void uiwin_set_resizeable(uicontrol_t* win, int enable);
int uiwin_is_resizeable(uicontrol_t* win);

/* Label controls */
uicontrol_t* uilabel_new(uicontrol_t* parent, float x, float y, const char* text);

/* Button controls */
uicontrol_t* uibutton_new(uicontrol_t* parent, float x, float y, const char* text);

/* Field controls */
uicontrol_t* uifield_new(uicontrol_t* parent, float x, float y, float w);

/* Checkbox controls */
uicontrol_t* uicheckbox_new(uicontrol_t* parent, float x, float y, const char* text, int checked, unsigned int group);
void uicheckbox_check(uicontrol_t* chk, int checked);
int uicheckbox_checked(uicontrol_t* chk);

/* Text editor controls */
uicontrol_t* uieditor_new(uicontrol_t* parent, float x, float y, float w, float h);
void uieditor_append(uicontrol_t* ted, const char* text);
void uieditor_insert(uicontrol_t* ted, int index, const char* text);
int uieditor_line_count(uicontrol_t* ted);
const char* uieditor_get_line(uicontrol_t* ted, unsigned int index);
void uieditor_set_line(uicontrol_t* ted, unsigned int index, const char* text);
void uieditor_remove_line(uicontrol_t* ted, unsigned int index);

#endif
