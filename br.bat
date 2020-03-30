@echo off
setlocal
cls
call "b.bat"
set "routdir=docs"
if not exist %routdir% md %routdir%
call uglifyjs out/btb.js > %routdir%/btb.js
copy /B out\index.html %routdir%\index.html /Y > nul
copy /B out\btb.wasm %routdir%\btb.wasm /Y > nul
