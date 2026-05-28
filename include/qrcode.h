/* 二维码生成模块头文件 */

#ifndef QRCODE_H
#define QRCODE_H

#include <windows.h>

/*
 * GenerateQRBitmap
 * 生成二维码位图，自动添加 QR 规范要求的空白边（quiet zone）
 * 参数：
 *   urlText   — URL 文本（UTF-8）
 *   pixelSize — 每个 QR 模块的像素大小（推荐 8）
 *   border    — 空白边宽（模块数，QR 规范要求 >= 4）
 * 返回：HBITMAP 位图句柄，失败返回 NULL
 */
HBITMAP GenerateQRBitmap(const char *urlText, int pixelSize, int border);

/*
 * GenerateQRBitmapAutoFit
 * 根据最大尺寸自动选择合适的 pixelSize，确保二维码完整显示
 * 参数：
 *   urlText  — URL 文本（UTF-8）
 *   maxSize  — 控件最大尺寸（像素）
 *   border   — 空白边宽（模块数）
 * 返回：HBITMAP 位图句柄，失败返回 NULL
 */
HBITMAP GenerateQRBitmapAutoFit(const char *urlText, int maxSize, int border);

/*
 * FreeQRBitmap
 * 释放二维码位图资源
 * 参数：hBmp — GenerateQRBitmap 返回的句柄
 */
void FreeQRBitmap(HBITMAP hBmp);

#endif /* QRCODE_H */
