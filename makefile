SRC_DIR := src
OBJ_DIR := build
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o,$(SRCS)) $(OUT_RUST_LIB)

OUT := monitor

C := g++
CFLAGS := -g -shared -I include -c -Wall -Werror

LDFLAGS := -o $(OUT) -llua

all: compile

compile: $(OUT)

$(OUT): $(OBJS)
	$(C) $(OBJS) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(C) $(CFLAGS) $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

run: compile
	./$(OUT)

clean:
	rm $(OBJS)

valgrind: compuie
	valgrind ./$(OUT)


.PHONY: all clean cmds source valgrind

