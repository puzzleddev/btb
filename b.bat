@echo off
setlocal
cls
set "outdir=out"
if not exist %outdir% md %outdir%
if not exist %outdir%\tmp md %outdir%\tmp
pushd %outdir%
set "cflags=-Wall -Wextra -Werror -fvisibility=hidden -Wno-unused-parameter -Wno-unused-function"
clang %cflags% -g ../preproc.c -o tmp/preproc.exe
pushd tmp
"preproc.exe"
popd
clang %cflags% -DB_PLAT=B_PLAT_HTML5 -O2 -flto -nostdlib -ffunction-sections -fdata-sections --target=wasm32-feestanding -Wl,--no-entry,--strip-all,--export-dynamic,--lto-O3,-O3 ../p_html5.c -Itmp -o btb.wasm
copy /B ..\p_html5.js btb.js /Y > nul
copy /B ..\p_html5.html index.html /Y > nul
popd %outdir%
