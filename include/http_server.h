/* HTTP 服务模块头文件 */

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <windows.h>
#include <winsock2.h>
#include <stdbool.h>

/*
 * HttpServerCtx — HTTP 线程上下文
 */
typedef struct {
    SOCKET listenSock;
    WCHAR  tempFilePath[MAX_PATH];   /* 临时文件完整路径 */
    WCHAR  fileName[MAX_PATH];       /* 原始文件名（用于匹配） */
    HWND   hwnd;                    /* 主窗口句柄 */
    char   mimeType[128];           /* MIME 类型 */
} HttpServerCtx;

/*
 * HttpThreadProc
 * HTTP 服务线程入口函数
 * 参数：lpParam — HttpServerCtx 指针
 * 返回：0 正常结束
 */
DWORD WINAPI HttpThreadProc(LPVOID lpParam);

/*
 * StartHttpThread
 * 启动 HTTP 服务线程
 * 参数：ctx — 已初始化的 HttpServerCtx
 * 返回：线程句柄，失败返回 NULL
 */
HANDLE StartHttpThread(HttpServerCtx *ctx);

#endif /* HTTP_SERVER_H */
