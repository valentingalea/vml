@echo off

set CC=cl

set DB=devenv

set CFLAGS=-Zi /EHsc /std:c++latest /DEBUG:FULL

set DEF=/DGLM_ENABLE_EXPERIMENTAL /DUNITY_BUILD /D_MBCS

REM Download GLFW from https://www.glfw.org/download.html
set GLFW_DIR=C:/dependencies/glfw-3.2.1.bin.WIN64
REM Download GLM from https://github.com/g-truc/glm/releases/tag/0.9.9.5
set GLM_DIR=C:/dependencies/glm
REM Download Vulkan from https://www.lunarg.com/vulkan-sdk/
REM NOTE: If progarm crashes because of a Vulkan function call, and no validation errors appear,
REM run "regedit" in console and check if under HKEY_LOCAL_MACHINE -> SOFTWARE -> Khronos -> Vulkan -> ExplicitLayers (and also ImplicitLayers)
REM that there are paths to json files like VkLayer_standard_validation.json
set VULKAN_SDK_DIR=C:/VulkanSDK/1.1.108.0

set VML_INC_DIR=/I ../../

REM May have to change the glfw-3.2.1.bin.WIN64 to the correct directory (dependending of GLFW version)
set GLFW_INC_DIR=/I %GLFW_DIR%/include
set GLM_INC_DIR=/I %GLM_DIR%
set VULKAN_INC_DIR=/I %VULKAN_SDK_DIR%/Include

set INC=%GLM_INC_DIR% %VULKAN_INC_DIR% %GLFW_INC_DIR% %VML_INC_DIR%

set BIN=DebuggingTest.exe

set SRC=core.cpp

set LIBS=winmm.lib user32.lib User32.lib Gdi32.lib Shell32.lib kernel32.lib gdi32.lib msvcrt.lib msvcmrt.lib %VULKAN_SDK_DIR%/Lib/vulkan-1.lib %GLFW_DIR%/lib-vc2015/glfw3.lib

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
