#ifndef LANG_H
#define LANG_H
#define C_RED     "\033[1;31m"
#define C_GREEN   "\033[1;32m"
#define C_YELLOW  "\033[1;33m"
#define C_BLUE    "\033[1;34m"
#define C_CYAN    "\033[1;36m"
#define C_RESET   "\033[0m"
typedef struct {
    const char *help_text, *err_no_args, *err_usage, *err_not_found, *err_open, *err_write;
    const char *analyze_done, *output_dir, *stats, *generated_files;
    const char *col_orig, *col_cat, *col_src, *cat_alert, *cat_err, *cat_warn, *cat_desc, *cat_btn, *cat_set, *cat_det, *cat_other;
} LangDict;
extern const LangDict *L;
void register_lang(const char *code, const LangDict *dict);
void init_lang(const char *lang_code);
#endif
