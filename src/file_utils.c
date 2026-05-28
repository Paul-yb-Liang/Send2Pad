/* 文件验证与 MIME 模块（步骤 2 实现） */

#include "file_utils.h"
#include <stdlib.h>
#include <wchar.h>

BOOL
ValidateFilePath(const WCHAR *path)
{
    if (path == NULL || path[0] == L'\0')
        return FALSE;

    DWORD attr = GetFileAttributesW(path);
    if (attr == INVALID_FILE_ATTRIBUTES)
        return FALSE;

    /* 排除目录 */
    if (attr & FILE_ATTRIBUTE_DIRECTORY)
        return FALSE;

    return TRUE;
}

typedef struct {
    const WCHAR *ext;
    const char  *mime;
} MimeEntry;

static const MimeEntry MIME_TABLE[] = {
    { L".txt",  "text/plain; charset=utf-8" },
    { L".pdf",  "application/pdf" },
    { L".jpg",  "image/jpeg" },
    { L".jpeg", "image/jpeg" },
    { L".png",  "image/png" },
    { L".gif",  "image/gif" },
    { L".bmp",  "image/bmp" },
    { L".zip",  "application/zip" },
    { L".mp4",  "video/mp4" },
    { L".mov",  "video/quicktime" },
    { L".doc",  "application/msword" },
    { L".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document" },
    { L".xls",  "application/vnd.ms-excel" },
    { L".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet" },
    { L".html", "text/html; charset=utf-8" },
    { L".htm",  "text/html; charset=utf-8" },
    { L".xml",  "application/xml" },
};

static const size_t MIME_TABLE_SIZE = sizeof(MIME_TABLE) / sizeof(MIME_TABLE[0]);

const char *
GetMimeType(const WCHAR *path)
{
    if (path == NULL)
        return "application/octet-stream";

    /* 定位最后一个 '.' 字符 */
    const WCHAR *dot = wcsrchr(path, L'.');
    if (dot == NULL)
        return "application/octet-stream";

    /* 转为小写后查表，表内全小写 */
    WCHAR ext_lower[16];
    size_t elen = wcslen(dot);
    if (elen > 15)  /* 扩展名过长，当成未知 */
        return "application/octet-stream";

    for (size_t i = 0; i < elen; i++)
        ext_lower[i] = towlower(dot[i]);
    ext_lower[elen] = L'\0';

    for (size_t i = 0; i < MIME_TABLE_SIZE; i++) {
        if (wcscmp(ext_lower, MIME_TABLE[i].ext) == 0)
            return MIME_TABLE[i].mime;
    }

    return "application/octet-stream";
}
