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

const char* arg_value(const char* name, const char* defvalue)
{
    const char* r;
    char* varname;
    lil_value_t varval;
    int i;
    if (lil) {
        varname = malloc(strlen(name) + 16);
        sprintf(varname, "engine:override%s", name);
        varval = lil_get_var_or(lil, varname, NULL);
        free(varname);
        if (varval) return lil_to_string(varval);
    }
    for (i=0; i<argc; i++) {
        if (!strcmp(argv[i], name)) {
            if (i < argc - 1)
                return argv[i + 1];
            return "";
        }
    }
    if (!lil) return defvalue;
    varname = malloc(strlen(name) + 15);
    sprintf(varname, "engine:default%s", name);
    varval = lil_get_var_or(lil, varname, NULL);
    free(varname);
    r = varval ? lil_to_string(varval) : defvalue;
    return r;
}

int arg_intval(const char* name, int defvalue)
{
    const char* value = arg_value(name, "");
    int i;
    if (!value[0]) return defvalue;
    for (i=0; value[i]; i++)
        if (!isdigit(value[i])) return defvalue;
    return atoi(value);
}
