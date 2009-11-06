#include "atest.h"

const char* arg_value(const char* name, const char* defvalue)
{
    int i;
    for (i=0; i<argc; i++) {
        if (!strcmp(argv[i], name)) {
            if (i < argc - 1)
                return argv[i + 1];
            return "";
        }
    }
    return "";
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
