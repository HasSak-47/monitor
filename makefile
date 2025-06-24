OUT := monitor

SRC_DIR := src
BUILD_DIR := build
INCLUDE_DIR := include

LUI_DIR := lui
LUI_LIB := $(LUI_DIR)/lib/libtui.a
LUI_INCLUDE := $(LUI_DIR)/include

C := ccache g++
AR := ar
CFLAGS := -g -pg -O2 -I"$(INCLUDE_DIR)" -I"$(LUI_INCLUDE)" -std=c++23 -MMD -MP -c
LDFLAGS := -L"$(LUI_DIR)/lib" -llui -llua
LDOUT := -o "$(OUT)"

# Gather all .cpp source files
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

-include $(OBJS:.o=.d)

.PHONY: build all compile clean run valgrind

build: $(LUI_LIB) all

# Default build
all: $(OUT)


compile: all

# Final binary links main + lib
$(OUT): $(OBJS) $(LUI_LIB)
	$(C) $(OBJS) $(LDOUT) $(LDFLAGS)

# Static lib from lui subproject
$(LUI_LIB):
	$(MAKE) -C $(LUI_DIR) libonly

# Compile .cpp to .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p "$(dir $@)"
	$(C) $(CFLAGS) -o $@ $<

run: $(build) $(OUT)
	./$(OUT)

valgrind: $(OUT)
	valgrind ./$(OUT)

clean:
	@rm -rf "$(BUILD_DIR)" "$(OUT)"
	$(MAKE) -C $(LUI_DIR) clean
