@echo off
setlocal
cls
set "outdir=docs"
if not exist %outdir% md %outdir%
pushd %outdir%
set "cflags=-Wall -Wextra -Werror -fvisibility=hidden"
clang %cflags% -DB_PLAT=B_PLAT_HTML5 -O2 -flto -nostdlib -ffunction-sections -fdata-sections --target=wasm32-feestanding -Wl,--no-entry,--strip-all,--export-dynamic,--lto-O3,-O3 ../p_html5.c -o btb.wasm
copy /B ..\p_html5.js btb.js /Y > nul
copy /B ..\p_html5.html index.html /Y > nul
popd %outdir%
