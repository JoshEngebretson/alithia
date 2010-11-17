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

#include <unistd.h>
#ifdef __GNUC__
#include <dirent.h>
#include <sys/stat.h>
#endif
#include "atest.h"

static list_t* dirs;

/* simple function that converts \s to /s and adds a / at the end if missing */
static char* unipath(const char* path)
{
    char* newpath = malloc(strlen(path) + 2);
    size_t i;
    for (i=0; path[i]; i++)
        if (path[i] == '\\')
            newpath[i] = '/';
        else
            newpath[i] = path[i];
    newpath[i] = 0;
    if (newpath[strlen(newpath) - 1] != '/')
        strcat(newpath, "/");
    return newpath;
}

#if defined(__MACOSX__)
const char* getApplicationDirectory(void); /* defined in SDLMain.m */

static char* rio_system_datapath(void)
{
    char* path = NULL;
    char* binpath = unipath(getApplicationDirectory());
    path = malloc(strlen(binpath) + 256);
    sprintf(path, "%s/Contents/Resources", binpath);
    free(binpath);
    return path;
}
#else
static char* rio_system_datapath(void)
{
    return strdup("data");
}
#endif

int rio_init(void)
{
    char* datapath = rio_system_datapath();
    const char* gamepath = arg_value("-game", datapath);
    char* tmp = malloc(strlen(gamepath) + 256);
    size_t i;
    dirs = list_new();
    dirs->item_free = free;

    rio_mountdir(gamepath);
    for (i=0; i<10; i++) {
        sprintf(tmp, "%s/alp%i.alp", gamepath, i);
        rio_mountalp(tmp);
    }

    free(tmp);
    free(datapath);
    return 0;
}

void rio_shutdown(void)
{
    list_free(dirs);
}

void rio_mountdir(const char* path)
{
    list_add(dirs, unipath(path));
}

void rio_mountalp(const char* path)
{
}

static void rio_files_add(list_t* files, const char* file)
{
    listitem_t* it;
    for (it=files->first; it; it=it->next)
        if (!strcmp(file, it->ptr)) return;
    list_add(files, strdup(file));
}

static void rio_files_scandir(list_t* files, const char* path)
{
    #ifdef __GNUC__
    DIR* dir;
    struct dirent* dp;

    if (!(dir = opendir(path))) return;

    while ((dp = readdir(dir)))
        if (dp->d_name[0] != '.')
            rio_files_add(files, dp->d_name);

    closedir(dir);
    #endif

    #ifdef __WATCOMC__
    HANDLE find;
    WIN32_FIND_DATA fd;
    char* tmp = malloc(strlen(path) + 3);
    int i;
    sprintf(tmp, "%s\\*", path);
    for (i=0; tmp[i]; i++)
        if (tmp[i] == '/') tmp[i] = '\\';

    if ((find = FindFirstFile(tmp, &fd)) != INVALID_HANDLE_VALUE) {
        do {
            rio_files_add(files, fd.cFileName);
        } while (FindNextFile(find, &fd));
        FindClose(find);
    }

    free(tmp);
    #endif
}

list_t* rio_files(const char* path)
{
    list_t* files = list_new();
    char* upath = unipath(path);
    listitem_t* it;

    files->item_free = free;

    for (it=dirs->first; it; it=it->next) {
        const char* dir = it->ptr;
        char* tmpath = malloc(strlen(upath) + strlen(dir) + 1);
        sprintf(tmpath, "%s%s", dir, upath);
        rio_files_scandir(files, tmpath);
        free(tmpath);
    }

    free(upath);

    return files;
}

#ifdef __MACOSX__
const char* getApplicationWriteDirectory(const char* appname);
static char* rio_system_write_path(void)
{
    return unipath(getApplicationWriteDirectory(arg_value("-appname", GAME_TITLE)));
}
#else
static char* rio_system_write_path(void)
{
    return strdup("./");
}
#endif

static char* rio_write_path(void)
{
    const char* writepath = arg_value("-writepath", NULL);
    const char* writename = arg_value("-writename", NULL);
    char* awdir = writepath ? strdup(writepath) : rio_system_write_path();
    char* fdir;
    char* dir;
    size_t i;

    if (writepath) {
        fdir = unipath(writepath);
        free(awdir);
        return fdir;
    }

    if (writename) {
        fdir = strdup(writename);
    } else {
        if (arg_value("-game", NULL)) {
            fdir = strdup(dirs->first->ptr);
        } else {
            fdir = strdup("");
        }
    }

    dir = malloc(strlen(awdir) + strlen(fdir) + 256);
    sprintf(dir, "%s%s", awdir, fdir);

    free(fdir);
    free(awdir);
    fdir = unipath(dir);
    free(dir);
    return fdir;
}

static int rio_force_dirs(const char* path)
{
    char part[4096];
    char cwd[4096];
    size_t i, len = 0;
    #ifdef __WATCOMC__
    GetCurrentDirectory(sizeof(cwd), cwd);
    #else
    getcwd(cwd, sizeof(cwd));
    #endif
    if (path[0] == '/') chdir("/");
    for (i=0; path[i]; i++) {
        if (path[i] == '/') {
            if (len) {
                part[len] = 0;
#ifdef WIN32
                if (len == 2 && part[1] == ':') {
                    strcat(part, "/");
                    if (chdir(part) != 0) {
                        chdir(cwd);
                        return 0;
                    }
                    len = 0;
                    continue;
                }
#endif
                if (chdir(part) != 0) {
#ifdef WIN32
                    mkdir(part);
#else
                    mkdir(part, 0777);
#endif
                    if (chdir(part) != 0) {
                        chdir(cwd);
                        return 0;
                    }
                }
            }
            len = 0;
        } else part[len++] = path[i];
    }
    chdir(cwd);
    return 1;
}

FILE* rio_create(const char* name)
{
    char* wpath = rio_write_path();
    char* fpath = malloc(strlen(wpath) + strlen(name) + 1);
    FILE* f = NULL;
    sprintf(fpath, "%s%s", wpath, name);
    free(wpath);
    if (rio_force_dirs(fpath)) {
        f = fopen(fpath, "wb");
    }
    free(fpath);
    return f;
}

FILE* rio_open(const char* name, size_t* size)
{
    listitem_t* it;
    for (it = dirs->last; it; it=it->prev) {
        const char* dir = it->ptr;
        char* tmp = malloc(strlen(name) + strlen(dir) + 1);
        FILE* f;
        sprintf(tmp, "%s%s", dir, name);
        f = fopen(tmp, "rb");
        free(tmp);
        if (f) {
            if (size) {
                fseek(f, 0, SEEK_END);
                *size = ftell(f);
                fseek(f, 0, SEEK_SET);
            }
            return f;
        }
    }
    return NULL;
}

char* rio_read(const char *name, size_t *size)
{
    size_t rsize;
    FILE* f = rio_open(name, &rsize);
    char* buff;
    if (!f) return NULL;
    buff = malloc(rsize + 1);
    if (!buff) return NULL;
    if (fread(buff, 1, rsize, f) != rsize) {
        free(buff);
        return NULL;
    }
    buff[rsize] = 0;
    if (size) *size = rsize;
    return buff;
}
