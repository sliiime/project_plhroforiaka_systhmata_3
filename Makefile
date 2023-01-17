# Paths
MODULES = modules
BINARIES = bin
TEMPLATES = templates

INCLUDE = -I include -I jsch/hdr -I jsch/temp -I jsch

JSCH_BIN = $(wildcard jsch/bin/*)

# Compiler
CC = g++

# Executable
EXEC = main

# Optimization
OPFLAGS = -O3 -march=native
# Compile options
CPPFLAGS = -pthread -Wall -g $(INCLUDE) -I $(TEMPLATES) $(OPFLAGS)

# Object files
SRC = $(wildcard $(MODULES)/*.cpp)
MODULE_OBJS = $(patsubst $(MODULES)/%.cpp,$(BINARIES)/%.o,$(SRC))
OBJS = $(EXEC).o $(MODULE_OBJS) 

# Template files
TMPLTS = $(TEMPLATES)/list.hpp $(TEMPLATES)/vector.hpp $(TEMPLATES)/queue.hpp

$(EXEC): $(OBJS) $(TMPLTS) include/config.hpp include/parser.hpp include/relation.hpp JSCH
	$(CC) $(CPPFLAGS) $(JSCH_BIN) $(OBJS) -o $(EXEC)

JSCH: FORCE
	make -C jsch

# https://www.gnu.org/software/make/manual/make.html#Force-Targets

FORCE:


intergration_test: intergration_test.o $(TMPLTS)
	$(CC) intergration_test.o -o intergration_test

js_test: js_test.o $(MODULE_OBJS)
	$(CC) $(CPPFLAGS) $^ -o $@

js_test.o: js_test.cpp
	$(CC) $(CPPFLAGS) -c js_test.cpp  

$(MODULE_OBJS): $(BINARIES)/%.o : $(MODULES)/%.cpp
	mkdir -p $(BINARIES)
	$(CC) $(CPPFLAGS) -c $<  -o $@
  
clean:
	rm -f $(BINARIES)/* $(EXEC) $(EXEC).o intergration_test output.txt 
	make clean -C jsch


ARGS = < input_relations.txt

run: $(EXEC)
	./$(EXEC) $(ARGS)

# Run with gdb
gdb: $(EXEC)
	gdb ./$(EXEC) $(ARGS)

# Run with valgrind
valgrind: $(EXEC)
	valgrind --leak-check=full --track-origins=yes ./$(EXEC) $(ARGS)

save: $(EXEC)
	./$(EXEC) $(ARGS) > output.txt

test: intergration_test
	./intergration_test
