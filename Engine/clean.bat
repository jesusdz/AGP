@echo off

set ROOT=%~dp0

if exist %ROOT%tmp rd /s /q %ROOT%tmp
if exist %ROOT%WorkingDir\engine.exe del %ROOT%WorkingDir\engine.exe
if exist %ROOT%WorkingDir\assimp.dll del %ROOT%WorkingDir\assimp.dll
if exist %ROOT%WorkingDir\glfw3.dll del %ROOT%WorkingDir\glfw3.dll

