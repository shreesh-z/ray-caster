#OBJS = gameLoop.cpp
OBJS = gameLoop.cpp GameMap.cpp MapObject.cpp custom_math.cpp blocks.cpp
CC = x86_64-w64-mingw32-g++
COMPILER_FLAGS = -Wall -pedantic -O2
LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image
OBJ_NAME = rayCast.exe
all : $(OBJS)
		$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)