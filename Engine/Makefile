.PHONY: engine

ENGINE_SOURCES = ./Code/engine.cpp \
								 ./Code/platform.cpp \
								 ./ThirdParty/glad/include/glad/glad.c \
								 ./ThirdParty/imgui-docking/imgui.cpp \
								 ./ThirdParty/imgui-docking/imgui_demo.cpp \
								 ./ThirdParty/imgui-docking/imgui_draw.cpp \
								 ./ThirdParty/imgui-docking/imgui_impl_glfw.cpp \
								 ./ThirdParty/imgui-docking/imgui_impl_opengl3.cpp \
								 ./ThirdParty/imgui-docking/imgui_widgets.cpp \
								 ./ThirdParty/imgui-docking/imgui_tables.cpp \
								 ./ThirdParty/stb/stb.cpp

CODE_DIR=./Code
OUTPUT_DIR=./WorkingDir
COMPILER_SWITCHES=-g
INCLUDE_DIRS= -I ./ThirdParty/glfw/include \
							-I ./ThirdParty/imgui-docking/ \
							-I ./ThirdParty/glad/include \
							-I ./ThirdParty/glm/include \
							-I ./ThirdParty/stb
LIBRARY_DIRS= -L ./ThirdParty/glfw/lib-osx
LIBS= -lglfw3
OSX_DEPS=-framework Cocoa -framework OpenGL -framework IOKit

engine:
	g++ ${COMPILER_SWITCHES} ${INCLUDE_DIRS} ${ENGINE_SOURCES} -o engine ${LIBRARY_DIRS} ${LIBS} ${OSX_DEPS}

