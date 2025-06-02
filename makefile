SRC_DIR := src
OBJ_DIR := build
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o,$(SRCS)) $(OUT_RUST_LIB)

OUT := monitor

C := g++
CFLAGS := -g -shared -I include -c -Wall -std=c++23 -MMD -MP

LDFLAGS := -o $(OUT) -llua

-include $(OBJS:.o=.d)

all: compile

compile: $(OUT)

$(OUT): $(OBJS)
	$(C) $(OBJS) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(C) $(CFLAGS) $< -o $@

$(OBJ_DIR)/%:
	@mkdir -p $(dir $@)

run: compile
	./$(OUT)

clean:
	@for f in $(OBJS); do \
		echo "Removing $$f"; \
		rm -rf $$f; \
	done

valgrind: compuie
	valgrind ./$(OUT)


.PHONY: all clean cmds source valgrind

