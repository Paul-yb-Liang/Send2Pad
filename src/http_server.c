/* HTTP 服务模块（步骤 2 实现） */

#include "http_server.h"
#include "window.h"        /* WM_HTTP_DONE */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 将 URL 百分比编码的字符串解码为 UTF-8 字符串 */
static void
PercentDecode(const char *src, char *dst, size_t maxLen)
{
    size_t di = 0;
    while (*src && di < maxLen - 1) {
        if (*src == '%' && src[1] && src[2]) {
            char hex[3] = { src[1], src[2], '\0' };
            dst[di++] = (char)strtol(hex, NULL, 16);
            src += 3;
        } else if (*src == '+') {
            dst[di++] = ' ';
            src++;
        } else {
            dst[di++] = *src++;
        }
    }
    dst[di] = '\0';
}

/* 比较两个文件名（忽略大小写），先做 percent-decode */
static BOOL
MatchFileName(const char *requestPath, const WCHAR *expectedFile)
{
    /* 将 expectedFile 转为 UTF-8 */
    char expectedUtf8[1024];
    WideCharToMultiByte(CP_UTF8, 0, expectedFile, -1,
                        expectedUtf8, sizeof(expectedUtf8), NULL, NULL);

    /* percent-decode 请求中的文件名 */
    char decoded[1024];
    PercentDecode(requestPath, decoded, sizeof(decoded));

    /* 大小写不敏感比较 */
    return (_stricmp(decoded, expectedUtf8) == 0);
}

/* 发送简单 HTTP 响应 */
static void
SendSimpleResponse(SOCKET sock, int statusCode, const char *statusText,
                   const char *body)
{
    char resp[4096];
    _snprintf(resp, sizeof(resp),
        "HTTP/1.1 %d %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        statusCode, statusText,
        body ? strlen(body) : 0,
        body ? body : "");
    send(sock, resp, strlen(resp), 0);
}

/* 处理单个 HTTP 连接：返回 TRUE 表示文件成功发送，FALSE 表示要继续等待 */
static BOOL
HandleOneRequest(SOCKET clientSock, HttpServerCtx *ctx)
{
    /* 读取 HTTP 请求首行 */
    char reqBuf[4096];
    int totalRead = 0;
    int lineEnd = 0;

    while (totalRead < (int)sizeof(reqBuf) - 1) {
        int n = recv(clientSock, reqBuf + totalRead,
                     sizeof(reqBuf) - 1 - totalRead, 0);
        if (n <= 0)
            break;
        totalRead += n;
        reqBuf[totalRead] = '\0';

        /* 检查是否收到了 \r\n 或 \n */
        for (int i = (totalRead - n > 0 ? totalRead - n : 0); i < totalRead; i++) {
            if (reqBuf[i] == '\n') {
                lineEnd = i;
                break;
            }
        }
        if (lineEnd > 0)
            break;
    }

    if (lineEnd == 0) {
        SendSimpleResponse(clientSock, 400, "Bad Request", NULL);
        return FALSE;
    }

    /* 去除行尾的 \r\n 或 \n */
    if (lineEnd > 0 && reqBuf[lineEnd - 1] == '\r')
        reqBuf[lineEnd - 1] = '\0';
    else
        reqBuf[lineEnd] = '\0';

    /* 解析 GET /path HTTP/1.x */
    char method[16], path[1024], version[16];
    int parsed = sscanf(reqBuf, "%15s %1023s %15s", method, path, version);

    if (parsed < 2 || strcmp(method, "GET") != 0) {
        SendSimpleResponse(clientSock, 405, "Method Not Allowed", NULL);
        return FALSE;
    }

    /* 去除路径开头的 '/' */
    const char *reqFile = path;
    while (*reqFile == '/')
        reqFile++;

    /* 匹配文件名 */
    if (!MatchFileName(reqFile, ctx->fileName)) {
        SendSimpleResponse(clientSock, 404, "Not Found", "Not Found");
        return FALSE;
    }

    /* 打开临时文件 */
    HANDLE hFile = CreateFileW(ctx->tempFilePath, GENERIC_READ,
                                FILE_SHARE_READ, NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        SendSimpleResponse(clientSock, 500, "Internal Server Error", NULL);
        return FALSE;
    }

    /* 获取文件大小 */
    LARGE_INTEGER fileSize;
    GetFileSizeEx(hFile, &fileSize);

    /* 对文件名做 URL 编码用于 Content-Disposition 头 */
    char encodedName[1024];
    {
        char fileNameUtf8[1024];
        WideCharToMultiByte(CP_UTF8, 0, ctx->fileName, -1,
                            fileNameUtf8, sizeof(fileNameUtf8), NULL, NULL);
        size_t ei = 0;
        for (char *p = fileNameUtf8; *p && ei < sizeof(encodedName) - 12; p++) {
            unsigned char uc = (unsigned char)*p;
            if ((uc >= 'A' && uc <= 'Z') || (uc >= 'a' && uc <= 'z') ||
                (uc >= '0' && uc <= '9') || uc == '-' || uc == '.' || uc == '_' || uc == '~') {
                encodedName[ei++] = (char)uc;
            } else {
                int n = _snprintf(encodedName + ei, sizeof(encodedName) - ei, "%%%02X", uc);
                if (n > 0) ei += n;
            }
        }
        encodedName[ei] = '\0';
    }

    /* 组装响应头 */
    char header[2048];
    int headerLen = _snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Disposition: attachment; filename*=UTF-8''%s\r\n"
        "Content-Length: %lld\r\n"
        "Connection: close\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "\r\n",
        ctx->mimeType, encodedName, fileSize.QuadPart);

    /* 发送响应头 */
    send(clientSock, header, headerLen, 0);

    /* 发送文件内容（64KB buffer 流式发送） */
    char buf[65536];
    DWORD bytesRead;
    int sendFailed = 0;
    while (!sendFailed &&
           ReadFile(hFile, buf, sizeof(buf), &bytesRead, NULL) && bytesRead > 0) {
        int sent = 0;
        while (sent < (int)bytesRead) {
            int n = send(clientSock, buf + sent, bytesRead - sent, 0);
            if (n <= 0) {
                sendFailed = 1;
                break;
            }
            sent += n;
        }
    }

    CloseHandle(hFile);
    closesocket(clientSock);

    /* 文件成功发送，通知主窗口 */
    if (ctx->hwnd != NULL) {
        PostMessageW(ctx->hwnd, WM_HTTP_DONE, 0, 0);
    }

    /* 不关闭监听 socket，保持服务运行以支持重复扫码 */
    return TRUE;
}

DWORD WINAPI
HttpThreadProc(LPVOID lpParam)
{
    HttpServerCtx *ctx = (HttpServerCtx *)lpParam;

    /* 开始监听 */
    if (listen(ctx->listenSock, 5) != 0) {
        closesocket(ctx->listenSock);
        return 1;
    }

    /* 循环 accept，直到窗口销毁关闭 listenSock */
    while (ctx->listenSock != INVALID_SOCKET) {
        SOCKET clientSock = accept(ctx->listenSock, NULL, NULL);
        if (clientSock == INVALID_SOCKET) {
            /* listenSock 已被关闭（窗口销毁时清理） */
            break;
        }

        /* 设置 3 秒发送/接收超时，防止客户端拒收时线程阻塞 */
        DWORD timeout = 3000;
        setsockopt(clientSock, SOL_SOCKET, SO_SNDTIMEO,
                   (const char *)&timeout, sizeof(timeout));
        setsockopt(clientSock, SOL_SOCKET, SO_RCVTIMEO,
                   (const char *)&timeout, sizeof(timeout));

        BOOL delivered = HandleOneRequest(clientSock, ctx);

        if (delivered) {
            /* 成功：clientSock 已在 HandleOneRequest 内部关闭 */
            /* 继续等待下一个连接（支持重复扫码） */
        } else {
            /* 失败：关闭客户端 socket，继续 accept */
            closesocket(clientSock);
        }
    }

    /* 确保监听 socket 关闭 */
    if (ctx->listenSock != INVALID_SOCKET) {
        closesocket(ctx->listenSock);
        ctx->listenSock = INVALID_SOCKET;
    }

    return 0;
}

HANDLE
StartHttpThread(HttpServerCtx *ctx)
{
    DWORD threadId;
    HANDLE hThread = CreateThread(NULL, 0, HttpThreadProc, ctx, 0, &threadId);
    return hThread;
}
