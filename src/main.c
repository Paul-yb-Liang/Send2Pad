/* 主程序入口 — File Transfer C 重写 */
/* 步骤 5：完整流程 — 拖放文件 → HTTP 服务 → QR 码 → GUI → 弹窗 → 清理 */

#include <windows.h>
#include <wchar.h>

/* 项目头文件 */
#include "parse_cmdline.h"
#include "file_utils.h"
#include "temp_utils.h"
#include "network.h"
#include "http_server.h"
#include "qrcode.h"
#include "window.h"
#include "dialogs.h"

/* 全局变量 */
HINSTANCE g_hInst = NULL;
static WCHAR g_tempDir[MAX_PATH];
static HBITMAP g_hQRBmp = NULL;   /* 用于在窗口销毁时释放 */

/* 清理 */
static void
CleanupAndExit(void)
{
    if (g_hQRBmp) {
        DeleteObject(g_hQRBmp);
        g_hQRBmp = NULL;
    }
    if (g_tempDir[0] != L'\0')
        CleanupTempWorkspace(g_tempDir);
    WSACleanup();
}

/* WinMain — Windows 程序入口 */
int WINAPI WinMain(HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine,
                    int nCmdShow)
{
    (void)hPrevInstance; (void)nCmdShow;
    g_hInst = hInstance;

    WCHAR filePath[MAX_PATH];
    WCHAR dstPath[MAX_PATH];
    WCHAR localIP[64];
    WCHAR fileName[MAX_PATH];
    char ipBuf[32];
    char encodedName[1024];
    WCHAR url[2048];
    SOCKET listenSock = INVALID_SOCKET;
    HttpServerCtx ctx;
    HANDLE hThread = NULL;

    /* --- 1. 解析命令行参数（拖放文件路径）--- */
    LPCWSTR wCmdLine = GetCommandLineW();
    while (*wCmdLine == L'"') {
        wCmdLine++;
        while (*wCmdLine && *wCmdLine != L'"')
            wCmdLine++;
        if (*wCmdLine == L'"')
            wCmdLine++;
        if (*wCmdLine == L' ')
            wCmdLine++;
        break;
    }
    if (*wCmdLine && *wCmdLine != L'"' && wCmdLine == GetCommandLineW()) {
        while (*wCmdLine && *wCmdLine != L' ')
            wCmdLine++;
        while (*wCmdLine == L' ')
            wCmdLine++;
    }

    if (!ParseCommandLine(wCmdLine, filePath))
        goto error_no_file;

    /* --- 2. 验证文件 --- */
    if (!ValidateFilePath(filePath))
        goto error_generic;

    /* --- 3. 获取 MIME 类型 --- */
    const char *mimeType = GetMimeType(filePath);

    /* --- 4. 创建临时工作目录 --- */
    if (!CreateTempWorkspace(g_tempDir, MAX_PATH))
        goto error_generic;

    /* --- 5. 复制文件到临时目录 --- */
    if (!CopyFileToTemp(filePath, g_tempDir, dstPath, MAX_PATH))
        goto error_generic;

    /* --- 6. 初始化 Winsock --- */
    if (!InitWinsock())
        goto error_generic;

    /* --- 7. 获取本机局域网 IP（支持多网卡） --- */
    {
        WCHAR allIPs[8][64];   /* 最多 8 个网卡 */
        int ipCount = GetAllLocalIPs(allIPs, 8);

        if (ipCount == 0)
            goto error_generic;

        /* 取第一个有效 IP */
        wcscpy(localIP, allIPs[0]);

        /* 如果有多网卡，提示用户确认 */
        if (ipCount > 1) {
            WCHAR msg[1024];
            int pos = _snwprintf(msg, 1024,
                L"检测到 %d 个网卡，自动选择：%s\n\n"
                L"全部 IP 列表：", ipCount, localIP);
            for (int i = 0; i < ipCount && i < 8; i++) {
                pos += _snwprintf(msg + pos, 1024 - pos,
                    L"\n  %d. %s", i + 1, allIPs[i]);
            }
            pos += _snwprintf(msg + pos, 1024 - pos,
                L"\n\n如果设备无法连接，请检查电脑是否连接了正确的网络。");
            ShowInfo(L"网卡信息", msg);
        }
    }

    WideCharToMultiByte(CP_UTF8, 0, localIP, -1, ipBuf, sizeof(ipBuf), NULL, NULL);

    /* --- 8. 绑定随机端口 --- */
    int port;
    listenSock = BindRandomPort(ipBuf, &port);
    if (listenSock == INVALID_SOCKET)
        goto error_generic;

    /* --- 9. URL 编码 + 构建 URL --- */
    const WCHAR *baseName = wcsrchr(filePath, L'\\');
    if (baseName) baseName++; else baseName = filePath;
    wcscpy(fileName, baseName);
    UrlEncode(fileName, encodedName, sizeof(encodedName));
    _snwprintf(url, 2048, L"http://%s:%d/%S", localIP, port, encodedName);

    /* --- 10. 生成二维码（自动适配控件尺寸） --- */
    {
        char urlUtf8[2048];
        WideCharToMultiByte(CP_UTF8, 0, url, -1, urlUtf8, sizeof(urlUtf8), NULL, NULL);
        g_hQRBmp = GenerateQRBitmapAutoFit(urlUtf8, 460, 4);
        /* QR 码为 NULL 也能继续（用户仍可手动输入 URL） */
    }

    /* --- 11. 注册窗口类 --- */
    if (!RegisterAppClass(hInstance))
        goto error_generic;

    /* --- 12. 初始化 HTTP 上下文（hwnd 稍后设置） --- */
    ctx.listenSock  = listenSock;
    wcscpy(ctx.tempFilePath, dstPath);
    wcscpy(ctx.fileName, fileName);
    ctx.hwnd        = NULL;
    strncpy(ctx.mimeType, mimeType, sizeof(ctx.mimeType) - 1);
    ctx.mimeType[sizeof(ctx.mimeType) - 1] = '\0';

    /* --- 13. 创建主窗口（会自动发送 WM_HTTP_READY 设置 QR + URL） --- */
    HWND hwnd = CreateMainWindow(hInstance, g_hQRBmp, url);
    if (hwnd == NULL)
        goto error_generic;

    /* 将 ctx 指针存入窗口，供 WM_DESTROY 关闭 listen socket 使用 */
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)&ctx);

    /* --- 14. 设置 hwnd 并启动 HTTP 服务线程 --- */
    ctx.hwnd = hwnd;
    hThread = StartHttpThread(&ctx);
    if (hThread == NULL) {
        DestroyWindow(hwnd);
        goto error_generic;
    }

    /* --- 15. 进入消息循环（窗口显示 + HTTP 服务） --- */
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    /* --- 清理 --- */
    if (hThread) {
        /* 等待 HTTP 线程自然结束：
         * - 若传输中 → 传完后线程退出
         * - 若未开始 → listenSock 已被 WM_DESTROY 关闭 → accept 失败 → 线程退出 */
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }
    CleanupAndExit();
    return 0;

error_generic:
    if (listenSock != INVALID_SOCKET)
        closesocket(listenSock);
    CleanupAndExit();
    ShowError(L"File Transfer", L"初始化失败，请重试。");
    return 1;

error_no_file:
    ShowError(L"File Transfer", L"请将文件拖到此程序上运行！");
    return 1;
}
