# Paths
MODULES = modules
INCLUDE = include
BINARIES = bin
TEMPLATES = templates

# Compiler
CC = g++

# Executable
EXEC = main

# Compile options
CPPFLAGS = -pthread -Wall -g -I $(INCLUDE) -I $(TEMPLATES)

# Object files
SRC = $(wildcard $(MODULES)/*.cpp)
MODULE_OBJS = $(patsubst $(MODULES)/%.cpp,$(BINARIES)/%.o,$(SRC))
OBJS = $(EXEC).o $(MODULE_OBJS) 

# Template files
TMPLTS = $(TEMPLATES)/list.hpp $(TEMPLATES)/vector.hpp $(TEMPLATES)/queue.hpp

$(EXEC): $(OBJS) $(TMPLTS) $(INCLUDE)/config.hpp $(INCLUDE)/parser.hpp $(INCLUDE)/relation.hpp
	$(CC) $(CPPFLAGS) $(OBJS) -o $(EXEC)

intergration_test: intergration_test.o $(TMPLTS)
	$(CC) intergration_test.o -o intergration_test

$(MODULE_OBJS): $(BINARIES)/%.o : $(MODULES)/%.cpp
	mkdir -p $(BINARIES)
	$(CC) -c $< $(CPPFLAGS) -o $@
  
clean:
	rm -f $(BINARIES)/* $(EXEC) $(EXEC).o intergration_test output.txt

print:
	echo $(MODULE_OBJS)

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
