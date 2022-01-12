@echo off
set libs_win=user32.lib gdi32.lib OpenGL32.lib  Comdlg32.lib
cl /nologo -Iinclude -Igsl\include\ src\main.c /std:c++latest /wd4244 /wd4267 /wd4101  /Fe./build/main.exe -Fo./build/main.obj -Fd./build/main.pdb %libs_win%  -W3 /Zi /MT /EHsc
