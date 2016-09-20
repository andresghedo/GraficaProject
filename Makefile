# Project: progettoCar4

CPP  = g++
CC   = gcc
BIN  = play

OBJ  = main.o car.o mesh.o controller.o
LINKOBJ  = main.o car.o mesh.o controller.o
LIBS = -L/usr/X11R6 -lGL -lGLU -lSDL2_image -lSDL2 -lSDL2_ttf -lm
INCS = -I. -I/usr/X11R6/include
CXXINCS=#
CXXFLAGS = $(CXXINCS)
CFLAGS = $(INCS)
RM = rm -f

all: $(BIN)


clean:
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o $(BIN) $(LIBS)

main.o: main.cpp
	$(CPP) -c main.cpp -o main.o $(CXXFLAGS)

car.o: car.cpp
	$(CPP) -c car.cpp -o car.o $(CXXFLAGS)

controller.o: controller.cpp
	$(CPP) -c controller.cpp -o controller.o $(CXXFLAGS)

mesh.o: mesh.cpp
	$(CPP) -c mesh.cpp -o mesh.o $(CXXFLAGS)
