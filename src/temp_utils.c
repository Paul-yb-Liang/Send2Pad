/* 临时目录管理模块（步骤 2 实现） */

#include "temp_utils.h"
#include <stdio.h>

BOOL
CreateTempWorkspace(WCHAR *outDir, size_t maxLen)
{
    WCHAR tempPath[MAX_PATH];

    /* 获取系统临时目录 */
    DWORD ret = GetTempPathW(MAX_PATH, tempPath);
    if (ret == 0 || ret > MAX_PATH)
        return FALSE;

    /* 用系统时间生成唯一子目录名：Send2Pad_YYYYMMDD_HHMMSS_xxxx */
    WCHAR dirName[MAX_PATH];
    SYSTEMTIME st;
    GetLocalTime(&st);

    /* 附加随机后缀避免高并发冲突 */
    DWORD randSuffix = GetTickCount() & 0xFFFF;

    _snwprintf(dirName, MAX_PATH,
               L"Send2Pad_%04d%02d%02d_%02d%02d%02d_%04x",
               st.wYear, st.wMonth, st.wDay,
               st.wHour, st.wMinute, st.wSecond,
               randSuffix);

    /* 拼接完整路径 */
    _snwprintf(outDir, maxLen, L"%s%s", tempPath, dirName);

    /* 创建目录 */
    if (!CreateDirectoryW(outDir, NULL)) {
        /* 如果目录已存在（极低概率），重试一次 */
        if (GetLastError() == ERROR_ALREADY_EXISTS)
            return FALSE;
        return FALSE;
    }

    return TRUE;
}

BOOL
CopyFileToTemp(const WCHAR *srcPath,
                 const WCHAR *tempDir,
                 WCHAR *outDstPath,
                 size_t maxLen)
{
    /* 获取源文件名 */
    const WCHAR *fileName = wcsrchr(srcPath, L'\\');
    if (fileName == NULL)
        fileName = srcPath;
    else
        fileName++; /* 跳过反斜线 */

    /* 拼出目标路径 */
    _snwprintf(outDstPath, maxLen, L"%s\\%s", tempDir, fileName);

    /* 复制文件 */
    if (!CopyFileW(srcPath, outDstPath, FALSE)) {
        outDstPath[0] = L'\0';
        return FALSE;
    }

    return TRUE;
}

/* 递归删除目录内容的辅助函数 */
static BOOL
RemoveDirectoryRecursive(const WCHAR *dir)
{
    WCHAR searchPath[MAX_PATH];
    _snwprintf(searchPath, MAX_PATH, L"%s\\*", dir);

    WIN32_FIND_DATAW ffd;
    HANDLE hFind = FindFirstFileW(searchPath, &ffd);
    if (hFind == INVALID_HANDLE_VALUE)
        return FALSE;

    do {
        /* 跳过 . 和 .. */
        if (wcscmp(ffd.cFileName, L".") == 0 || wcscmp(ffd.cFileName, L"..") == 0)
            continue;

        WCHAR fullPath[MAX_PATH];
        _snwprintf(fullPath, MAX_PATH, L"%s\\%s", dir, ffd.cFileName);

        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            /* 递归删除子目录 */
            RemoveDirectoryRecursive(fullPath);
            RemoveDirectoryW(fullPath);
        } else {
            /* 删除文件 */
            SetFileAttributesW(fullPath, FILE_ATTRIBUTE_NORMAL); /* 去掉只读等属性 */
            DeleteFileW(fullPath);
        }
    } while (FindNextFileW(hFind, &ffd) != 0);

    FindClose(hFind);
    return TRUE;
}

void
CleanupTempWorkspace(const WCHAR *dir)
{
    if (dir == NULL || dir[0] == L'\0')
        return;

    RemoveDirectoryRecursive(dir);
    RemoveDirectoryW(dir);
}
