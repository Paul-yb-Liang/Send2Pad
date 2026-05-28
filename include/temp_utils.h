/* 临时目录管理模块头文件 */

#ifndef TEMP_UTILS_H
#define TEMP_UTILS_H

#include <windows.h>
#include <stdbool.h>

/*
 * CreateTempWorkspace
 * 在系统临时目录创建 Send2Pad 工作子目录
 * 参数：
 *   outDir  — 输出缓冲区（WCHAR[MAX_PATH]），接收完整目录路径
 *   filename — 原始文件名（用于构造目标路径，可选）
 * 返回：TRUE 成功，FALSE 失败
 */
BOOL CreateTempWorkspace(WCHAR *outDir, size_t maxLen);

/*
 * CopyFileToTemp
 * 将源文件复制到临时工作目录
 * 参数：
 *   srcPath  — 源文件完整路径
 *   tempDir  — 临时目录路径
 *   outDstPath — 输出缓冲区，接收目标完整路径
 * 返回：TRUE 成功，FALSE 失败
 */
BOOL CopyFileToTemp(const WCHAR *srcPath,
                    const WCHAR *tempDir,
                    WCHAR *outDstPath,
                    size_t maxLen);

/*
 * CleanupTempWorkspace
 * 递归删除临时工作目录及其内容
 * 参数：dir — 要删除的目录路径
 */
void CleanupTempWorkspace(const WCHAR *dir);

#endif /* TEMP_UTILS_H */
