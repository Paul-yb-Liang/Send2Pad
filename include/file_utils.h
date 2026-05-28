/* 文件验证与 MIME 模块头文件 */

#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <windows.h>
#include <stdbool.h>

/*
 * ValidateFilePath
 * 检查路径是否为有效文件
 * 参数：path — 完整文件路径（宽字符）
 * 返回：TRUE 有效，FALSE 无效
 */
BOOL ValidateFilePath(const WCHAR *path);

/*
 * GetMimeType
 * 根据文件扩展名返回 MIME 类型字符串
 * 参数：path — 完整文件路径
 * 返回：静态字符串指针，无需释放
 */
const char *GetMimeType(const WCHAR *path);

#endif /* FILE_UTILS_H */
