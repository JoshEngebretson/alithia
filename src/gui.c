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

#include "atest.h"

#define MAX_CLIPS 256

typedef struct _clipbox_t
{
    float x1;
    float y1;
    float x2;
    float y2;
} clipbox_t;

static uicontrol_t* capture = NULL;
static uicontrol_t* focus = NULL;
static float save_x;
static float save_y;
static clipbox_t clipbox[MAX_CLIPS];
static int clipboxhead = 0;
static list_t* checkboxes = NULL;
uicontrol_t* uiroot = NULL;

/* Helper rendering functions */
static void set_clip(float x1, float y1, float x2, float y2)
{
    x1 = round(vid_width*((x1 + 1.0f)/2.0f));
    y1 = round(vid_height*((y1 + 1.0f)/2.0f));
    x2 = round(vid_width*((x2 + 1.0f)/2.0f));
    y2 = round(vid_height*((y2 + 1.0f)/2.0f));
    if (x2 <= x1 || y2 <= y1) {
        glScissor(0, 0, 0, 0);
    } else {
        glScissor((int)x1, (int)y1, (int)(x2 - x1), (int)(y2 - y1));
    }
}

static void open_clip(float x1, float y1, float x2, float y2)
{
    if (clipboxhead == MAX_CLIPS) return;
    if (clipboxhead) {
        if (x1 < clipbox[clipboxhead - 1].x1) x1 = clipbox[clipboxhead - 1].x1;
        if (y1 < clipbox[clipboxhead - 1].y1) y1 = clipbox[clipboxhead - 1].y1;
        if (x2 > clipbox[clipboxhead - 1].x2) x2 = clipbox[clipboxhead - 1].x2;
        if (y2 > clipbox[clipboxhead - 1].y2) y2 = clipbox[clipboxhead - 1].y2;
    }
    clipbox[clipboxhead].x1 = x1;
    clipbox[clipboxhead].y1 = y1;
    clipbox[clipboxhead].x2 = x2;
    clipbox[clipboxhead].y2 = y2;
    clipboxhead++;
    set_clip(x1, y1, x2, y2);
}

static void close_clip(void)
{
    if (!clipboxhead) {
        glScissor(0, 0, 0, 0);
        return;
    }
    clipboxhead--;
    if (!clipboxhead) {
        glScissor(0, 0, 0, 0);
        return;
    }
    set_clip(clipbox[clipboxhead - 1].x1, clipbox[clipboxhead - 1].y1, clipbox[clipboxhead - 1].x2, clipbox[clipboxhead - 1].y2);
}

void gui_color(float r, float g, float b, float a)
{
    if (a < 1.0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }
    glColor4f(r, g, b, a);
}

void gui_bar(float x, float y, float w, float h)
{
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x, y + h);
    glVertex2f(x + w, y + h);
    glVertex2f(x + w, y);
    glEnd();
}

/* GUI funcs */
void do_layout(uicontrol_t* win)
{
    uicontrol_t* ted = win->userdata;
    uictl_place(ted, 0.02*hw_ratio, 0.02, win->w - 0.04*hw_ratio, win->h - FONTSIZE*1.3 - 0.04);
}

void gui_init(void)
{
    /*
    uicontrol_t* win;
    uicontrol_t* lbl;
    uicontrol_t* btn;
    uicontrol_t* fld;
    uicontrol_t* chk;
    uicontrol_t* ted;

    uiroot = uictl_new(NULL);
    uictl_place(uiroot, -1, -1, 2, 2);

    checkboxes = list_new();

    win = uiwin_new(0.4*hw_ratio, 1.0, 0.6*hw_ratio, 0.35, "A window");
    lbl = uilabel_new(win, 0, 0, "Hello, world");
    btn = uibutton_new(win, FONTSIZE*0.15*hw_ratio, lbl->hpref*1.5, "Push me!");
    fld = uifield_new(win, FONTSIZE*0.15*hw_ratio, lbl->hpref*3.5, btn->wpref);
    chk = uicheckbox_new(win, btn->wpref + FONTSIZE*0.6*hw_ratio, lbl->hpref*3.5, "Check me", 0, 0);
    chk = uicheckbox_new(win, btn->wpref + FONTSIZE*0.6*hw_ratio, lbl->hpref*2.0, "This...", 1, 1);
    chk = uicheckbox_new(win, btn->wpref + FONTSIZE*0.6*hw_ratio, lbl->hpref*1.0, "...or that", 0, 1);

    win = uiwin_new(0.8, 0.7, 1.5*hw_ratio, 0.75, "Another window");
    uiwin_set_resizeable(win, 1);
    ted = uieditor_new(win, 0.02*hw_ratio, 0.02, win->w - 0.04*hw_ratio, win->h - FONTSIZE*1.3 - 0.04);
    win->layout = do_layout;
    win->userdata = ted;
    */
}

void gui_render(void)
{
    int i;

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    for (i=0; i<8; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glDisable(GL_TEXTURE_2D);
    }
    glActiveTexture(GL_TEXTURE0);

    glEnable(GL_SCISSOR_TEST);
    clipboxhead = 0;

    if (uiroot) uictl_render(uiroot);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glPopClientAttrib();
    glPopAttrib();
}

void gui_shutdown(void)
{
    list_free(checkboxes);
}

int gui_handle_event(SDL_Event* ev)
{
    uicontrol_t* target = capture;
    if (!uiroot) return 0;
    if (!target && (ev->type == SDL_KEYDOWN || ev->type == SDL_KEYUP) && focus) target = focus;
    if (!target) target = uictl_child_at(uiroot, mouse_x + 1.0, mouse_y + 1.0);
    while (target) {
        if (target->handle_event && target->handle_event(target, ev))
            return 1;
        target = target->parent;
    }
    return 0;
}

void gui_capture(uicontrol_t* ctl)
{
    capture = ctl;
}

void gui_focus(uicontrol_t* ctl)
{
    focus = ctl;
}

uicontrol_t* gui_get_focused(void)
{
    return focus;
}

/* Generic controls functions */
uicontrol_t* uictl_new(uicontrol_t* parent)
{
    uicontrol_t* ctl = new(uicontrol_t);
    ctl->wpref = ctl->hpref = -1;
    if (parent) uictl_add(parent, ctl);
    return ctl;
}

void uictl_free(uicontrol_t* ctl)
{
    if (!ctl) return;
    uictl_callback(ctl, CB_DESTROYED, NULL);
    if (capture == ctl) capture = NULL;
    if (ctl->parent) uictl_remove(ctl->parent, ctl);
    if (ctl->dispose) ctl->dispose(ctl);
    if (ctl->ctldata) free(ctl->ctldata);
    if (ctl->text) free(ctl->text);
    if (ctl->children) {
        listitem_t* itchild;
        for (itchild=ctl->children->first; itchild; itchild=itchild->next)
            ((uicontrol_t*)itchild->ptr)->parent = NULL;
        list_free(ctl->children);
    }
    free(ctl);
}

void uictl_render(uicontrol_t* ctl)
{
    float x = 0, y = 0;
    uictl_local_to_screen(ctl, &x, &y);
    open_clip(x, y, x + ctl->w - pixelw, y + ctl->h - pixelh);
    if (ctl->render) {
        ctl->render(ctl);
    } else {
        uictl_render_children(ctl);
    }
    close_clip();
}

void uictl_render_children(uicontrol_t* ctl)
{
    if (!ctl->children) return;

    if (ctl->render_children) {
        ctl->render_children(ctl);
    } else {
        listitem_t* itchild;
        glPushMatrix();
        glTranslatef(ctl->x, ctl->y, 0.0);

        for (itchild=ctl->children->first; itchild; itchild=itchild->next) {
            uicontrol_t* child = itchild->ptr;
            uictl_render(child);
        }

        glPopMatrix();
    }
}

void uictl_update(uicontrol_t* ctl)
{
    if (ctl->update) ctl->update(ctl);
}

void uictl_layout(uicontrol_t* ctl)
{
    if (ctl->layout) ctl->layout(ctl);
}

void uictl_callback(uicontrol_t* ctl, int cbtype, void* cbdata)
{
    if (ctl->callback) ctl->callback(ctl, cbtype, cbdata);
}

void uictl_place(uicontrol_t* ctl, float x, float y, float w, float h)
{
    int need_layout = ctl->w != w || ctl->h != h;
    ctl->x = x;
    ctl->y = y;
    ctl->w = w;
    ctl->h = h;
    if (need_layout) uictl_layout(ctl);
}

void uictl_add(uicontrol_t* parent, uicontrol_t* child)
{
    if (!parent->children) {
        parent->children = list_new();
        parent->children->item_free = (void*)uictl_free;
    }
    list_add(parent->children, child);
    child->parent = parent;
    uictl_update(parent);
    uictl_update(child);
    uictl_layout(parent);
}

void uictl_remove(uicontrol_t* parent, uicontrol_t* child)
{
    list_remove(parent->children, child, 0);
    child->parent = NULL;
    uictl_update(parent);
    uictl_update(child);
    uictl_layout(parent);
}

void uictl_set_text(uicontrol_t* ctl, const char* text)
{
    if (ctl->text) free(ctl->text);
    ctl->text = (text && text[0]) ? strdup(text) : NULL;
    ctl->textlen = ctl->text ? strlen(ctl->text) : 0;
    uictl_update(ctl);
}

const char* uictl_get_text(uicontrol_t* ctl)
{
    return ctl->text ? ctl->text : "";
}

uicontrol_t* uictl_child_at(uicontrol_t* ctl, float x, float y)
{
    listitem_t* itchild;
    if (ctl->block_child_at && ctl->block_child_at(ctl, x, y))
        return ctl;
    if (ctl->children) {
        for (itchild=ctl->children->last; itchild; itchild=itchild->prev) {
            uicontrol_t* child = itchild->ptr;

            if (x >= child->x && y >= child->y && x < child->x + child->w && y < child->y + child->h) {
                uicontrol_t* grandchild = uictl_child_at(child, x - child->x, y - child->y);
                return grandchild ? grandchild : child;
            }
        }
    }

    return ctl;
}

void uictl_screen_to_local(uicontrol_t* ctl, float* x, float* y)
{
    while (ctl) {
        *x -= ctl->x;
        *y -= ctl->y;
        ctl = ctl->parent;
    }
}

void uictl_local_to_screen(uicontrol_t* ctl, float* x, float* y)
{
    while (ctl) {
        *x += ctl->x;
        *y += ctl->y;
        ctl = ctl->parent;
    }
}

void uictl_mouse_position(uicontrol_t* ctl, float* x, float* y)
{
    *x = mouse_x;
    *y = mouse_y;
    uictl_screen_to_local(ctl, x, y);
}

/* Root */
uicontrol_t* uiroot_new(void)
{
    return uictl_new(NULL);
}

uicontrol_t* uiroot_set(uicontrol_t* newroot)
{
    uicontrol_t* prev = uiroot;
    uiroot = newroot;
    uictl_place(uiroot, -1, -1, 2, 2);
    return prev;
}

/* Popup menus */
menu_t* uimenu_new(void)
{
    menu_t* menu = new(menu_t);
    menu->root = menu;
    return menu;
}

void uimenu_free(menu_t* menu)
{
    size_t i;
    if (!menu) return;
    for (i=0; i<menu->items; i++) {
        free(menu->item[i].title);
        uimenu_free(menu->item[i].submenu);
    }
    free(menu->item);
    if (menu->ctrl)
        uictl_free(menu->ctrl);
    free(menu);
}

menuitem_t* uimenu_add(menu_t* menu, const char* title, menu_t* submenu, menuitem_callback_t callback, void* cbdata)
{
    menu->item = realloc(menu->item, sizeof(menuitem_t)*(menu->items + 1));
    menu->item[menu->items].callback = callback;
    menu->item[menu->items].callback_data = cbdata;
    menu->item[menu->items].menu = menu;
    menu->item[menu->items].root = menu->root;
    menu->item[menu->items].submenu = submenu;
    menu->item[menu->items].title = strdup(title);
    if (submenu) submenu->root = menu->root;
    menu->items++;
    menu->index = menu->items;
    return menu->item + (menu->items - 1);
}

menuitem_t* uimenu_find(menu_t* menu, const char* title)
{
    size_t i;
    for (i=0; i<menu->items; i++)
        if (!strcmp(title, menu->item[i].title))
            return menu->item + i;
    return NULL;
}

static void uimenu_ctl_render(uicontrol_t* ctl)
{
    menu_t* menu = ctl->userdata;
    size_t i;
    gui_color(0.0, 0.0, 0.0, 0.5);
    glDisable(GL_SCISSOR_TEST);
    gui_bar(ctl->x + 0.02*hw_ratio, ctl->y - 0.02, ctl->w, ctl->h);
    glEnable(GL_SCISSOR_TEST);
    gui_color(0.3, 0.35, 0.37, 1.0);
    gui_bar(ctl->x, ctl->y, ctl->w, ctl->h);
    gui_color(0.0, 0.0, 0.0, 1.0);
    for (i=0; i<menu->items; i++) {
        if ((i == menu->index || (menu->submenu_visible && i == menu->submenu_visible_index)) && menu->item[i].title[0] != '-') {
            gui_color(0.2, 0.25, 0.27, 1.0);
            gui_bar(ctl->x, ctl->y + ctl->h - i*FONTSIZE*1.3 - FONTSIZE*1.3, ctl->w, FONTSIZE*1.3);
            gui_color(0.0, 0.0, 0.0, 1.0);
        }
        if (menu->item[i].title[0] == '-') {
            gui_bar(ctl->x, ctl->y + ctl->h - i*FONTSIZE*1.3 - FONTSIZE*1.3*0.5, ctl->w, pixelh);
        } else {
            font_render(font_normal, ctl->x + 0.01*hw_ratio, ctl->y + ctl->h - i*FONTSIZE*1.3 - FONTSIZE*1.3, menu->item[i].title, -1, FONTSIZE);
        }
    }
}

static int uimenu_ctl_handle_event(uicontrol_t* ctl, SDL_Event* ev)
{
    float mx, my;
    int idx, retval = 0;
    menu_t* menu = ctl->userdata;
    uictl_mouse_position(ctl, &mx, &my);

    switch (ev->type) {
    case SDL_MOUSEBUTTONDOWN:
        if (!menu->submenu_visible && (mx < 0 || my < 0 || mx > ctl->w || my > ctl->h)) {
            uimenu_hide(menu->root);
            return 1;
        }
        if (menu->index < menu->items) {
            if (menu->item[menu->index].callback) {
                ((menuitem_callback_t)menu->item[menu->index].callback)(MI_SELECT, menu->item + menu->index, menu->item[menu->index].callback_data);
                uimenu_hide(menu->root);
                return 1;
            }
        }
        retval = 1;
        break;
    case SDL_MOUSEMOTION:
        if (mx < 0 || mx > ctl->w)
            idx = -1;
        else
            idx = (ctl->h - my)/(FONTSIZE*1.3);
        if (idx < 0 || idx > (int)menu->items)
            idx = menu->items;
        if (idx != (int)menu->index) {
            menu->index = idx;
            if (menu->index != menu->items) {
                if (menu->submenu_visible) {
                    uimenu_hide(menu->submenu_visible);
                    menu->submenu_visible = NULL;
                    gui_capture(menu->ctrl);
                }
                if (menu->item[menu->index].callback)
                    ((menuitem_callback_t)menu->item[menu->index].callback)(MI_HIGHLIGHT, menu->item + menu->index, menu->item[menu->index].callback_data);
                if (menu->item[menu->index].submenu) {
                    uimenu_show(menu->item[menu->index].submenu, menu->ctrl->x + menu->ctrl->w,
                        menu->ctrl->y + menu->ctrl->h - FONTSIZE*1.3*menu->index);
                    menu->submenu_visible = menu->item[menu->index].submenu;
                    menu->item[menu->index].submenu->parent = menu;
                    menu->item[menu->index].submenu->root = menu->root;
                    menu->submenu_visible_index = menu->index;
                }
            }
        }
        retval = 1;
        break;
    }

    if (menu->parent && menu->parent->ctrl)
        return uimenu_ctl_handle_event(menu->parent->ctrl, ev);

    return retval;
}

void uimenu_show(menu_t* menu, float x, float y)
{
    size_t i;
    float maxw = 0;
    if (menu->ctrl) uimenu_hide(menu);
    menu->ctrl = uictl_new(uiroot);
    menu->ctrl->userdata = menu;
    menu->ctrl->render = uimenu_ctl_render;
    menu->ctrl->handle_event = uimenu_ctl_handle_event;
    menu->submenu_visible = NULL;
    for (i=0; i<menu->items; i++) {
        float itw = font_width(font_normal, menu->item[i].title, -1, FONTSIZE);
        if (itw > maxw) maxw = itw;
    }
    y = y - FONTSIZE*1.3*menu->items;
    if (y < 0) y = 0;
    uictl_place(menu->ctrl, x, y, maxw + 0.02*hw_ratio, FONTSIZE*1.3*menu->items);
    gui_capture(menu->ctrl);
}

void uimenu_hide(menu_t* menu)
{
    if (!menu->ctrl) return;
    if (menu->submenu_visible) {
        uimenu_hide(menu->submenu_visible);
        menu->submenu_visible = NULL;
    }
    uictl_free(menu->ctrl);
    menu->ctrl = NULL;
    gui_capture(NULL);
    if (menu->closed) menu->closed(menu);
}

/* Top-level windows */
#define WINF_MOVING 0x0001
#define WINF_RESIZEABLE 0x0002
#define WINF_RESIZING 0x0004

static void win_render(uicontrol_t* win)
{
    gui_color(0.0, 0.0, 0.0, 0.5);
    glDisable(GL_SCISSOR_TEST);
    gui_bar(win->x + 0.03*hw_ratio, win->y - 0.03, win->w, win->h);
    gui_bar(win->x - pixelw, win->y - pixelh, win->w + pixelw*1, win->h + pixelh*1);
    glEnable(GL_SCISSOR_TEST);
    gui_color(0.3, 0.35, 0.37, 1.0);
    gui_bar(win->x, win->y, win->w, win->h - FONTSIZE*1.3);

    uictl_render_children(win);

    if (win->ctlflags & WINF_RESIZEABLE) {
        gui_color(0.4, 0.45, 0.47, 1.0);
        gui_bar(win->x + win->w - 0.04*hw_ratio - pixelw, win->y, 0.04*hw_ratio, 0.04);
    }
    gui_color(0.4, 0.45, 0.47, 1.0);
    gui_bar(win->x, win->y + win->h - FONTSIZE*1.3, win->w, FONTSIZE*1.3);
    gui_color(0.0, 0.0, 0.0, 1.0);
    font_render(font_normal, win->x + FONTSIZE*0.15*hw_ratio, win->y + win->h - FONTSIZE*1.15, win->text, win->textlen, FONTSIZE);
}

static int win_block_child_at(uicontrol_t* win, float x, float y)
{
    if (y > win->h - FONTSIZE*1.3) return 1;
    if ((win->ctlflags & WINF_RESIZEABLE) && y < 0.04 && x < win->w && x > win->w - 0.04*hw_ratio) return 1;
    return 0;
}

static int win_handle_event(uicontrol_t* win, SDL_Event* ev)
{
    float mx, my;
    uictl_mouse_position(win, &mx, &my);

    switch (ev->type) {
    case SDL_MOUSEBUTTONDOWN:
        if (ev->button.button == 1) {
            if (my > win->h - FONTSIZE*1.3) {
                win->ctlflags |= WINF_MOVING;
                save_x = mouse_x;
                save_y = mouse_y;
                gui_capture(win);
                uiwin_raise(win);
                return 1;
            }
            if ((win->ctlflags & WINF_RESIZEABLE) && my < 0.04 && mx < win->w && mx > win->w - 0.04*hw_ratio) {
                win->ctlflags |= WINF_RESIZING;
                save_x = mouse_x;
                save_y = mouse_y;
                gui_capture(win);
                uiwin_raise(win);
                return 1;
            }
        }
        return 1;
    case SDL_MOUSEBUTTONUP:
        if (win->ctlflags & WINF_MOVING) {
            win->ctlflags &= ~WINF_MOVING;
            gui_capture(NULL);
            return 1;
        }
        if (win->ctlflags & WINF_RESIZING) {
            win->ctlflags &= ~WINF_RESIZING;
            gui_capture(NULL);
            return 1;
        }
        return 1;
    case SDL_MOUSEMOTION:
        if (win->ctlflags & WINF_MOVING) {
            uictl_place(win, win->x - (save_x - mouse_x), win->y - (save_y - mouse_y), win->w, win->h);
            save_x = mouse_x;
            save_y = mouse_y;
            return 1;
        }
        if (win->ctlflags & WINF_RESIZING) {
            float new_w = win->w - save_x + mouse_x;
            float new_h = win->h + save_y - mouse_y;
            float new_y = win->y - save_y + mouse_y;
            if (new_w < floor(FONTSIZE*2.3/pixelw)*pixelw) new_w = floor(FONTSIZE*2.3/pixelw)*pixelw;
            if (new_h < floor(FONTSIZE*2.3/pixelh)*pixelh) new_h = floor(FONTSIZE*2.3/pixelh)*pixelh;
            uictl_place(win, win->x, new_y, new_w, new_h);
            save_x = mouse_x;
            save_y = mouse_y;
            return 1;
        }
        return 0;
    }
    return 0;
}

uicontrol_t* uiwin_new(float x, float y, float w, float h, const char* title)
{
    uicontrol_t* win = uictl_new(uiroot);
    win->render = win_render;
    win->handle_event = win_handle_event;
    win->block_child_at = win_block_child_at;
    uictl_set_text(win, title);
    uictl_place(win, x, y, w, h);
    return win;
}

void uiwin_raise(uicontrol_t* win)
{
    listitem_t* it = list_find(uiroot->children, win);
    it->ptr = uiroot->children->last->ptr;
    uiroot->children->last->ptr = win;
}

void uiwin_set_resizeable(uicontrol_t* win, int enable)
{
    if (enable)
        win->ctlflags |= WINF_RESIZEABLE;
    else
        win->ctlflags &= ~WINF_RESIZEABLE;
}

int uiwin_is_resizeable(uicontrol_t* win)
{
    return win->ctlflags&WINF_RESIZEABLE;
}


/* Label controls */
static void label_render(uicontrol_t* lbl)
{
    gui_color(0.0, 0.0, 0.0, 1.0);
    font_render(font_normal, lbl->x, lbl->y, lbl->text, lbl->textlen, FONTSIZE);
}

static void label_update(uicontrol_t* lbl)
{
    lbl->wpref = font_width(font_normal, lbl->text, lbl->textlen, FONTSIZE);
    lbl->hpref = FONTSIZE;
}

uicontrol_t* uilabel_new(uicontrol_t* parent, float x, float y, const char* text)
{
    uicontrol_t* lbl = uictl_new(parent);
    lbl->render = label_render;
    lbl->update = label_update;
    uictl_set_text(lbl, text);
    uictl_place(lbl, x, y, lbl->wpref, lbl->hpref);
    return lbl;
}

/* Button controls */
#define BTNF_DOWN 0x0001
#define BTNF_OVER 0x0002

static void button_render(uicontrol_t* btn)
{
    if ((btn->ctlflags & BTNF_DOWN) && (btn->ctlflags & BTNF_OVER)) {
        gui_color(0.4, 0.45, 0.47, 1.0);
        gui_bar(btn->x + 0.01*hw_ratio, btn->y - 0.01, btn->w, btn->h);
        gui_color(0.0, 0.0, 0.0, 1.0);
        font_render(font_normal, btn->x + FONTSIZE*0.15 + 0.01*hw_ratio, btn->y + FONTSIZE*0.1 - 0.01, btn->text, btn->textlen, FONTSIZE);
    } else {
        glDisable(GL_SCISSOR_TEST);
        gui_color(0.0, 0.0, 0.0, 0.5);
        gui_bar(btn->x + 0.02*hw_ratio, btn->y - 0.02, btn->w, btn->h);
        glEnable(GL_SCISSOR_TEST);
        gui_color(0.4, 0.45, 0.47, 1.0);
        gui_bar(btn->x, btn->y, btn->w, btn->h);
        gui_color(0.0, 0.0, 0.0, 1.0);
        font_render(font_normal, btn->x + FONTSIZE*0.15, btn->y + FONTSIZE*0.1, btn->text, btn->textlen, FONTSIZE);
    }
}

static void button_update(uicontrol_t* btn)
{
    btn->wpref = font_width(font_normal, btn->text, btn->textlen, FONTSIZE) + FONTSIZE*0.3;
    btn->hpref = FONTSIZE*1.3;
}

static int button_handle_event(uicontrol_t* btn, SDL_Event* ev)
{
    float mx, my;
    uictl_mouse_position(btn, &mx, &my);

    switch (ev->type) {
    case SDL_MOUSEBUTTONDOWN:
        if (ev->button.button == 1) {
            btn->ctlflags |= BTNF_DOWN|BTNF_OVER;
            save_x = mouse_x;
            save_y = mouse_y;
            gui_capture(btn);
            gui_focus(btn);
            return 1;
        }
        return 0;
    case SDL_MOUSEBUTTONUP:
        if (btn->ctlflags & BTNF_DOWN) {
            btn->ctlflags &= ~BTNF_DOWN;
            gui_capture(NULL);
            if (btn->ctlflags & BTNF_OVER) {
                btn->ctlflags &= ~BTNF_OVER;
                uictl_callback(btn, CB_ACTIVATED, NULL);
            }
            return 1;
        }
        return 0;
    case SDL_MOUSEMOTION:
        if (mx >= 0 && my >= 0 && mx < btn->w && my < btn->h)
            btn->ctlflags |= BTNF_OVER;
        else
            btn->ctlflags &= ~BTNF_OVER;
        return 1;
    case SDL_KEYDOWN:
        if (ev->key.keysym.sym == SDLK_RETURN || ev->key.keysym.sym == SDLK_SPACE)
            uictl_callback(btn, CB_ACTIVATED, NULL);
    }
    return 0;
}

uicontrol_t* uibutton_new(uicontrol_t* parent, float x, float y, const char* text)
{
    uicontrol_t* btn = uictl_new(parent);
    btn->render = button_render;
    btn->update = button_update;
    btn->handle_event = button_handle_event;
    uictl_set_text(btn, text);
    uictl_place(btn, x, y, btn->wpref, btn->hpref);
    return btn;
}


/* Field controls */
typedef struct _uifield_data_t
{
    float xscroll;
    int cursor;
} uifield_data_t;

static void field_render(uicontrol_t* fld)
{
    uifield_data_t* data = fld->ctldata;
    if (focus == fld)
        gui_color(0.22, 0.27, 0.29, 1.0);
    else
        gui_color(0.20, 0.25, 0.27, 1.0);
    gui_bar(fld->x, fld->y, fld->w, fld->h);
    if (focus == fld)
        gui_color(0.27, 0.32, 0.35, 1.0);
    else
        gui_color(0.25, 0.3, 0.33, 1.0);
    gui_bar(fld->x + 0.01*hw_ratio, fld->y, fld->w - 0.01*hw_ratio, fld->h - 0.01 - pixelh);
    gui_color(0.0, 0.0, 0.0, 1.0);
    font_render(font_normal, fld->x + FONTSIZE*0.15*hw_ratio - data->xscroll, fld->y + FONTSIZE*0.15, fld->text, fld->textlen, FONTSIZE);
    if (focus == fld) {
        float w = font_width(font_normal, fld->text, data->cursor, FONTSIZE);
        gui_color(0.5, 0.2, 0.1, 1.0);
        gui_bar(fld->x + FONTSIZE*0.15*hw_ratio + w - data->xscroll, fld->y + 0.005, 3*pixelw, fld->h - 0.01);
    }
}

static void field_check_scroll(uicontrol_t* fld)
{
    uifield_data_t* data = fld->ctldata;
    float curpos = font_width(font_normal, fld->text, data->cursor, FONTSIZE) - data->xscroll;
    while (curpos >= fld->w - 0.01*hw_ratio - FONTSIZE*0.15*hw_ratio) {
        data->xscroll += fld->w/2;
        curpos -= fld->w/2;
    }
    while (curpos < 0) {
        data->xscroll -= fld->w/2;
        curpos += fld->w/2;
        if (data->xscroll < 0) data->xscroll = 0;
    }
}

static int field_handle_event(uicontrol_t* fld, SDL_Event* ev)
{
    uifield_data_t* data = fld->ctldata;
    float mx, my;
    uictl_mouse_position(fld, &mx, &my);

    switch (ev->type) {
    case SDL_MOUSEBUTTONDOWN:
        if (ev->button.button == 1) {
            gui_focus(fld);
            return 1;
        }
        return 0;
    case SDL_KEYDOWN:
        if (ev->key.keysym.sym == SDLK_HOME) {
            data->cursor = 0;
            field_check_scroll(fld);
        } else if (ev->key.keysym.sym == SDLK_END) {
            data->cursor = fld->textlen;
            field_check_scroll(fld);
        } else if (ev->key.keysym.sym == SDLK_LEFT) {
            if (data->cursor) data->cursor--;
            field_check_scroll(fld);
        } else if (ev->key.keysym.sym == SDLK_RIGHT) {
            if (data->cursor < fld->textlen) data->cursor++;
            field_check_scroll(fld);
        } else if (ev->key.keysym.sym == SDLK_BACKSPACE) {
            if (data->cursor) {
                char* newtext = malloc(fld->textlen);
                char* tmp = strdup(fld->text);
                tmp[data->cursor - 1] = 0;
                sprintf(newtext, "%s%s", tmp, fld->text + data->cursor);
                uictl_set_text(fld, newtext);
                free(tmp);
                free(newtext);
                data->cursor--;
                field_check_scroll(fld);
                uictl_callback(fld, CB_CHANGED, NULL);
            }
            return 1;
        } else if (ev->key.keysym.sym == SDLK_DELETE) {
            if (fld->text && fld->text[data->cursor]) {
                char* newtext = malloc(fld->textlen);
                char* tmp = strdup(fld->text);
                tmp[data->cursor] = 0;
                sprintf(newtext, "%s%s", tmp, fld->text + data->cursor + 1);
                uictl_set_text(fld, newtext);
                free(tmp);
                free(newtext);
                uictl_callback(fld, CB_CHANGED, NULL);
            }
            return 1;
        } else if (!(ev->key.keysym.unicode & 0xFF80) && ev->key.keysym.unicode > 31) {
            char* newtext = malloc(fld->textlen + 2);
            char* tmp = strdup(uictl_get_text(fld));
            tmp[data->cursor] = 0;
            sprintf(newtext, "%s%c%s", tmp, ev->key.keysym.unicode, uictl_get_text(fld) + data->cursor);
            uictl_set_text(fld, newtext);
            free(tmp);
            free(newtext);
            data->cursor++;
            field_check_scroll(fld);
            uictl_callback(fld, CB_CHANGED, NULL);
            return 1;
        }
    }
    return 0;
}

uicontrol_t* uifield_new(uicontrol_t* parent, float x, float y, float w)
{
    uicontrol_t* fld = uictl_new(parent);
    fld->render = field_render;
    fld->handle_event = field_handle_event;
    fld->ctldata = new(uifield_data_t);
    uictl_place(fld, x, y, w, FONTSIZE*1.3);
    return fld;
}


/* Checkbox controls */
typedef struct _checkbox_data_t
{
    int checked;
    unsigned int group;
} checkbox_data_t;

static void checkbox_render(uicontrol_t* chk)
{
    checkbox_data_t* data = chk->ctldata;
    gui_color(0.4, 0.45, 0.47, 1.0);
    gui_bar(chk->x + FONTSIZE*0.1*hw_ratio, chk->y + FONTSIZE*0.2, chk->h*hw_ratio - FONTSIZE*0.4*hw_ratio, chk->h - FONTSIZE*0.4);
    gui_color(0.3, 0.35, 0.37, 1.0);
    gui_bar(chk->x + FONTSIZE*0.1*hw_ratio + pixelw*2, chk->y + FONTSIZE*0.2 + pixelh*2.0, chk->h*hw_ratio - FONTSIZE*0.4*hw_ratio - pixelw*4.0, chk->h - FONTSIZE*0.4 - pixelh*4.0);
    if (data->checked) {
        gui_color(0.4, 0.45, 0.47, 1.0);
        if (data->group)
            gui_bar(chk->x + FONTSIZE*0.1*hw_ratio + pixelw*5, chk->y + FONTSIZE*0.2 + pixelh*5.0, chk->h*hw_ratio - FONTSIZE*0.4*hw_ratio - pixelw*10.0, chk->h - FONTSIZE*0.4 - pixelh*10.0);
        else
            gui_bar(chk->x + FONTSIZE*0.1*hw_ratio + pixelw*4, chk->y + FONTSIZE*0.2 + pixelh*4.0, chk->h*hw_ratio - FONTSIZE*0.4*hw_ratio - pixelw*8.0, chk->h - FONTSIZE*0.4 - pixelh*8.0);
    }
    gui_color(0.0, 0.0, 0.0, 1.0);
    font_render(font_normal, chk->x + FONTSIZE*1.15*hw_ratio, chk->y + FONTSIZE*0.1, chk->text, chk->textlen, FONTSIZE);
}

static void checkbox_update(uicontrol_t* chk)
{
    chk->wpref = font_width(font_normal, chk->text, chk->textlen, FONTSIZE) + FONTSIZE*1.3;
    chk->hpref = FONTSIZE*1.3;
}

static void checkbox_dispose(uicontrol_t* chk)
{
    list_remove(checkboxes, chk, 0);
}

static int checkbox_handle_event(uicontrol_t* chk, SDL_Event* ev)
{
    checkbox_data_t* data = chk->ctldata;
    if (ev->type == SDL_MOUSEBUTTONDOWN && ev->button.button == 1) {
        uicheckbox_check(chk, data->group ? 1 : !data->checked);
        gui_focus(chk);
    } else if (ev->type == SDL_KEYDOWN && ev->key.keysym.sym == SDLK_SPACE) {
        uicheckbox_check(chk, data->group ? 1 : !data->checked);
    }
    return 0;
}

uicontrol_t* uicheckbox_new(uicontrol_t* parent, float x, float y, const char* text, int checked, unsigned int group)
{
    uicontrol_t* chk = uictl_new(parent);
    chk->render = checkbox_render;
    chk->update = checkbox_update;
    chk->dispose = checkbox_dispose;
    chk->handle_event = checkbox_handle_event;
    chk->ctldata = new(uifield_data_t);
    ((checkbox_data_t*)chk->ctldata)->checked = checked ? 1 : 0;
    ((checkbox_data_t*)chk->ctldata)->group = group;
    uictl_set_text(chk, text);
    uictl_place(chk, x, y, chk->wpref, chk->hpref);
    list_add(checkboxes, chk);
    return chk;
}

void uicheckbox_check(uicontrol_t* chk, int checked)
{
    listitem_t* litem;
    checkbox_data_t* data = chk->ctldata;
    checked = checked ? 1 : 0;
    if (data->checked != checked) {
        data->checked = checked;
        uictl_callback(chk, CB_CHANGED, NULL);
        if (checked && data->group) {
            for (litem = checkboxes->first; litem; litem=litem->next) {
                uicontrol_t* cbox = litem->ptr;
                if (cbox == chk || ((checkbox_data_t*)cbox->ctldata)->group != data->group) continue;
                if (uicheckbox_checked(cbox))
                    uicheckbox_check(cbox, 0);
            }
        }
    }
}

int uicheckbox_checked(uicontrol_t* chk)
{
    checkbox_data_t* data = chk->ctldata;
    return data->checked;
}


/* Text editor controls */
typedef struct _editline_t
{
    char* text;
    int textlen;
} editline_t;

typedef struct _editor_data_t
{
    float xcursor;
    float ycursor;
    float xscroll;
    float yscroll;
    int row, col;
    list_t* lines;
    editline_t** line;
} editor_data_t;

static void editor_update_line_array(uicontrol_t* ted)
{
    editor_data_t* data = ted->ctldata;
    int i = 0;
    listitem_t* liline;
    if (data->line) free(data->line);
    data->line = malloc(sizeof(editline_t*)*data->lines->count);
    for (liline = data->lines->first; liline; liline=liline->next) {
        data->line[i++] = liline->ptr;
    }
}

static void editor_check_cursor(uicontrol_t* ted)
{
    editor_data_t* data = ted->ctldata;
    float pos;
    if (data->row < data->lines->count) {
        if (data->line[data->row]->textlen < data->col)
            data->col = data->line[data->row]->textlen;
    } else {
        data->col = 0;
    }

    pos = ted->h - FONTSIZE*1.15 + data->yscroll - data->row*FONTSIZE;
    while (pos <= 0.0) {
        pos += FONTSIZE;
        data->yscroll += FONTSIZE;
    }

    while (pos >= ted->h - FONTSIZE) {
        pos -= FONTSIZE;
        data->yscroll -= FONTSIZE;
    }
    if (data->yscroll < 0) data->yscroll = 0;

    pos = (data->row == data->lines->count ? 0 : font_width(font_normal, data->line[data->row]->text, data->col, FONTSIZE)) - data->xscroll + FONTSIZE*0.15*hw_ratio;
    while (pos <= 0.0) {
        pos += ted->w*0.35;
        data->xscroll -= ted->w*0.35;
    }

    while (pos >= ted->w) {
        pos -= ted->w*0.35;
        data->xscroll += ted->w*0.35;
    }
    if (data->xscroll < 0) data->xscroll = 0;
}

static void editor_render_text(uicontrol_t* ted, editor_data_t* data)
{
    float x = ted->x - data->xscroll + 0.15*FONTSIZE*hw_ratio;
    float y = ted->y + ted->h - FONTSIZE + data->yscroll - 0.15*FONTSIZE;
    int linecnt = 0;
    listitem_t* liline;

    for (liline = data->lines->first; liline; liline=liline->next) {
        editline_t* line = liline->ptr;
        if (ted == focus && linecnt == data->row) {
            gui_color(0.32, 0.31, 0.33, 1.0);
            gui_bar(0, y, 10, FONTSIZE);
            gui_color(0.27, 0.26, 0.28, 1.0);
            gui_bar(ted->x, y, 0.01*hw_ratio, FONTSIZE);
            gui_color(0, 0, 0, 1);
        }
        font_render(font_normal, x, y, line->text, line->textlen, FONTSIZE);
        if (linecnt == data->row) {
            data->xcursor = ted->x + font_width(font_normal, line->text, data->col, FONTSIZE)- data->xscroll + 0.15*FONTSIZE*hw_ratio;
            data->ycursor = y;
        }
        y -= FONTSIZE;
        linecnt++;
    }
    if (linecnt == data->row) {
        if (ted == focus) {
            gui_color(0.32, 0.31, 0.33, 1.0);
            gui_bar(0, y, 10, FONTSIZE);
            gui_color(0.27, 0.26, 0.28, 1.0);
            gui_bar(ted->x, y, 0.01*hw_ratio, FONTSIZE);
            gui_color(0, 0, 0, 1);
        }
        data->xcursor = ted->x + 0.15*FONTSIZE*hw_ratio;
        data->ycursor = y;
    }
}

static void editor_render(uicontrol_t* ted)
{
    editor_data_t* data = ted->ctldata;
    if (focus == ted)
        gui_color(0.22, 0.27, 0.29, 1.0);
    else
        gui_color(0.20, 0.25, 0.27, 1.0);
    gui_bar(ted->x, ted->y, ted->w, ted->h);
    if (focus == ted)
        gui_color(0.27, 0.32, 0.35, 1.0);
    else
        gui_color(0.25, 0.3, 0.33, 1.0);
    gui_bar(ted->x + 0.01*hw_ratio, ted->y, ted->w - 0.01*hw_ratio, ted->h - 0.01);
    gui_color(0.0, 0.0, 0.0, 1.0);
    editor_render_text(ted, data);
    if (focus == ted) {
        gui_color(0.5, 0.2, 0.1, 1.0);
        gui_bar(data->xcursor, data->ycursor, 3*pixelw, FONTSIZE + 0.005);
    }
}

static void editor_dispose(uicontrol_t* ted)
{
    editor_data_t* data = ted->ctldata;
    listitem_t* liline;
    for (liline=data->lines->first; liline; liline=liline->next) {
        editline_t* line = liline->ptr;
        free(line->text);
    }
    list_free(data->lines);
    if (data->line) free(data->line);
}

static int editor_handle_event(uicontrol_t* ted, SDL_Event* ev)
{
    editor_data_t* data = ted->ctldata;
    editline_t* line;
    float mx, my;
    uictl_mouse_position(ted, &mx, &my);

    switch (ev->type) {
    case SDL_MOUSEBUTTONDOWN:
        if (ev->button.button == 1) {
            data->row = ((ted->h - my - FONTSIZE*0.15) + data->yscroll)/FONTSIZE;
            if (data->row < 0) data->row = 0;
            else if (data->row > data->lines->count) data->row = data->lines->count;
            editor_check_cursor(ted);
            gui_focus(ted);
            return 1;
        }
        return 0;
    case SDL_KEYDOWN:
        if (ev->key.keysym.sym == SDLK_DOWN) {
            if (data->row < data->lines->count) {
                data->row++;
                editor_check_cursor(ted);
            }
            return 1;
        } else if (ev->key.keysym.sym == SDLK_UP) {
            if (data->row) {
                data->row--;
                editor_check_cursor(ted);
            }
            return 1;
        } else if (ev->key.keysym.sym == SDLK_LEFT) {
            if (data->col) {
                data->col--;
                editor_check_cursor(ted);
            }
            return 1;
        } else if (ev->key.keysym.sym == SDLK_RIGHT) {
            data->col++;
            editor_check_cursor(ted);
            return 1;
        } else if (ev->key.keysym.sym == SDLK_HOME) {
            data->col = 0;
            editor_check_cursor(ted);
            return 1;
        } else if (ev->key.keysym.sym == SDLK_END) {
            if (data->row < data->lines->count) {
                data->col = data->line[data->row]->textlen;
                editor_check_cursor(ted);
            }
            return 1;
        } else if (ev->key.keysym.sym == SDLK_BACKSPACE) {
            char* newtext;
            char* tmp;

            if (data->row == 0 && data->col == 0) return 1;

            if (data->row == data->lines->count) {
                if (data->line[data->row - 1]->textlen == 0) {
                    uieditor_remove_line(ted, data->row - 1);
                } else {
                    data->col = data->line[data->row - 1]->textlen;
                    data->row--;
                    editor_check_cursor(ted);
                }
                return 1;
            }

            line = data->line[data->row];
            if (data->col == 0) { /* combine with line above */
                newtext = malloc(line->textlen + data->line[data->row - 1]->textlen + 1);
                sprintf(newtext, "%s%s", data->line[data->row - 1]->text, line->text);
                free(data->line[data->row - 1]->text);
                data->line[data->row - 1]->text = newtext;
                data->col = data->line[data->row - 1]->textlen;
                data->line[data->row - 1]->textlen = strlen(newtext);
                uieditor_remove_line(ted, data->row);
                data->row--;
                editor_check_cursor(ted);
                return 1;
            }

            newtext = malloc(line->textlen);
            tmp = strdup(line->text);
            tmp[data->col - 1] = 0;
            sprintf(newtext, "%s%s", tmp, line->text + data->col);
            free(line->text);
            line->text = newtext;
            line->textlen--;
            data->col--;
            free(tmp);
            editor_check_cursor(ted);

            return 1;
        } else if (ev->key.keysym.sym == SDLK_DELETE) {
            char* newtext;
            char* tmp;

            if (data->row == data->lines->count) return 1;

            line = data->line[data->row];
            if (data->col == line->textlen) { /* combine with line below */
                if (data->row == data->lines->count - 1) {
                    if (data->col == 0) {
                        uieditor_remove_line(ted, data->row);
                    }
                    return 1;
                }
                newtext = malloc(line->textlen + data->line[data->row + 1]->textlen + 1);
                sprintf(newtext, "%s%s", line->text, data->line[data->row + 1]->text);
                free(data->line[data->row + 1]->text);
                data->line[data->row + 1]->text = newtext;
                data->line[data->row + 1]->textlen = strlen(newtext);
                uieditor_remove_line(ted, data->row);
                editor_check_cursor(ted);
                return 1;
            }

            newtext = malloc(line->textlen);
            tmp = strdup(line->text);
            tmp[data->col] = 0;
            sprintf(newtext, "%s%s", tmp, line->text + data->col + 1);
            free(line->text);
            line->text = newtext;
            line->textlen--;
            free(tmp);
            editor_check_cursor(ted);

            return 1;
        } else if (ev->key.keysym.sym == SDLK_RETURN) {
            char* newtext;
            char* newline;
            if (data->row == data->lines->count) {
                uieditor_append(ted, "");
                data->row++;
                editor_check_cursor(ted);
                return 1;
            }

            line = data->line[data->row];
            newtext = strdup(line->text);
            newline = strdup(line->text + data->col);
            newtext[data->col] = 0;
            uieditor_insert(ted, data->row + 1, newline);
            free(newline);
            free(line->text);
            line->text = newtext;
            line->textlen = strlen(newtext);
            data->row++;
            data->col = 0;
            editor_check_cursor(ted);

            if (data->row > 0) {
                SDL_Event fake;
                int i, j;
                memset(&fake, 0, sizeof(fake));
                fake.type = SDL_KEYDOWN;
                fake.key.keysym.sym = SDLK_SPACE;
                fake.key.keysym.unicode = 32;
                j = 0;
                line = data->line[data->row - 1];
                while (j < line->textlen && line->text[j] == 32) j++;
                for (i=0; i<j; i++) editor_handle_event(ted, &fake);
            }
            return 1;
        } else if (ev->key.keysym.sym == SDLK_TAB) {
            SDL_Event fake;
            int i, j;
            memset(&fake, 0, sizeof(fake));
            fake.type = SDL_KEYDOWN;
            fake.key.keysym.sym = SDLK_SPACE;
            fake.key.keysym.unicode = 32;
            j = 4 - (data->col&3);
            for (i=0; i<j; i++) editor_handle_event(ted, &fake);
            return 1;
        } else if (ev->key.keysym.sym == SDLK_ESCAPE) {
            gui_focus(NULL);
            return 1;
        } else if (!(ev->key.keysym.unicode & 0xFF80) && ev->key.keysym.unicode > 31) {
            char* newtext;
            char* tmp;
            if (data->row >= data->lines->count) uieditor_append(ted, "");
            line = data->line[data->row];
            newtext = malloc(line->textlen + 2);
            tmp = strdup(line->text);
            tmp[data->col] = 0;
            sprintf(newtext, "%s%c%s", tmp, ev->key.keysym.unicode, line->text + data->col);
            free(tmp);
            free(line->text);
            line->text = newtext;
            line->textlen++;
            data->col++;
            editor_check_cursor(ted);
            uictl_callback(ted, CB_CHANGED, NULL);
            return 1;
        }
        return 0;
    }
    return 0;
}

uicontrol_t* uieditor_new(uicontrol_t* parent, float x, float y, float w, float h)
{
    uicontrol_t* ted = uictl_new(parent);
    ted->render = editor_render;
    ted->dispose = editor_dispose;
    ted->handle_event = editor_handle_event;
    ted->ctldata = new(editor_data_t);
    ((editor_data_t*)ted->ctldata)->lines = list_new();
    uictl_place(ted, x, y, w, h);

    return ted;
}

void uieditor_append(uicontrol_t* ted, const char* text)
{
    editor_data_t* data = ted->ctldata;
    editline_t* line = new(editline_t);
    line->text = strdup(text);
    line->textlen = strlen(text);
    list_add(data->lines, line);
    editor_update_line_array(ted);
}

void uieditor_insert(uicontrol_t* ted, int index, const char* text)
{
    editor_data_t* data = ted->ctldata;
    editline_t* line = new(editline_t);
    line->text = strdup(text);
    line->textlen = strlen(text);
    if (index <= 0)
        list_insert(data->lines, NULL, line);
    else if (index >= data->lines->count)
        list_add(data->lines, line);
    else
        list_insert(data->lines, data->line[index - 1], line);
    editor_update_line_array(ted);
    if (index <= data->row) {
        data->row++;
        editor_check_cursor(ted);
    }
}

char* uieditor_get_text(uicontrol_t* ted)
{
    char* total = strdup("");
    size_t tlen = 0, i, cnt = uieditor_line_count(ted);
    for (i=0; i<cnt; i++) {
        const char* line = uieditor_get_line(ted, i);
        total = realloc(total, tlen + strlen(line) + 2);
        strcat(total, line);
        strcat(total, "\n");
        tlen = strlen(total);
    }
    return total;
}

int uieditor_line_count(uicontrol_t* ted)
{
    editor_data_t* data = ted->ctldata;
    return data->lines->count;
}

const char* uieditor_get_line(uicontrol_t* ted, unsigned int index)
{
    editor_data_t* data = ted->ctldata;
    if (index >= 0 && index < data->lines->count) {
        return data->line[index]->text;
    } else if (index == data->lines->count) {
        return "";
    } else return NULL;
}

void uieditor_set_line(uicontrol_t* ted, unsigned int index, const char* text)
{
    editor_data_t* data = ted->ctldata;
    if (index >= 0 && index < data->lines->count) {
        free(data->line[index]->text);
        data->line[index]->text = strdup(text);
        data->line[index]->textlen = strlen(text);
    } else if (index == data->lines->count) {
        uieditor_append(ted, text);
    }
}

void uieditor_remove_line(uicontrol_t* ted, unsigned int index)
{
    editor_data_t* data = ted->ctldata;
    if (index >= 0 && index < data->lines->count) {
        free(data->line[index]->text);
        list_remove(data->lines, data->line[index], 1);
        editor_update_line_array(ted);
        if (data->row > index) data->row--;
    }
}
