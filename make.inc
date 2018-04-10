# Set paths & use `bash`
SHELL := bash
TARGET ?= debug
BUILD_PATH = build/$(TARGET)
OUT = $(BUILD_PATH)/out

# Utilities
DEFLATE = $(SCREEPS_PATH)/deflate.js
STRIP_MAP = $(SCREEPS_PATH)/strip-map.js

# Object paths
SCREEPS_PATH ?= screeps
SCREEPS_SRCS = creep.cc game.cc memory.cc path-finder.cc position.cc resource.cc room.cc structure.cc terrain.cc
SCREEPS_RUNTIME = array.js creep.js game.js main.js object.js position.js resource.js room.js string.js structure.js util.js vector.js

# Source map for debug builds
ifeq ($(TARGET), debug)
SOURCE_MAP_RUNTIME += source-map.js inflate.js inflate-wasm.wasm
endif

.SECONDEXPANSION:
SRCS += $(addprefix $(SCREEPS_PATH)/src/,$(SCREEPS_SRCS))
OBJS = $(addprefix build/$(TARGET)/,$(patsubst %.cc,%.bc,$(SRCS)))

# Default compiler flags
CXXFLAGS += \
	-std=c++1z \
	-Wall -Wextra -Wno-invalid-offsetof \
	-isystem $(SCREEPS_PATH)/include \
	-I.
EMFLAGS += \
	-s STRICT=1 -s MODULARIZE=1 -s NO_FILESYSTEM=1 -s BINARYEN_ASYNC_COMPILATION=0 \
	-s DISABLE_EXCEPTION_CATCHING=0 \
	-s EXPORTED_FUNCTIONS="['__Z4loopv']" \
	-s ALLOW_MEMORY_GROWTH=0 -s TOTAL_MEMORY=201326592

# Additional flags for debug or release
ifeq ($(TARGET), debug)
CXXFLAGS += -g4 -O1 -ftrapv
EMFLAGS += -s ASSERTIONS=1 -s DEMANGLE_SUPPORT=1
else
ifeq ($(TARGET), release)
CXXFLAGS += -O3
EMFLAGS += --llvm-lto 3
else
$(error Invalid target. Must be "debug" or "release" $(TARGET))
endif
endif

# Default targets
asmjs: runtime $(addprefix $(OUT)/,$(SOURCE_MAP_RUNTIME))
wasm: runtime $(OUT)/$(MODULE_NAME).js $(OUT)/$(MODULE_NAME)-wasm.wasm
runtime: $(addprefix $(OUT)/,$(SCREEPS_RUNTIME))

# Compress asmjs in debug builds, also include source map
ifeq ($(TARGET), debug)
asmjs: $(OUT)/$(MODULE_NAME)-asmjs.wasm $(OUT)/$(MODULE_NAME)-map.wasm
else
asmjs: $(OUT)/$(MODULE_NAME)-asmjs.js
endif

# Include header dependencies
include $(shell find build -name '*.d' 2>/dev/null)
%.h: %.h

# Directory creation
build/.:
	mkdir -p $@

build%/.:
	mkdir -p $@

# Bytecode targets
$(BUILD_PATH)/%.bc: %.cc | $$(@D)/.
	$(CXX) -c -o $@ $< $(CXXFLAGS) -MD

# asmjs
$(BUILD_PATH)/module-asmjs.js: $$(OBJS) | $$(@D)/.
	$(CXX) -o $@ $^ $(CXXFLAGS) $(EMFLAGS) --memory-init-file 0
$(BUILD_PATH)/module-asmjs.js.map: $(BUILD_PATH)/module-asmjs.js ;

$(OUT)/$(MODULE_NAME)-asmjs.js: $(BUILD_PATH)/module-asmjs.js | $$(@D)/.
	@rm $(OUT)/$(MODULE_NAME)-asmjs.wasm 2>/dev/null || true
	@rm $(OUT)/$(MODULE_NAME)-map.wasm 2>/dev/null || true
	@rm $(OUT)/$(MODULE_NAME).js 2>/dev/null || true
	@rm $(OUT)/$(MODULE_NAME)-wasm.wasm 2>/dev/null || true
	cp $< $@

$(OUT)/$(MODULE_NAME)-asmjs.wasm: $(BUILD_PATH)/module-asmjs.js | $$(@D)/.
	@rm $(OUT)/$(MODULE_NAME)-asmjs.js 2>/dev/null || true
	@rm $(OUT)/$(MODULE_NAME).js 2>/dev/null || true
	@rm $(OUT)/$(MODULE_NAME)-wasm.wasm 2>/dev/null || true
	cat $< | $(DEFLATE) > $@

$(OUT)/$(MODULE_NAME)-map.wasm: $(BUILD_PATH)/module-asmjs.js.map | $$(@D)/.
	cat $< | $(STRIP_MAP) | $(DEFLATE) > $@

# wasm
$(BUILD_PATH)/module-wasm.js: $$(OBJS) | $$(@D)/.
	$(CXX) -o $@ $^ $(CXXFLAGS) $(EMFLAGS) -s WASM=1
$(BUILD_PATH)/module-wasm.wasm: $(BUILD_PATH)/module-wasm.js ;

$(OUT)/$(MODULE_NAME)-wasm.wasm: $(BUILD_PATH)/module-wasm.wasm | $$(@D)/.
	@rm $(OUT)/inflate.js 2>/dev/null || true
	@rm $(OUT)/inflate-wasm.wasm 2>/dev/null || true
	@rm $(OUT)/$(MODULE_NAME)-asmjs.js 2>/dev/null || true
	@rm $(OUT)/$(MODULE_NAME)-asmjs.wasm 2>/dev/null || true
	@rm $(OUT)/$(MODULE_NAME)-map.wasm 2>/dev/null || true
	cp $< $@

$(OUT)/$(MODULE_NAME).js: $(BUILD_PATH)/module-wasm.js | $$(@D)/.
	cp $< $@

# Copy JS files to build/{}/
$(OUT)/%.js: $(SCREEPS_PATH)/js/%.js | $$(@D)/.
	cp $< $@

# Needed for inflate
$(OUT)/inflate-wasm.wasm: $(SCREEPS_PATH)/js/inflate-wasm.wasm | $$(@D)/.
	cp $< $@

# Flags
.PRECIOUS: build/. build%/. $(BUILD_PATH)/%.bc

.PHONY: clean asmjs wasm runtime

clean:
	$(RM) -rf build
