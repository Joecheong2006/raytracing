CC = g++
CFLAGS := -std=c++11 -Wall -Wextra -Wpedantic -Wstrict-aliasing -g

SOURCE = src src/**
LIBRARY = lib/glad/src lib/stb_image
INCLUDE = -Ilib -Ilib/glad/include -Ilib/stb_image -Ilib/glfw/ -Ilib/glm

LIBOBJ = $(wildcard $(patsubst %, %/*.o, $(LIBRARY))) 
SRC = $(wildcard $(patsubst %, %/*.cpp, $(SOURCE)))
OBJ = $(SRC:%.cpp=%.o)
DEP = $(OBJ:%.o=%.d)

DLL = lib/glfw/glfw3.dll 
LIB = $(LIBOBJ) $(DLL) lib/glfw/libglfw3dll.a -lopengl32
# LIB = $(LIBOBJ) -lglfw -lOpenGL

BIN = bin
PROGRAM = glsl_test
OUT = $(BIN)/$(PROGRAM)

.PHONY: all clean

all: bin app

build:
	@echo "Compiling library"
	@cd lib/stb_image && $(CC) -o stb_image.o -c stb_image.cpp
	@cd lib/glad && $(CC) -Iinclude -o src/glad.o -c src/glad.c
	@echo "Completed"

run:
	@./$(OUT)

debug:
	@gdb -q $(OUT) --eval-command=run --eval-command=exit

clean:
	@rm -rf $(OBJ) $(DEP) $(BIN)
	@echo Cleaned

bin:
	@mkdir -p $(BIN)
	@cp $(DLL) $(BIN)/.


app: bin $(OBJ)
	$(CC) -o $(OUT) $(OBJ) $(LIB) -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -Wl,-Bdynamic
	@echo "[100%] Compilation completed"

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $< -MMD

-include $(DEP)
