# Copyright 2020 Grama Nicolae

.PHONY: gitignore clean memory beauty run
.SILENT: beauty clean memory gitignore

# Compilation variables
CC = g++
CFLAGS = -Wno-unused-parameter -Wall -Wextra -pedantic -Wno-unknown-pragmas -g -O3 -std=c++14
EXE = router
SRC = $(wildcard */*.cpp)
OBJ = $(SRC:.cpp=.o)

# Compiles the program
build: $(OBJ)
	$(info Compiling code...)
	@$(CC) -o $(EXE) $^ $(CFLAGS) ||:
	$(info Compilation successfull)
	-@rm -f *.o ||:
	@$(MAKE) -s gitignore ||:// Copyright Grama Nicolae 2020

%.o: %.cpp
	$(CC) -o $@ -c $< $(CFLAGS) 
	
# Executes the binary
run: clean build
	./$(EXE)

# Deletes the binary and object files
clean:
	rm -f $(EXE) $(OBJ) CppRouter.zip
	echo "Deleted the binary and object files"

# Automatic coding style, in my personal style
beauty:
	clang-format -i -style=file */*.cpp
	clang-format -i -style=file */*.hpp

# Starts the virtual network:
network:
	-@sudo fuser -k 6653/tcp
	sudo python3 topo.py

# Checks the memory for leaks
MFLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes
memory:clean build
	valgrind $(MFLAGS) ./$(EXE)

# Adds and updates gitignore rules
gitignore:
	@echo "$(EXE)" > .gitignore ||:
	@echo "src/*.o" >> .gitignore ||:
	@echo ".vscode*" >> .gitignore ||:	
	@echo "__pycache__" >> .gitignore ||:	
	@echo "hosts_output" >> .gitignore ||:	
	@echo "router_err.txt" >> .gitignore ||:	
	@echo "router_out.txt" >> .gitignore ||:
	@echo "CppRouter.zip" >> .gitignore ||:		
	echo "Updated .gitignore"
	
# Creates an archive of the project
pack: clean
	zip -FSr CppRouter.zip Readme.md src/* Makefile

