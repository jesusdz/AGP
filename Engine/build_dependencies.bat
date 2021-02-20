@echo off

set ROOT=%~dp0
set ROOT=%ROOT:~0,-1%

if not exist tmp/ mkdir tmp
pushd tmp

set cl_opts=-nologo -DIMGUI_IMPL_OPENGL_LOADER_GLAD
cl %cl_opts% -c %ROOT%/ThirdParty/glad/include/glad/glad.c             -Fo%ROOT%/tmp/glad.obj
cl %cl_opts% -c %ROOT%/ThirdParty/imgui-docking/imgui.cpp              -Fo%ROOT%/tmp/imgui.obj
cl %cl_opts% -c %ROOT%/ThirdParty/imgui-docking/imgui_demo.cpp         -Fo%ROOT%/tmp/imgui_demo.obj
cl %cl_opts% -c %ROOT%/ThirdParty/imgui-docking/imgui_draw.cpp         -Fo%ROOT%/tmp/imgui_draw.obj
cl %cl_opts% -c %ROOT%/ThirdParty/imgui-docking/imgui_impl_glfw.cpp    -Fo%ROOT%/tmp/imgui_impl_glfw.obj    -I %ROOT%/ThirdParty/glfw/include
cl %cl_opts% -c %ROOT%/ThirdParty/imgui-docking/imgui_impl_opengl3.cpp -Fo%ROOT%/tmp/imgui_impl_opengl3.obj -I %ROOT%/ThirdParty/glad/include
cl %cl_opts% -c %ROOT%/ThirdParty/imgui-docking/imgui_widgets.cpp      -Fo%ROOT%/tmp/imgui_widgets.obj
cl %cl_opts% -c %ROOT%/ThirdParty/imgui-docking/imgui_tables.cpp       -Fo%ROOT%/tmp/imgui_tables.obj
cl %cl_opts% -c %ROOT%/ThirdParty/stb/stb.cpp                          -Fo%ROOT%/tmp/stb.obj

set link_opts=-nologo
lib %link_opts% -out:dependencies.lib glad.obj imgui.obj imgui_demo.obj imgui_draw.obj imgui_impl_glfw.obj imgui_impl_opengl3.obj imgui_widgets.obj imgui_tables.obj stb.obj

popd
