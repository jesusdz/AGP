@echo off

set ROOT=%~dp0
set ROOT=%ROOT:~0,-1%

if not exist tmp/ mkdir tmp
pushd tmp

set switches=-FC -GR- -EHa- -nologo -Zi
set include_dirs=-I %ROOT%/ThirdParty/glad/include -I %ROOT%/ThirdParty/imgui-docking -I %ROOT%/ThirdParty/stb -I %ROOT%/ThirdParty/glfw/include -I %ROOT%/ThirdParty/glm/include -I %ROOT%/ThirdParty/Assimp\include
set source_files=%ROOT%/Code/platform.cpp %ROOT%/Code/engine.cpp
set libraries=%ROOT%/ThirdParty\glfw\lib-vc2019\glfw3dll.lib %ROOT%/tmp/dependencies.lib %ROOT%/ThirdParty/Assimp/lib/windows/assimp.lib gdi32.lib
cl %switches% %include_dirs% %source_files% %libraries% /link /MACHINE:x64 /out:engine.exe

REM --- Copy executable and dlls
copy engine.exe %ROOT%\WorkingDir\engine.exe
if not exist %ROOT%\WorkingDir\assimp.dll copy %ROOT%\ThirdParty\Assimp\lib\windows\assimp.dll %ROOT%\WorkingDir
if not exist %ROOT%\WorkingDir\glfw3.cll copy %ROOT%\ThirdParty\glfw\lib-vc2019\glfw3.dll %ROOT%\WorkingDir

popd
