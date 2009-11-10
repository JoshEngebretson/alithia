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

#define FONTSIZE 0.05

static uicontrol_t* capture = NULL;
static uicontrol_t* focus = NULL;
static float save_x;
static float save_y;
uicontrol_t* uiroot = NULL;

/* Helper rendering functions */
static void color(float r, float g, float b, float a)
{
    if (a < 1.0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }
    glColor4f(r, g, b, a);
}

static void bar(float x, float y, float w, float h)
{
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x, y + h);
    glVertex2f(x + w, y + h);
    glVertex2f(x + w, y);
    glEnd();
}

/* GUI funcs */
void gui_init(void)
{
    uicontrol_t* win;
    uicontrol_t* lbl;
    uicontrol_t* btn;
    uicontrol_t* fld;

    uiroot = uictl_new(NULL);
    uictl_place(uiroot, -1, -1, 2, 2);

    win = uiwin_new(0.4, 1.0, 0.5*hw_ratio, 0.35, "A window");
    lbl = uilabel_new(win, 0, 0, "Hello, world");
    btn = uibutton_new(win, FONTSIZE*0.15, lbl->hpref*1.5, "Push me!");
    fld = uifield_new(win, FONTSIZE*0.15, lbl->hpref*3.5, btn->wpref);

    win = uiwin_new(0.8, 0.7, 1.5*hw_ratio, 0.75, "Another window");
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

    uictl_render(uiroot);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glPopClientAttrib();
    glPopAttrib();
}

void gui_shutdown(void)
{
    uictl_free(uiroot);
}

void gui_handle_event(SDL_Event* ev)
{
    uicontrol_t* target = capture;
    if (!target && (ev->type == SDL_KEYDOWN || ev->type == SDL_KEYUP) && focus) target = focus;
    if (!target) target = uictl_child_at(uiroot, mouse_x + 1.0, mouse_y + 1.0);
    while (target) {
        if (target->handle_event && target->handle_event(target, ev)) break;
        target = target->parent;
    }
}

void gui_capture(uicontrol_t* ctl)
{
    capture = ctl;
}

void gui_focus(uicontrol_t* ctl)
{
    focus = ctl;
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
    if (ctl->dispose) ctl->dispose(ctl);
    if (ctl->text) return;
    if (ctl->children) list_free(ctl->children);
    free(ctl);
}

void uictl_render(uicontrol_t* ctl)
{
    if (ctl->render) {
        ctl->render(ctl);
    } else {
        uictl_render_children(ctl);
    }
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
            uictl_render(itchild->ptr);
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
    ctl->x = x;
    ctl->y = y;
    ctl->w = w;
    ctl->h = h;
    uictl_layout(ctl);
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

/* Top-level windows */
#define WINF_MOVING 0x0001

static void win_render(uicontrol_t* win)
{
    color(0.0, 0.0, 0.0, 0.5);
    bar(win->x + 0.03*hw_ratio, win->y - 0.03, win->w, win->h);
    color(0.3, 0.35, 0.37, 1.0);
    bar(win->x, win->y, win->w, win->h - FONTSIZE*1.3);
    color(0.4, 0.45, 0.47, 1.0);
    bar(win->x, win->y + win->h - FONTSIZE*1.3, win->w, FONTSIZE*1.3);
    color(0.0, 0.0, 0.0, 1.0);
    font_render(font_normal, win->x + FONTSIZE*0.15, win->y + win->h - FONTSIZE*1.15, win->text, win->textlen, FONTSIZE);

    uictl_render_children(win);
}

static int win_handle_event(uicontrol_t* win, SDL_Event* ev)
{
    float mx, my;
    uictl_mouse_position(win, &mx, &my);

    switch (ev->type) {
    case SDL_MOUSEBUTTONDOWN:
        if (ev->button.button == 1 && my > win->h - FONTSIZE*1.3) {
            win->ctlflags |= WINF_MOVING;
            save_x = mouse_x;
            save_y = mouse_y;
            gui_capture(win);
            uiwin_raise(win);
            return 1;
        }
        return 0;
    case SDL_MOUSEBUTTONUP:
        if (win->ctlflags & WINF_MOVING) {
            win->ctlflags &= ~WINF_MOVING;
            gui_capture(NULL);
            return 1;
        }
        return 0;
    case SDL_MOUSEMOTION:
        if (win->ctlflags & WINF_MOVING) {
            win->x -= save_x - mouse_x;
            win->y -= save_y - mouse_y;
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


/* Label controls */
static void label_render(uicontrol_t* lbl)
{
    color(0.0, 0.0, 0.0, 1.0);
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
        color(0.4, 0.45, 0.47, 1.0);
        bar(btn->x + 0.01*hw_ratio, btn->y - 0.01, btn->w, btn->h);
        color(0.0, 0.0, 0.0, 1.0);
        font_render(font_normal, btn->x + FONTSIZE*0.15 + 0.01*hw_ratio, btn->y + FONTSIZE*0.1 - 0.01, btn->text, btn->textlen, FONTSIZE);
    } else {
        color(0.0, 0.0, 0.0, 0.5);
        bar(btn->x + 0.02*hw_ratio, btn->y - 0.02, btn->w, btn->h);
        color(0.4, 0.45, 0.47, 1.0);
        bar(btn->x, btn->y, btn->w, btn->h);
        color(0.0, 0.0, 0.0, 1.0);
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
static void field_render(uicontrol_t* fld)
{
    if (focus == fld)
        color(0.22, 0.27, 0.29, 1.0);
    else
        color(0.20, 0.25, 0.27, 1.0);
    bar(fld->x, fld->y, fld->w, fld->h);
    if (focus == fld)
        color(0.27, 0.32, 0.35, 1.0);
    else
        color(0.25, 0.3, 0.33, 1.0);
    bar(fld->x + 0.01*hw_ratio, fld->y, fld->w - 0.01*hw_ratio, fld->h - 0.01);
    color(0.0, 0.0, 0.0, 1.0);
    font_render(font_normal, fld->x + FONTSIZE*0.15, fld->y + FONTSIZE*0.15, fld->text, fld->textlen, FONTSIZE);
    if (focus == fld) {
        float w = font_width(font_normal, fld->text, fld->textlen, FONTSIZE);
        color(0.5, 0.2, 0.1, 1.0);
        bar(fld->x + FONTSIZE*0.15 + w, fld->y + 0.005, 0.005, fld->h - 0.01);
    }
}

static int field_handle_event(uicontrol_t* fld, SDL_Event* ev)
{
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
        if (ev->key.keysym.sym == SDLK_BACKSPACE) {
            if (fld->textlen) {
                char* newtext = strdup(fld->text);
                newtext[strlen(newtext) - 1] = 0;
                uictl_set_text(fld, newtext);
                free(newtext);
                uictl_callback(fld, CB_CHANGED, NULL);
            }
            return 1;
        } else if (!(ev->key.keysym.unicode & 0xFF80)) {
            char* newtext = malloc(fld->textlen + 2);
            sprintf(newtext, "%s%c", uictl_get_text(fld), ev->key.keysym.unicode);
            uictl_set_text(fld, newtext);
            free(newtext);
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
    uictl_set_text(fld, "field");
    uictl_place(fld, x, y, w, FONTSIZE*1.3);
    return fld;
}
