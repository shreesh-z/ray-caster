
IDIR = include
ODIR = obj

CC = x86_64-w64-mingw32-g++
COMPILER_FLAGS = -Wall -pedantic -O2 -I $(IDIR)
LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image

_DEPS = helper.h MapObject.h custom_math.h GameMap.h blocks.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = gameLoop.o GameMap.o MapObject.o custom_math.o blocks.o helper.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c $< -o $@ $(COMPILER_FLAGS) $(LINKER_FLAGS)

#EXEC_NAME = rayCast.exe
EXEC_NAME = square_chase.exe

$(EXEC_NAME): $(OBJ)
	$(CC) -o $@ $^ $(COMPILER_FLAGS) $(LINKER_FLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o $(EXEC_NAME)