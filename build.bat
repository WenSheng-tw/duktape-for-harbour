@echo off
setlocal enabledelayedexpansion

:: 設置開發工具環境變量
set HB_ROOT=D:\hb32
set FWH_ROOT=D:\fwh2006
set BCC_ROOT=D:\bcc74
set PATH=%HB_ROOT%\bin\win\bcc;%BCC_ROOT%\bin;%PATH%

:: 編譯 Duktape 函式庫
echo Building Duktape library...
bcc32 -c -I. duktape.c
tlib duktape.lib + duktape.obj

:: 使用 hbmk2 編譯
echo Building with hbmk2...
hbmk2 -comp=bcc duktape.hbp

echo Build completed successfully! 