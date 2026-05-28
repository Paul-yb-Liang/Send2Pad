/* 网络初始化模块（步骤 6-7 实现） */

#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <iphlpapi.h>    /* GetAdaptersAddresses */
/* iphlpapi.lib linked via LDFLAGS in Makefile */

BOOL
InitWinsock(void)
{
    WSADATA wsa;
    return (WSAStartup(MAKEWORD(2, 2), &wsa) == 0);
}

BOOL
GetLocalIP(WCHAR *outIP, size_t maxLen)
{
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET)
        return FALSE;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(80);
    /* 用 8.8.8.8 作为 UDP 探测目标，不会真的发数据包 */
    addr.sin_addr.s_addr = inet_addr("8.8.8.8");

    if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        closesocket(s);
        return FALSE;
    }

    struct sockaddr_in localAddr;
    int localLen = sizeof(localAddr);
    BOOL ok = FALSE;
    if (getsockname(s, (struct sockaddr *)&localAddr, &localLen) == 0) {
        /* 将 IP 转为宽字符串 */
        char ipBuf[32];
        strcpy(ipBuf, inet_ntoa(localAddr.sin_addr));
        size_t len = mbstowcs(outIP, ipBuf, maxLen - 1);
        if (len != (size_t)-1) {
            outIP[len] = L'\0';
            ok = TRUE;
        }
    }

    closesocket(s);
    return ok;
}

SOCKET
BindRandomPort(const char *ip, int *outPort)
{
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET)
        return INVALID_SOCKET;

    /* 允许地址重用，避免 TIME_WAIT 问题 */
    int optval = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof(optval));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(0);   /* 0 = 系统分配随机端口 */
    addr.sin_addr.s_addr = inet_addr(ip);

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        closesocket(s);
        return INVALID_SOCKET;
    }

    /* 获取实际分配的端口 */
    int nameLen = sizeof(addr);
    if (getsockname(s, (struct sockaddr *)&addr, &nameLen) != 0) {
        closesocket(s);
        return INVALID_SOCKET;
    }

    *outPort = ntohs(addr.sin_port);
    return s;
}

/* 判断字符是否需要 URL 编码（保留 unreserved 字符） */
static BOOL
NeedsUrlEncode(char c)
{
    /* unreserved: ALPHA / DIGIT / "-" / "." / "_" / "~" */
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
        return FALSE;
    if (c == '-' || c == '.' || c == '_' || c == '~')
        return FALSE;
    return TRUE;
}

BOOL
UrlEncode(const WCHAR *src, char *dst, size_t maxLen)
{
    if (src == NULL || dst == NULL || maxLen == 0)
        return FALSE;

    size_t di = 0;
    char utf8Buf[8]; /* UTF-8 编码一个宽字符最多 4 字节 + 3 字节 %XX */

    while (*src != L'\0' && di < maxLen - 1) {
        /* 将宽字符转为 UTF-8 */
        int utf8Len = WideCharToMultiByte(CP_UTF8, 0, src, 1, utf8Buf, sizeof(utf8Buf), NULL, NULL);
        if (utf8Len <= 0) {
            /* 转换失败，跳过此字符 */
            src++;
            continue;
        }

        for (int i = 0; i < utf8Len; i++) {
            unsigned char uc = (unsigned char)utf8Buf[i];
            if (NeedsUrlEncode((char)uc)) {
                /* percent-encode */
                if (di + 3 >= maxLen)
                    return FALSE;   /* 缓冲区不足 */
                _snprintf(dst + di, maxLen - di, "%%%02X", uc);
                di += 3;
            } else {
                if (di + 1 >= maxLen)
                    return FALSE;
                dst[di++] = (char)uc;
            }
        }
        src++;
    }

    dst[di] = '\0';
    return TRUE;
}

/*
 * GetAllLocalIPs
 * 枚举所有活跃的非回环 IPv4 地址
 * 参数：
 *   outIPs  — 输出缓冲区数组（每个元素 WCHAR[64]）
 *   maxCount — 数组最大容量
 * 返回：找到的有效 IP 数量（<= maxCount）
 */
int
GetAllLocalIPs(WCHAR outIPs[][64], int maxCount)
{
    ULONG bufLen = 0;
    GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &bufLen);

    IP_ADAPTER_ADDRESSES *buf = (IP_ADAPTER_ADDRESSES *)malloc(bufLen);
    if (buf == NULL)
        return 0;

    ULONG ret = GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX,
                                      NULL, buf, &bufLen);
    if (ret != ERROR_SUCCESS) {
        free(buf);
        return 0;
    }

    int count = 0;
    for (IP_ADAPTER_ADDRESSES *p = buf; p != NULL; p = p->Next) {
        /* 只取已启动的非回环适配器 */
        if (p->OperStatus != IfOperStatusUp)
            continue;
        if (p->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
            continue;

        IP_ADAPTER_UNICAST_ADDRESS *uaddr = p->FirstUnicastAddress;
        while (uaddr != NULL && count < maxCount) {
            SOCKADDR_IN *sa = (SOCKADDR_IN *)uaddr->Address.lpSockaddr;
            if (sa->sin_family == AF_INET) {
                char ipBuf[32];
                strcpy(ipBuf, inet_ntoa(sa->sin_addr));
                mbstowcs(outIPs[count], ipBuf, 63);
                outIPs[count][63] = L'\0';
                count++;
            }
            uaddr = uaddr->Next;
        }
    }

    free(buf);
    return count;
}
