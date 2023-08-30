@echo off

call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"


SET includes=/I"src" /I"%VULKAN_SDK%/Include"
SET links=/link /LIBPATH:%VULKAN_SDK%/Lib vulkan-1.lib user32.lib
SET defines=/D DEBUG


echo "Building main..."

cl /EHsc /Z7 /Fe"main" %includes% %defines% src/platform/win32_platform.cpp %links%