/* 网络初始化模块头文件 */

#ifndef NETWORK_H
#define NETWORK_H

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdbool.h>

/* ws2_32.lib linked via LDFLAGS in Makefile */

/*
 * InitWinsock
 * 初始化 WinSock2 DLL
 * 返回：TRUE 成功，FALSE 失败
 */
BOOL InitWinsock(void);

/*
 * GetLocalIP
 * 通过 UDP 路由探测获取本机局域网 IP
 * 参数：outIP — 输出缓冲区（宽字符）
 *       maxLen — 缓冲区长度（字符数）
 * 返回：TRUE 成功，FALSE 失败
 */
BOOL GetLocalIP(WCHAR *outIP, size_t maxLen);

/*
 * BindRandomPort
 * 绑定随机可用端口
 * 参数：ip — 绑定的 IP 地址字符串
 *       outPort — 输出端口号
 * 返回：socket 句柄（>0），失败返回 INVALID_SOCKET
 */
SOCKET BindRandomPort(const char *ip, int *outPort);

/*
 * UrlEncode
 * 对文件名进行 URL 编码（percent-encoding）
 * 参数：src    — 输入宽字符串（原始文件名）
 *       dst    — 输出缓冲区（UTF-8 URL 编码）
 *       maxLen — 输出缓冲区长度
 * 返回：TRUE 成功，FALSE 失败
 */
BOOL UrlEncode(const WCHAR *src, char *dst, size_t maxLen);

/*
 * GetAllLocalIPs
 * 枚举所有活跃的非回环 IPv4 地址
 * 参数：
 *   outIPs  — 输出缓冲区数组（每个元素 WCHAR[64]）
 *   maxCount — 数组最大容量
 * 返回：找到的有效 IP 数量（<= maxCount）
 */
int GetAllLocalIPs(WCHAR outIPs[][64], int maxCount);

#endif /* NETWORK_H */
