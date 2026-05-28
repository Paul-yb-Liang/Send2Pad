/* 对话框辅助模块（步骤 6-7 实现） */

#include "dialogs.h"

void
ShowError(const WCHAR *title, const WCHAR *msg)
{
    MessageBoxW(NULL, msg, title, MB_OK | MB_ICONERROR);
}

void
ShowInfo(const WCHAR *title, const WCHAR *msg)
{
    MessageBoxW(NULL, msg, title, MB_OK | MB_ICONINFORMATION);
}

void
ShowFirewallHint(HWND hwnd)
{
    MessageBoxW(hwnd,
        L"首次使用可能触发 Windows 防火墙提示。\n\n"
        L"请在弹出窗口中点击「允许访问」。\n"
        L"如果未弹出，文件传输将无法正常进行。",
        L"防火墙提示",
        MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
}
