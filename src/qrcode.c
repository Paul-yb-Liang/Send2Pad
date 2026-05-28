/* 二维码生成模块（步骤 3 实现 — 集成 libqrencode） */

#include "qrcode.h"
#include "qrencode.h"
#include <stdlib.h>

/*
 * GenerateQRBitmap
 * 生成二维码位图，自动添加 QR 规范要求的 4 模块白色空白边（quiet zone）
 * 参数：
 *   urlText   — URL 文本（UTF-8）
 *   pixelSize — 每个 QR 模块的像素大小（推荐 8）
 *   border    — 空白边宽（模块数，QR 规范要求 >= 4）
 */
HBITMAP
GenerateQRBitmap(const char *urlText, int pixelSize, int border)
{
    if (urlText == NULL || urlText[0] == '\0' || pixelSize < 1)
        return NULL;

    if (border < 0)
        border = 0;

    /* 调用 libqrencode 生成二维码数据 */
    QRcode *qrcode = QRcode_encodeString(urlText, 0, QR_ECLEVEL_M, QR_MODE_8, 1);
    if (qrcode == NULL)
        return NULL;

    int width = qrcode->width;               /* QR 模块矩阵宽度 */
    int totalModules = width + 2 * border;    /* 含空白边的总模块数 */
    int bmpSize = totalModules * pixelSize;

    /* 准备 DIBSection 信息 */
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = bmpSize;
    bmi.bmiHeader.biHeight      = -bmpSize;          /* 负值 = 从上到下 */
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;                 /* 32-bit BGRA */
    bmi.bmiHeader.biCompression = BI_RGB;

    void *pixels = NULL;
    HDC hdc = GetDC(NULL);
    HBITMAP hBmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pixels, NULL, 0);
    ReleaseDC(NULL, hdc);

    if (hBmp == NULL || pixels == NULL) {
        QRcode_free(qrcode);
        if (hBmp) DeleteObject(hBmp);
        return NULL;
    }

    /* 先全部填充为白色（空白边） */
    unsigned int *row = (unsigned int *)pixels;
    for (int y = 0; y < bmpSize; y++) {
        for (int x = 0; x < bmpSize; x++) {
            row[x] = 0xFFFFFFFF;
        }
        row += bmpSize;
    }

    /* 在中间区域绘制 QR 码模块 */
    int offset = border * pixelSize;  /* 左边/上边空白边的像素偏移 */
    int qrPixels = width * pixelSize; /* QR 码实际占用的像素大小 */

    row = (unsigned int *)pixels + offset * bmpSize + offset;
    for (int y = 0; y < qrPixels; y++) {
        int moduleY = y / pixelSize;
        for (int x = 0; x < qrPixels; x++) {
            int moduleX = x / pixelSize;
            int moduleIdx = moduleY * width + moduleX;

            /* libqrencode 的 data 中，0 = 白，非 0 = 黑 */
            if (qrcode->data[moduleIdx] & 1)
                row[x] = 0xFF000000;  /* 黑色 (BGRA) */
            /* 否则保持白色（已在上面预设） */
        }
        row += bmpSize;
    }

    QRcode_free(qrcode);
    return hBmp;
}

/*
 * GenerateQRBitmapAutoFit
 * 根据最大尺寸自动选择最佳 pixelSize，确保二维码完整显示
 */
HBITMAP
GenerateQRBitmapAutoFit(const char *urlText, int maxSize, int border)
{
    if (urlText == NULL || urlText[0] == '\0' || maxSize < 32)
        return NULL;

    if (border < 0)
        border = 0;

    /* 先用版 0（自动选择）编码，获取所需模块数 */
    QRcode *qrcode = QRcode_encodeString(urlText, 0, QR_ECLEVEL_M, QR_MODE_8, 1);
    if (qrcode == NULL)
        return NULL;

    int width = qrcode->width;
    int totalModules = width + 2 * border;

    /* 计算最佳 pixelSize：在不超过 maxSize 的前提下尽可能大 */
    int pixelSize = maxSize / totalModules;
    if (pixelSize < 2) {
        /* 即使 pixelSize=1 也放不下，此时用最小尺寸 */
        pixelSize = 1;
    }
    if (pixelSize > 12) {
        /* 上限 12px，再大就太大了 */
        pixelSize = 12;
    }

    QRcode_free(qrcode);

    /* 用计算出的 pixelSize 重新生成 */
    return GenerateQRBitmap(urlText, pixelSize, border);
}

void
FreeQRBitmap(HBITMAP hBmp)
{
    if (hBmp)
        DeleteObject(hBmp);
}
