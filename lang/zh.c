#include "lang.h"
static const LangDict dict_zh = {
    .help_text = C_CYAN "strmap" C_RESET " - 二进制字符串与 UI 提取工具\n\n" 
                 C_YELLOW "用法:" C_RESET "\n  strmap [选项] <App>\n\n" 
                 C_YELLOW "选项:" C_RESET "\n"
                 "  " C_GREEN "-h, --help" C_RESET "              显示帮助信息\n"
                 "  " C_GREEN "-v, --version" C_RESET "           显示版本信息\n"
                 "  " C_GREEN "-l, --lang <en|zh>" C_RESET "      指定界面语言\n"
                 "  " C_GREEN "-n, --min-len <len>" C_RESET "     指定最小字符串长度 (默认: 4)\n"
                 "  " C_GREEN "-o, --out <dir>" C_RESET "         指定输出根目录\n",
    .err_no_args = C_RED "错误: 无参数。" C_RESET,
    .err_usage = C_RED "错误: 参数无效。" C_RESET "\n使用 -h 查看帮助。",
    .err_not_found = C_RED "错误: 找不到 App:" C_RESET,
    .err_open = C_RED "错误: 无法打开:" C_RESET,
    .err_write = C_RED "错误: 写入失败:" C_RESET,
    .analyze_done = C_GREEN "分析完成:" C_RESET,
    .output_dir = C_BLUE "输出目录:" C_RESET,
    .stats = "解析统计: 类 %d | 属性 %d | UI %d",
    .generated_files = C_YELLOW "生成文件:" C_RESET,
    .col_orig = "原文", .col_cat = "类别", .col_src = "来源",
    .cat_alert = "弹窗", .cat_err = "错误", .cat_warn = "警告", .cat_desc = "说明", .cat_btn = "按钮", .cat_set = "设置", .cat_det = "详情", .cat_other = "其他"
};
__attribute__((constructor)) static void reg_zh(void) { register_lang("zh", &dict_zh); }
