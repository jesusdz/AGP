.PHONY: engine clean

GFX_API=OGL
#GFX_API=METAL

ENGINE_SOURCES = ./Code/engine.cpp \
				 ./Code/platform.cpp

ifeq ($(GFX_API),OGL)
ENGINE_SOURCES+= ./Code/ogl_engine.cpp
else
ENGINE_SOURCES+= ./Code/metal_engine.mm
endif

OUTPUT_DIR=./WorkingDir

COMPILER_SWITCHES=-g -std=c++11

INCLUDE_DIRS= -I ./ThirdParty/glfw/include \
			  -I ./ThirdParty/imgui-docking/ \
			  -I ./ThirdParty/glad/include \
			  -I ./ThirdParty/glm/include \
			  -I ./ThirdParty/stb \
			  -I ./ThirdParty/Assimp/include

LIBRARY_DIRS= -L ./ThirdParty/glfw/lib-osx \
			  -L ./ThirdParty/Assimp/lib/osx \
			  -L ./tmp

LIBS= -lglfw3 -ldeps -lassimp

ifeq ($(GFX_API),OGL)
DEFINITIONS=-DUSE_GFX_API_OPENGL=1
OSX_DEPS =-framework Cocoa -framework OpenGL -framework IOKit
else
DEFINITIONS=-DUSE_GFX_API_METAL=1
OSX_DEPS =-framework IOKit -framework AppKit -framework Metal -framework QuartzCore
endif

engine: tmp/libdeps.a
	g++ ${COMPILER_SWITCHES} ${DEFINITIONS} ${INCLUDE_DIRS} ${ENGINE_SOURCES} -o ${OUTPUT_DIR}/engine ${LIBRARY_DIRS} ${LIBS} ${OSX_DEPS}

tmp/libdeps.a:
	mkdir -p tmp
	gcc -c -g ./ThirdParty/glad/include/glad/glad.c             -o ./tmp/glad.o
	g++ -c -g ./ThirdParty/imgui-docking/imgui.cpp              -o ./tmp/imgui.o
	g++ -c -g ./ThirdParty/imgui-docking/imgui_demo.cpp         -o ./tmp/imgui_demo.o
	g++ -c -g ./ThirdParty/imgui-docking/imgui_draw.cpp         -o ./tmp/imgui_draw.o
	g++ -c -g ./ThirdParty/imgui-docking/imgui_impl_glfw.cpp    -o ./tmp/imgui_impl_glfw.o    -I ./ThirdParty/glfw/include
	g++ -c -g ./ThirdParty/imgui-docking/imgui_widgets.cpp      -o ./tmp/imgui_widgets.o
	g++ -c -g ./ThirdParty/imgui-docking/imgui_tables.cpp       -o ./tmp/imgui_tables.o
	g++ -c -g ./ThirdParty/stb/stb.cpp                          -o ./tmp/stb.o
ifeq ($(GFX_API),OGL)
	g++ -c -g ./ThirdParty/imgui-docking/imgui_impl_opengl3.cpp -o ./tmp/imgui_impl_opengl3.o -I ./ThirdParty/glad/include
	ar rvs $@ tmp/glad.o tmp/imgui.o tmp/imgui_demo.o tmp/imgui_draw.o tmp/imgui_impl_glfw.o tmp/imgui_impl_opengl3.o tmp/imgui_widgets.o tmp/imgui_tables.o tmp/stb.o
else
	g++ -c -g ./ThirdParty/imgui-docking/imgui_impl_metal.mm    -o ./tmp/imgui_impl_metal.o   -fobjc-weak
	ar rvs $@ tmp/glad.o tmp/imgui.o tmp/imgui_demo.o tmp/imgui_draw.o tmp/imgui_impl_glfw.o tmp/imgui_impl_metal.o tmp/imgui_widgets.o tmp/imgui_tables.o tmp/stb.o
endif

clean:
	rm -rf tmp
	rm -rf ${OUTPUT_DIR}/engine
	rm -rf ${OUTPUT_DIR}/engine.dSYM

