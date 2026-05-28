/* GUI 窗口模块（步骤 5 实现 — 完整 Win32 GUI） */

#include "window.h"
#include "dialogs.h"
#include "http_server.h"   /* HttpServerCtx, closesocket */

/* 控件尺寸常量 */
#define WINDOW_CLIENT_W    480   /* 期望客户区宽度 */
#define WINDOW_CLIENT_H    580   /* 期望客户区高度 */
#define QR_SIZE            460   /* QR 码静态控件尺寸（像素） */
#define QR_X               ((WINDOW_CLIENT_W - QR_SIZE) / 2)  /* 水平居中 */
#define QR_Y               40
#define STATUS_H           24
#define URL_Y              (QR_Y + QR_SIZE + 12)
#define URL_H              60

/* 全局变量 */
static HWND g_hStatusLabel = NULL;
static HWND g_hQRLabel     = NULL;
static HWND g_hUrlLabel    = NULL;

/* 窗口回调 */
LRESULT CALLBACK
WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        case WM_CREATE:
        {
            HINSTANCE hInst = (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE);

            /* 创建顶部状态标签 */
            g_hStatusLabel = CreateWindowW(L"STATIC",
                L"📲 请用 iPad 相机扫码下载...",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                0, 10, WINDOW_CLIENT_W, STATUS_H,
                hwnd, NULL, hInst, NULL);

            /* 二维码图片区域 */
            g_hQRLabel = CreateWindowW(L"STATIC", NULL,
                WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTERIMAGE,
                QR_X, QR_Y, QR_SIZE, QR_SIZE,
                hwnd, NULL, hInst, NULL);

            /* 创建底部 URL 标签（只读编辑框，自动换行） */
            g_hUrlLabel = CreateWindowW(L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_READONLY,
                10, URL_Y, WINDOW_CLIENT_W - 20, URL_H,
                hwnd, NULL, hInst, NULL);

            return 0;
        }

        case WM_CTLCOLORSTATIC:
        {
            /* 让 STATIC 控件背景白底 */
            HDC hdcStatic = (HDC)wParam;
            SetBkMode(hdcStatic, TRANSPARENT);
            return (LRESULT)GetStockObject(WHITE_BRUSH);
        }

        case WM_CTLCOLOREDIT:
        {
            /* 让 EDIT（URL 标签）背景白底，文字黑色 */
            HDC hdcEdit = (HDC)wParam;
            SetBkColor(hdcEdit, RGB(255, 255, 255));
            SetTextColor(hdcEdit, RGB(0, 0, 0));
            return (LRESULT)GetStockObject(WHITE_BRUSH);
        }

        case WM_HTTP_READY:
        {
            /* wParam = HBITMAP, lParam = URL 字符串指针 */
            HBITMAP hQRBmp = (HBITMAP)wParam;
            const WCHAR *url = (const WCHAR *)lParam;

            if (hQRBmp) {
                /* 设置二维码图片 */
                SendMessageW(g_hQRLabel, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hQRBmp);
                /* 不要 DeleteObject — bitmap 会在退出时由 main 清理 */
            }

            if (url) {
                SetWindowTextW(g_hUrlLabel, url);
            }

            return 0;
        }

        case WM_HTTP_DONE:
        {
            /* 文件传输完成 */
            UpdateWindow(hwnd);
            SetWindowTextW(g_hStatusLabel, L"✅ 文件已发送！");

            /* 弹出确认对话框 */
            MessageBoxW(hwnd,
                L"文件成功发送到 iPad？",
                L"传输结束",
                MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND);

            /* 无论是/否都关闭窗口 */
            DestroyWindow(hwnd);
            return 0;
        }

        case WM_DESTROY:
        {
            /* 先关闭监听 socket，唤醒 HTTP 线程 */
            HttpServerCtx *pCtx = (HttpServerCtx *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
            if (pCtx && pCtx->listenSock != INVALID_SOCKET) {
                closesocket(pCtx->listenSock);
                pCtx->listenSock = INVALID_SOCKET;
            }

            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

BOOL
RegisterAppClass(HINSTANCE hInst)
{
    WNDCLASSEXW wc = {0};
    wc.cbSize        = sizeof(WNDCLASSEXW);
    wc.lpszClassName = L"Send2PadWndClass";
    wc.lpfnWndProc   = WndProc;
    wc.hInstance      = hInst;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
    /* 从嵌入资源加载自定义图标（ID 101） */
    wc.hIcon         = LoadIconW(hInst, MAKEINTRESOURCEW(101));
    wc.hIconSm       = LoadIconW(hInst, MAKEINTRESOURCEW(101));
    return RegisterClassExW(&wc);
}

HWND
CreateMainWindow(HINSTANCE hInst, HBITMAP hQRBmp, const WCHAR *url)
{
    /* 根据期望的客户区尺寸精确计算窗口尺寸 */
    RECT rc = {0, 0, WINDOW_CLIENT_W, WINDOW_CLIENT_H};
    DWORD style = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
    AdjustWindowRect(&rc, style, FALSE);

    HWND hwnd = CreateWindowW(L"Send2PadWndClass",
                               L"Send to iPad",
                               style,
                               CW_USEDEFAULT, CW_USEDEFAULT,
                               rc.right - rc.left,
                               rc.bottom - rc.top,
                               NULL, NULL, hInst, NULL);

    if (hwnd == NULL)
        return NULL;

    /* 向窗口发送二维码和 URL */
    if (hQRBmp || url) {
        SendMessageW(hwnd, WM_HTTP_READY, (WPARAM)hQRBmp, (LPARAM)url);
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    return hwnd;
}
