#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdint.h>
#include <time.h>
#include <getopt.h>
#include "lang.h"
#include "version.h"

#ifndef APP_VERSION
#endif
int g_min_len = 4;
typedef struct { char **items; size_t count; size_t cap; } StrList;
void list_init(StrList *l) { l->items = NULL; l->count = 0; l->cap = 0; }
void list_add(StrList *l, const char *s) {
    if (!s) return;
    for (size_t i = 0; i < l->count; i++) if (strcmp(l->items[i], s) == 0) return;
    if (l->count == l->cap) {
        size_t new_cap = l->cap ? l->cap * 2 : 64;
        char **new_items = realloc(l->items, new_cap * sizeof(char*));
        if (!new_items) return;
        l->items = new_items; l->cap = new_cap;
    }
    char *dup = strdup(s); if (dup) l->items[l->count++] = dup;
}
void list_free(StrList *l) {
    if (!l->items) return;
    for (size_t i = 0; i < l->count; i++) free(l->items[i]);
    free(l->items); l->items = NULL; l->count = 0; l->cap = 0;
}
void mkdir_p(const char *path) {
    char tmp[1024]; char *p = NULL; size_t len;
    snprintf(tmp, sizeof(tmp), "%s", path); len = strlen(tmp);
    if (len > 0 && tmp[len - 1] == '/') tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++) { if (*p == '/') { *p = 0; mkdir(tmp, 0755); *p = '/'; } }
    mkdir(tmp, 0755);
}
int find_app(const char *name, char *out_exe, size_t max_len) {
    struct stat st; if (stat(name, &st) == 0 && S_ISREG(st.st_mode)) { strncpy(out_exe, name, max_len - 1); out_exe[max_len - 1] = '\0'; return 1; }
    char app_name[256]; if (strstr(name, ".app")) { strncpy(app_name, name, sizeof(app_name) - 1); app_name[sizeof(app_name) - 1] = '\0'; } else snprintf(app_name, sizeof(app_name), "%s.app", name);
    char app_path[1024] = {0};
    if (stat(name, &st) == 0 && S_ISDIR(st.st_mode)) { strncpy(app_path, name, sizeof(app_path) - 1); } else {
        const char *base_dirs[] = { "/Applications", "/var/containers/Bundle/Application", "/rootfs/private/var/containers/Bundle/Application", "/var/jb/Applications", NULL };
        for (int i = 0; base_dirs[i] && !app_path[0]; i++) {
            DIR *d = opendir(base_dirs[i]); if (!d) continue; struct dirent *ent;
            while ((ent = readdir(d))) {
                if (ent->d_name[0] == '.') continue; char tmp[1024];
                snprintf(tmp, sizeof(tmp), "%s/%s", base_dirs[i], app_name);
                if (strcmp(ent->d_name, app_name) == 0 && stat(tmp, &st) == 0 && S_ISDIR(st.st_mode)) { strncpy(app_path, tmp, sizeof(app_path) - 1); break; }
                snprintf(tmp, sizeof(tmp), "%s/%s/%s", base_dirs[i], ent->d_name, app_name);
                if (stat(tmp, &st) == 0 && S_ISDIR(st.st_mode)) { strncpy(app_path, tmp, sizeof(app_path) - 1); break; }
            }
            closedir(d);
        }
        if (!app_path[0]) return 0;
    }
    char plist_path[1024]; snprintf(plist_path, sizeof(plist_path), "%s/Info.plist", app_path);
    char exe_name[256] = {0}; FILE *pf = fopen(plist_path, "rb");
    if (pf) {
        fseek(pf, 0, SEEK_END); long psz = ftell(pf);
        if (psz > 0) {
            fseek(pf, 0, SEEK_SET); char *pbuf = malloc(psz + 1);
            if (pbuf) {
                size_t read_bytes = fread(pbuf, 1, psz, pf); pbuf[read_bytes] = '\0';
                char *p = strstr(pbuf, "<key>CFBundleExecutable</key>");
                if (p) { p = strstr(p, "<string>"); if (p) { p += 8; char *end = strstr(p, "</string>"); if (end) { size_t len = end - p; if (len < sizeof(exe_name)) { strncpy(exe_name, p, len); exe_name[len] = '\0'; } } } }
                free(pbuf);
            }
        }
        fclose(pf);
    }
    if (strlen(exe_name) == 0) { char base[256]; const char *slash = strrchr(app_path, '/'); strncpy(base, slash ? slash + 1 : app_path, sizeof(base) - 1); char *dot = strstr(base, ".app"); if (dot) *dot = '\0'; strncpy(exe_name, base, sizeof(exe_name) - 1); }
    snprintf(out_exe, max_len, "%s/%s", app_path, exe_name);
    if (stat(out_exe, &st) == 0 && S_ISREG(st.st_mode)) return 1;
    return 0;
}
void extract_and_process(const char *exe, const char *outdir) {
    FILE *f = fopen(exe, "rb"); if (!f) { printf("%s %s\n", L->err_open, exe); return; }
    fseek(f, 0, SEEK_END); long sz = ftell(f); if (sz <= 0) { fclose(f); return; } fseek(f, 0, SEEK_SET);
    unsigned char *buf = malloc(sz); if (!buf) { fclose(f); return; }
    size_t read_bytes = fread(buf, 1, sz, f); fclose(f); if (read_bytes == 0) { free(buf); return; }
    StrList strs = {0}; list_init(&strs);
    char *cur = malloc(read_bytes + 1); size_t cur_len = 0;
    for (size_t i = 0; i < read_bytes; i++) {
        unsigned char c = buf[i];
        if (c >= 32 && c <= 126) { if (cur_len < read_bytes) cur[cur_len++] = c; } else {
            if ((int)cur_len >= g_min_len) { cur[cur_len] = '\0'; list_add(&strs, cur); }
            cur_len = 0;
        }
    }
    char path[1024];
    snprintf(path, sizeof(path), "%s/strings.txt", outdir);
    f = fopen(path, "w"); if (f) { for(size_t i=0; i<strs.count; i++) fprintf(f, "%s\n", strs.items[i]); fclose(f); }
    printf(C_GREEN "%s" C_RESET " %s\n", L->analyze_done, outdir);
    list_free(&strs); free(cur); free(buf);
}
int main(int argc, char *argv[]) {
    if (argc <= 1) { init_lang(NULL); printf("%s\n", L->err_no_args); return 1; }
    char *target = NULL; char *out_root = "./strmap_out"; char *lang_code = NULL;
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'}, {"version", no_argument, 0, 'v'},
        {"lang", required_argument, 0, 'l'}, {"out", required_argument, 0, 'o'},
        {"min-len", required_argument, 0, 'n'}, {0, 0, 0, 0}
    };
    int opt;
    while ((opt = getopt_long(argc, argv, "hvl:o:n:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h': init_lang(NULL); printf("%s", L->help_text); return 0;
            case 'v': printf("strmap %s\n", APP_VERSION); return 0;
            case 'l': lang_code = optarg; break;
            case 'o': out_root = optarg; break;
            case 'n': g_min_len = atoi(optarg); if (g_min_len < 1) g_min_len = 1; break;
            case '?': default: init_lang(lang_code); printf("%s\n", L->err_usage); return 1;
        }
    }
    if (optind < argc) target = argv[optind];
    init_lang(lang_code);
    if (!target) { printf("%s\n", L->err_usage); return 1; }
    char exe[1024]; if (!find_app(target, exe, sizeof(exe))) { printf("%s %s\n", L->err_not_found, target); return 1; }
    mkdir_p(out_root); extract_and_process(exe, out_root); return 0;
}
