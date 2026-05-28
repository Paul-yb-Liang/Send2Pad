# Send2Pad — 局域网文件传输工具

> 将电脑文件通过 Wi-Fi 发送到 iPad / iPhone，无需数据线、无需安装客户端。

## 使用方法

1. **拖放文件** 到 `Send2Pad.exe` 图标上（不要双击）
2. 窗口弹出，显示 **二维码**
3. 用 iPad 相机扫描二维码，或手动输入下方 URL
4. 浏览器自动下载文件
5. 传输完成后点击「是/否」关闭窗口

## 系统要求

| 项目 | 要求 |
|------|------|
| 操作系统 | Windows 10 / 11（64位） |
| 运行时依赖 | 无（仅 Windows 系统 DLL） |
| 文件大小 | ~60KB（单 exe） |
| 网络 | 电脑与 iPad 连接同一 Wi-Fi |

## 注意事项

- 首次运行可能触发 Windows 防火墙提示，请选择「允许访问」
- 程序启动后不要关闭窗口，传输完成后会自动退出
- 如有多块网卡，程序会自动检测并提示确认
- 支持中文文件名（自动 URL 编码）

## 文件说明

```
Send2Pad_C_Release/
├── Send2Pad.exe      # 可执行文件（拖放使用）
├── README.md         # 本文件
├── 项目概述.md        # 项目背景与目标
├── 技术文档.md        # 技术实现细节
├── 问题与迭代记录.md   # 开发中遇到的问题与解决方案
├── src/              # C 源代码
│   ├── main.c              # 主入口：流程串联
│   ├── parse_cmdline.c     # 命令行参数解析（拖放路径）
│   ├── file_utils.c        # 文件验证 + MIME 类型
│   ├── temp_utils.c        # 临时目录创建/清理
│   ├── network.c           # Winsock 初始化 + IP 探测 + URL 编码 + 多网卡枚举
│   ├── http_server.c       # 手搓 HTTP 服务（单线程）
│   ├── qrcode.c            # libqrencode 集成 + QR 位图生成
│   ├── window.c            # Win32 GUI 窗口
│   └── dialogs.c           # 消息对话框辅助
├── include/           # C 头文件
├── vendor/            # 第三方库（libqrencode 4.1.1 源码）
├── app_icon.rc        # Windows 资源脚本
├── app_icon.ico       # 应用程序图标
└── Makefile           # 编译脚本（MinGW-w64）
```
