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
CPPFLAGS = -Wall -g -I $(INCLUDE) -I $(TEMPLATES)

# Object files
SRC = $(wildcard $(MODULES)/*.cpp)
MODULE_OBJS = $(patsubst $(MODULES)/%.cpp,$(BINARIES)/%.o,$(SRC))
OBJS = $(EXEC).o $(MODULE_OBJS) 

# Template files
TMPLTS = $(TEMPLATES)/list.hpp $(TEMPLATES)/vector.hpp $(TEMPLATES)/queue.hpp

$(EXEC): $(OBJS) $(TMPLTS) $(INCLUDE)/config.hpp $(INCLUDE)/parser.hpp $(INCLUDE)/relation.hpp
	$(CC) $(OBJS) -o $(EXEC)

acc_test: acc_test.o $(TMPLTS)
	$(CC) acc_test.o -o acc_test

$(MODULE_OBJS): $(BINARIES)/%.o : $(MODULES)/%.cpp
	mkdir -p $(BINARIES)
	$(CC) -c $< $(CPPFLAGS) -o $@
  
clean:
	rm -f $(BINARIES)/* $(EXEC) $(EXEC).o acc_test output.txt

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

test: acc_test
	./acc_test
