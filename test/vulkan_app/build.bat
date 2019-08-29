@echo off

set CC=cl

set DB=devenv

set CFLAGS=-Zi /EHsc /std:c++latest /DEBUG:FULL

set DEF=/DGLM_ENABLE_EXPERIMENTAL /DUNITY_BUILD /D_MBCS

set GLFW_INC_DIR=/I C:/dependencies/glfw-3.2.1.bin.WIN64/include
set GLM_INC_DIR=/I C:/dependencies/
set VULKAN_INC_DIR=/I C:/VulkanSDK/1.1.108.0/Include
set VML_INC_DIR=/I ../../

set INC=%GLM_INC_DIR% %VULKAN_INC_DIR% %GLFW_INC_DIR% %VML_INC_DIR%

set BIN=DebugginTest.exe

set SRC=core.cpp

set LIBS=winmm.lib user32.lib User32.lib Gdi32.lib Shell32.lib kernel32.lib gdi32.lib msvcrt.lib msvcmrt.lib C:/VulkanSDK/1.1.108.0/Lib/vulkan-1.lib C:/dependencies/glfw-3.2.1.bin.WIN64/lib-vc2015/glfw3.lib C:/dependencies/Lua/lib/lua5.1.lib

If "%1" == "compile" goto :compile
If "%1" == "debug" goto :debug
If "%1" == "clean" goto :clean
If "%1" == "run" goto :run
If "%1" == "help" goto :help

:compile
%CC% %CFLAGS% %DEF% %INC% /Fe%BIN% %SRC% %LIBS%
goto :eof

:debug
%DB% %BIN%
goto :eof

:clean
rm *.exe *.obj *.ilk *.pdb
goto :eof

:run
%BIN%
goto :eof

:help
echo To build application, enter into command line: build.bat compile
echo To debug application, enter into command line: build.bat debug
echo To run application, enter into command line: build.bat run
echo To clean application, enter into command line: build.bat clean

:eof
