/* GUI 窗口模块头文件 */

#ifndef WINDOW_H
#define WINDOW_H

#include <windows.h>

#define WM_HTTP_DONE  (WM_USER + 1)   /* HTTP 传输完成消息 */
#define WM_HTTP_READY (WM_USER + 2)   /* HTTP 服务就绪：wParam=HBITMAP, lParam=URL */

/*
 * RegisterAppClass
 * 注册窗口类
 * 参数：hInst — 应用程序实例句柄
 * 返回：TRUE 成功，FALSE 失败
 */
BOOL RegisterAppClass(HINSTANCE hInst);

/*
 * CreateMainWindow
 * 创建主窗口
 * 参数：hInst    — 应用程序实例句柄
 *       hQRBmp   — 二维码位图句柄
 *       url       — 服务器 URL（用于显示）
 * 返回：主窗口句柄，失败返回 NULL
 */
HWND CreateMainWindow(HINSTANCE hInst, HBITMAP hQRBmp, const WCHAR *url);

/*
 * WndProc
 * 主窗口消息处理函数
 */
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#endif /* WINDOW_H */
