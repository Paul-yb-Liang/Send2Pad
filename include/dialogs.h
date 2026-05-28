/* 对话框辅助模块头文件 */

#ifndef DIALOGS_H
#define DIALOGS_H

#include <windows.h>
#include <stdbool.h>

/*
 * ShowError
 * 显示错误对话框（图标：红叉）
 * 参数：title — 对话框标题（宽字符）
 *       msg   — 错误消息（宽字符）
 */
void ShowError(const WCHAR *title, const WCHAR *msg);

/*
 * ShowInfo
 * 显示信息对话框（图标：信息）
 * 参数：title — 对话框标题（宽字符）
 *       msg   — 信息消息（宽字符）
 */
void ShowInfo(const WCHAR *title, const WCHAR *msg);

/*
 * ShowFirewallHint
 * 在首次启动时提示用户注意防火墙弹窗
 * 参数：hwnd — 父窗口句柄（可为 NULL）
 */
void ShowFirewallHint(HWND hwnd);

#endif /* DIALOGS_H */
