/*
 * Alithia Engine Packager
 * Copyright (C) 2010 Kostas Michalopoulos
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <dirent.h>

#define VERSION "0.1"

typedef struct _file_t
{
    char* name;
    uint32_t pos;
    uint32_t size;
} file_t;

const char* dirname = "data";
const char* pakname = "alithia.alp";
file_t* file;
size_t files;
FILE* o;

void add(const char* herefile, const char* filename)
{
    FILE* f = fopen(herefile, "rb");
    char* buffer;
    size_t size;
    if (!f) {
        printf("alpak: warning: failed to open file '%s'\n", filename);
        return;
    }

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);

    buffer = malloc(size);
    fread(buffer, 1, size, f);
    fclose(f);

    file = realloc(file, sizeof(file_t)*(files + 1));
    file[files].name = strdup(filename);
    file[files].pos = ftell(o);
    file[files].size = (uint32_t)size;
    files++;

    fwrite(buffer, 1, size, o);
    free(buffer);
}

void scan(const char* prefix)
{
    DIR* dir;
    struct dirent* de;
    char* tmp;

    dir = opendir(".");
    if (!dir) {
        printf("alpak: warning: failed to open directory '%s'\n", prefix);
        return;
    }

    while ((de = readdir(dir))) {
        if (de->d_name[0] == '.') continue;
        if (!chdir(de->d_name)) {
            printf("alpak: entering %s/%s\n", prefix, de->d_name);
            tmp = malloc(strlen(prefix) + strlen(de->d_name) + 2);
            sprintf(tmp, "%s/%s", prefix, de->d_name);
            scan(tmp);
            printf("alpak: leaving %s/%s\n", prefix, de->d_name);
            free(tmp);
            chdir("..");
        } else {
            printf("alpak: adding %s/%s\n", prefix, de->d_name);
            tmp = malloc(strlen(prefix) + strlen(de->d_name) + 2);
            sprintf(tmp, "%s/%s", prefix, de->d_name);
            add(de->d_name, tmp);
            free(tmp);
        }
    }

    closedir(dir);
}

void makepak(void)
{
    const char* id = "ALP1";
    uint32_t pos = 0;
    size_t i;
    o = fopen(pakname, "wb");
    if (!o) {
        printf("alpak: failed to create '%s'\n", pakname);
        exit(1);
    }
    if (chdir(dirname)) {
        fclose(o);
        printf("alpak: failed to change to '%s'\n", dirname);
        remove(pakname);
        exit(1);
    }
    fwrite(id, 1, 4, o);
    fwrite(&pos, 1, 4, o);
    scan("");
    pos = ftell(o);
    for (i=0; i<files; i++) {
        uint16_t len = strlen(file[i].name);
        fwrite(&len, 1, 2, o);
        fwrite(file[i].name, 1, len, o);
        fwrite(&file[i].pos, 1, 4, o);
        fwrite(&file[i].size, 1, 4, o);
    }
    fseek(o, 4, SEEK_SET);
    fwrite(&pos, 1, 4, o);
    fseek(o, 0, SEEK_END);
    printf("alpak: done. File size=%i KiB\n", (int)(ftell(o)/1024));
    fclose(o);
}

int main(int argc, char** argv)
{
    size_t i;
    for (i=1; i<argc; i++) {
        if (argv[i][0] == '-') {
            if (!strcmp(argv[i], "--help")) {
                printf("Alithia Engine Packager version "VERSION" (C) 2010 Kostas Michalopoulos\n");
                printf("Usage:  alpak [options] [archive]\n");
                printf("Options:\n");
                printf("  -d or --dir <name>  Get data files from given dir. Default=data.\n");
                printf("If <archive> is not given, alithia.alp is used.\n");
                return 0;
            } else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--dir")) {
                dirname = argv[++i];
            } else {
                printf("alpak: unknown option '%s'\n", argv[i]);
            }
        } else pakname = argv[i];
    }

    makepak();

    return 0;
}
