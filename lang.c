#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lang.h"
#define MAX_LANGS 10
static const LangDict *dicts[MAX_LANGS];
static const char *codes[MAX_LANGS];
static int dict_count = 0;
const LangDict *L = NULL;
void register_lang(const char *code, const LangDict *dict) {
    if (dict_count < MAX_LANGS) { codes[dict_count] = code; dicts[dict_count] = dict; dict_count++; }
}
void init_lang(const char *lang_code) {
    const char *target = "en";
    if (lang_code) { target = lang_code; } else {
        char *env = getenv("LANG"); if (!env) env = getenv("LC_ALL");
        if (env) { if (strstr(env, "zh")) target = "zh"; else if (strstr(env, "en")) target = "en"; }
    }
    for (int i = 0; i < dict_count; i++) { if (strncmp(codes[i], target, 2) == 0) { L = dicts[i]; return; } }
    for (int i = 0; i < dict_count; i++) { if (strcmp(codes[i], "en") == 0) { L = dicts[i]; return; } }
    if (dict_count > 0) L = dicts[0];
}
