/* 命令行解析模块 — 步骤 1 骨架（空函数，链接用） */

#ifndef PARSE_CMDLINE_H
#define PARSE_CMDLINE_H

#include <windows.h>
#include <stdbool.h>

/*
 * ParseCommandLine
 * 从 WinMain 的 lpCmdLine 提取拖放文件路径
 * 参数：
 *   lpCmdLine  — WinMain 传入的命令行参数
 *   outPath    — 输出缓冲区（WCHAR[MAX_PATH]）
 * 返回：
 *   TRUE  成功，outPath 包含有效文件路径
 *   FALSE 失败（无参数或路径无效）
 */
BOOL ParseCommandLine(LPCWSTR lpCmdLine, WCHAR *outPath);

#endif /* PARSE_CMDLINE_H */
