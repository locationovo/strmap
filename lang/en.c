#include "lang.h"
static const LangDict dict_en = {
    .help_text = C_CYAN "strmap" C_RESET " - Binary String & UI Extractor\n\n" 
                 C_YELLOW "Usage:" C_RESET "\n  strmap [options] <App>\n\n" 
                 C_YELLOW "Options:" C_RESET "\n"
                 "  " C_GREEN "-h, --help" C_RESET "              Show help\n"
                 "  " C_GREEN "-v, --version" C_RESET "           Show version\n"
                 "  " C_GREEN "-l, --lang <en|zh>" C_RESET "      UI language\n"
                 "  " C_GREEN "-n, --min-len <len>" C_RESET "     Minimum string length (default: 4)\n"
                 "  " C_GREEN "-o, --out <dir>" C_RESET "         Output root directory\n",
    .err_no_args = C_RED "Error: No arguments." C_RESET,
    .err_usage = C_RED "Error: Invalid args." C_RESET "\nUse -h for help.",
    .err_not_found = C_RED "Error: App not found:" C_RESET,
    .err_open = C_RED "Error: Cannot open:" C_RESET,
    .err_write = C_RED "Error: Write failed:" C_RESET,
    .analyze_done = C_GREEN "Done:" C_RESET,
    .output_dir = C_BLUE "Output:" C_RESET,
    .stats = "Stats: Classes %d | Ivars %d | UI %d",
    .generated_files = C_YELLOW "Files:" C_RESET,
    .col_orig = "Original", .col_cat = "Category", .col_src = "Source",
    .cat_alert = "Alert", .cat_err = "Error", .cat_warn = "Warning", .cat_desc = "Description", .cat_btn = "Button", .cat_set = "Settings", .cat_det = "Details", .cat_other = "Other"
};
__attribute__((constructor)) static void reg_en(void) { register_lang("en", &dict_en); }
