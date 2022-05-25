.SUFFIX:

CXX = g++
CFLAGS = -std=c++11 -w

ifeq ($(OS), Windows_NT)
	OBJ = *.o
	LIB = "C:\Program Files (x86)\CodeBlocks\SDL-1.2.15\lib"
	HEADER = "C:\Program Files (x86)\CodeBlocks\SDL-1.2.15\include"
	SDL = -lmingw32 -lSDLmain -lSDL
	CLEAN_SCREEN = cls
	DEL_OBJ = del *.o
	DEL_BIN = del *.exe
else
	CXX = g++-10
	OBJ = obj/*
	LIB = .
	HEADER = /usr/include/SDL/
	SDL = -lSDL -lSDL_image -lopnegl32
	MOVE = mkdir -p obj/ && mv *.o obj/
	DEL_OBJ = rm -f *.o
	DEL_BIN = rm -f gb
endif


SRC = *.cpp

all:
	@$(CLEAN_SCREEN)

	@echo ... Compiling project, Please wait a moment ...
	@echo ***********************************************
	$(CXX) $(CFLAGS) -I$(HEADER) -L$(LIB) $(SRC) -o gb $(SDL)
	@echo ***********************************************
	@$(CLEAN_SCREEN)

Debug:
	@$(CLEAN_SCREEN)

	@echo ... Compiling project, Please wait a moment ...
	@echo ***********************************************
	$(CXX) $(CFLAGS) -I$(HEADER) -L$(LIB) $(SRC) -g -o gb $(SDL)
	@echo ***********************************************
	@$(CLEAN_SCREEN)

allrun:
	@$(CLEAN_SCREEN)

	@echo ... Compiling project, Please wait a moment ...
	@echo ***********************************************
	$(CXX) $(CFLAGS) -I$(HEADER) -L$(LIB) -o gb $(SRC) $(SDL)
	@echo ***********************************************
	@$(CLEAN_SCREEN)
	@gb.exe

link : obj
	@echo ... Linking files ...
	@echo *********************
	$(CXX) $(OBJ) -L$(LIB) $(SDL) -o gb
	@echo *********************
	@$(CLEAN_SCREEN)


obj : $(SRC)
	@$(CLEAN_SCREEN)

	@echo ... Compiling, Please wait a moment ...
	@echo ***************************************
	$(CXX) $(CFLAGS) -I$(HEADER) -c $?
	@echo ***************************************

clean :
	@$(DEL_OBJ)
	@$(DEL_BIN)
