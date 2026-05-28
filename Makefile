# Makefile — Send2Pad C 版编译脚本
# 用法：make all  或  make clean
# 环境：MinGW-w64 (Git Bash / MSYS2)

CC      = gcc
CFLAGS  = -O1 -s -Wall -DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN \
          -DSTATIC_IN_RELEASE=static \
          -DMAJOR_VERSION=4 -DMINOR_VERSION=1 -DMICRO_VERSION=1 -DVERSION='"4.1.1"' \
          -Iinclude -Ivendor/libqrencode-4.1.1
LDFLAGS = -mwindows -lws2_32 -lgdi32 -luser32 -lshell32 -lshlwapi -liphlpapi
TARGET  = Send2Pad.exe
RES_OBJ = app_icon.o

# 源文件
CSRCS = src/main.c \
        src/parse_cmdline.c \
        src/file_utils.c \
        src/temp_utils.c \
        src/network.c \
        src/http_server.c \
        src/qrcode.c \
        src/window.c \
        src/dialogs.c

# libqrencode 源码（直接编译进 exe，无 DLL 依赖）
QRCS = vendor/libqrencode-4.1.1/qrencode.c \
        vendor/libqrencode-4.1.1/qrinput.c \
        vendor/libqrencode-4.1.1/qrspec.c \
        vendor/libqrencode-4.1.1/split.c \
        vendor/libqrencode-4.1.1/mask.c \
        vendor/libqrencode-4.1.1/mmask.c \
        vendor/libqrencode-4.1.1/rsecc.c \
        vendor/libqrencode-4.1.1/bitstream.c \
        vendor/libqrencode-4.1.1/mqrspec.c

all: $(RES_OBJ) $(TARGET)

$(RES_OBJ): app_icon.rc app_icon.ico
	windres app_icon.rc -O coff -o $@

$(TARGET): $(CSRCS) $(QRCS) $(RES_OBJ)
	$(CC) $(CFLAGS) $(RES_OBJ) $(CSRCS) $(QRCS) -o $(TARGET) $(LDFLAGS)
	-strip $(TARGET)

clean:
	rm -f $(TARGET) $(RES_OBJ)

.PHONY: all clean
