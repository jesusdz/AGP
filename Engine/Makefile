.PHONY: engine clean

ENGINE_SOURCES = ./Code/engine.cpp \
								 ./Code/platform.cpp

OUTPUT_DIR=./WorkingDir
COMPILER_SWITCHES=-g
INCLUDE_DIRS= -I ./ThirdParty/glfw/include \
							-I ./ThirdParty/imgui-docking/ \
							-I ./ThirdParty/glad/include \
							-I ./ThirdParty/glm/include \
							-I ./ThirdParty/stb
LIBRARY_DIRS= -L ./ThirdParty/glfw/lib-osx -L ./tmp
LIBS= -lglfw3 -ldeps
OSX_DEPS=-framework Cocoa -framework OpenGL -framework IOKit

engine: tmp/libdeps.a
	g++ ${COMPILER_SWITCHES} ${INCLUDE_DIRS} ${ENGINE_SOURCES} -o ${OUTPUT_DIR}/engine ${LIBRARY_DIRS} ${LIBS} ${OSX_DEPS}

tmp/libdeps.a:
	mkdir tmp
	gcc -c ./ThirdParty/glad/include/glad/glad.c             -o ./tmp/glad.o
	g++ -c ./ThirdParty/imgui-docking/imgui.cpp              -o ./tmp/imgui.o
	g++ -c ./ThirdParty/imgui-docking/imgui_demo.cpp         -o ./tmp/imgui_demo.o
	g++ -c ./ThirdParty/imgui-docking/imgui_draw.cpp         -o ./tmp/imgui_draw.o
	g++ -c ./ThirdParty/imgui-docking/imgui_impl_glfw.cpp    -o ./tmp/imgui_impl_glfw.o    -I ./ThirdParty/glfw/include
	g++ -c ./ThirdParty/imgui-docking/imgui_impl_opengl3.cpp -o ./tmp/imgui_impl_opengl3.o -I ./ThirdParty/glad/include
	g++ -c ./ThirdParty/imgui-docking/imgui_widgets.cpp      -o ./tmp/imgui_widgets.o
	g++ -c ./ThirdParty/imgui-docking/imgui_tables.cpp       -o ./tmp/imgui_tables.o
	g++ -c ./ThirdParty/stb/stb.cpp                          -o ./tmp/stb.o
	ar rvs $@ tmp/glad.o tmp/imgui.o tmp/imgui_demo.o tmp/imgui_draw.o tmp/imgui_impl_glfw.o tmp/imgui_impl_opengl3.o tmp/imgui_widgets.o tmp/imgui_tables.o tmp/stb.o

clean:
	rm -rf tmp
	rm -rf ${OUTPUT_DIR}/engine
	rm -rf ${OUTPUT_DIR}/engine.dSYM

