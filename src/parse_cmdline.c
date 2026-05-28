/* 命令行解析模块（步骤 2 实现） */

#include "parse_cmdline.h"
#include <shlwapi.h>   /* PathFileExistsW */
/* shlwapi.lib linked via LDFLAGS in Makefile */

BOOL
ParseCommandLine(LPCWSTR lpCmdLine, WCHAR *outPath)
{
    LPCWSTR p;

    if (lpCmdLine == NULL || lpCmdLine[0] == L'\0')
        return FALSE;

    /* 跳过前导空格 */
    p = lpCmdLine;
    while (*p == L' ' || *p == L'\t')
        p++;

    if (*p == L'\0')
        return FALSE;

    /* 处理引号包裹的路径 */
    if (*p == L'"') {
        p++; /* 跳过开头引号 */
        LPCWSTR end = wcschr(p, L'"');
        if (end == NULL)
            return FALSE; /* 引号未闭合 */
        /* 计算路径长度（不含引号） */
        size_t len = (size_t)(end - p);
        if (len >= MAX_PATH)
            return FALSE;
        wcsncpy(outPath, p, len);
        outPath[len] = L'\0';
    } else {
        /* 无引号，直接读到空格或结尾 */
        size_t i = 0;
        while (*p != L'\0' && *p != L' ' && *p != L'\t' && i < MAX_PATH - 1) {
            outPath[i++] = *p++;
        }
        outPath[i] = L'\0';
        if (i == 0)
            return FALSE;
    }

    /* 验证路径存在性 */
    if (!PathFileExistsW(outPath))
        return FALSE;

    /* 排除目录（我们只接收文件） */
    DWORD attr = GetFileAttributesW(outPath);
    if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY))
        return FALSE;

    return TRUE;
}
