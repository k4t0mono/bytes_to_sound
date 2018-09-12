# I'm just a Makefile

EXE = bts
CC = clang

SRC_DIR = src
OBJ_DIR = obj

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CPPFLAGS += -I include
CXXFLAGS += -Wall -std=c11
LDFLAGS += -L lib
LDLIBS +=

.PHONY: all clean

all: $(EXE).out

$(EXE).out: $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ) $(EXE).out
