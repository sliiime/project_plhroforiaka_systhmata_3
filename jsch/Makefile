SRCd = src
BINd = bin
HDRd = hdr
TEMPd = temp

CC = g++

SRC = $(wildcard $(SRCd)/*.cpp)
BIN = $(patsubst $(SRCd)/%.cpp, $(BINd)/%.o, $(SRC) )
TEMP = $(wildcard $(TEMPd)/*.hpp)

INCLUDE = -I $(HDRd) -I $(TEMPd)
FLAGS = -pthread -Wall -g -O3 -march=native

jsch: $(BIN)

$(BIN) : $(BINd)/%.o : $(SRCd)/%.cpp $(TEMP)
	$(CC) -c $(INCLUDE) $(FLAGS) $< -o $@

.PHONY: clean

clean:
	rm $(BINd)/* 