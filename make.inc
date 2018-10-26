# Set paths & use `bash`
SHELL := bash
TARGET ?= debug
BUILD_PATH = build/$(TARGET)
SCREEPS_PATH ?= screeps
ASMJS = $(BUILD_PATH)/asmjs
WASM = $(BUILD_PATH)/wasm

# Utilities
DEFLATE = $(SCREEPS_PATH)/deflate.js
MAKE_ARCHIVE = $(SCREEPS_PATH)/make-archive.js
MERGE_SYMBOLS = $(SCREEPS_PATH)/merge-symbols.js
STRIP_MAP = $(SCREEPS_PATH)/strip-map.js
NOTHING =
SPACE = $(NOTHING) $(NOTHING)
COMMA = ,

# Source map for debug builds
ifeq ($(TARGET), debug)
SOURCE_MAP_RUNTIME += source-map.js inflate.js inflate-wasm.wasm
endif

# Default compiler flags
CXXFLAGS += \
	-std=c++1z \
	-Wall -Wextra -Wno-invalid-offsetof \
	-isystem $(SCREEPS_PATH)/include \
	-I.
EMFLAGS += -s STRICT=1 -s DISABLE_EXCEPTION_CATCHING=0 -s ABORTING_MALLOC=0
MAIN_EMFLAGS += -s NO_FILESYSTEM=1 -s ALLOW_MEMORY_GROWTH=0 -s TOTAL_MEMORY=201326592

# Additional flags for debug or release
ifeq ($(TARGET), debug)
CXXFLAGS += -g4 -O2 -ftrapv
EMFLAGS += -s ASSERTIONS=1
MAIN_EMFLAGS += -s DEMANGLE_SUPPORT=1
DYNAMIC_LINK = 1
else
ifeq ($(TARGET), release)
CXXFLAGS += -O3
EMFLAGS += --llvm-lto 3
else
$(error Invalid target. Must be "debug" or "release" $(TARGET))
endif
endif
ASMJS_EMFLAGS += $(MAIN_EMFLAGS)
WASM_EMFLAGS += $(MAIN_EMFLAGS) -s BINARYEN_ASYNC_COMPILATION=0

# Object paths
.SECONDEXPANSION:
SCREEPS_SRCS = creep.cc game.cc memory.cc path-finder.cc position.cc resource.cc room.cc structure.cc terrain.cc visual.cc
SCREEPS_RUNTIME = array.js main.js object.js position.js string.js util.js vector.js

# Create objects from sources
SCREEPS_FULL_SRCS = $(addprefix $(SCREEPS_PATH)/src/,$(SCREEPS_SRCS))
ifeq ($(DYNAMIC_LINK), 1)
MAIN_SRCS = $(SCREEPS_FULL_SRCS)
DYLIB_SRCS = $(SRCS)
ASMJS_EMFLAGS += -s MAIN_MODULE=2 \
	-s EXPORTED_FUNCTIONS=$$(cat $(BUILD_PATH)/symbols.json) \
	-s DEFAULT_LIBRARY_FUNCS_TO_INCLUDE=[$$(cat $(SCREEPS_PATH)/emscripten_symbols | awk '{print "\""$$1"\""}' | tr '\n' ,)] \
	-s EXTRA_EXPORTED_RUNTIME_METHODS="['loadDynamicLibrary']"
else
MAIN_SRCS = $(SCREEPS_FULL_SRCS) $(SRCS)
ASMJS_EMFLAGS = -s MODULARIZE=1
endif
WASM_EMFLAGS += -s MODULARIZE=1 -s WASM=1 -s EXPORTED_FUNCTIONS="['__Z4loopv']"
OBJS = $(addprefix $(BUILD_PATH)/,$(patsubst %.cc,%.bc,$(subst /,!,$(MAIN_SRCS))))
DYLIB_OBJS = $(addprefix $(BUILD_PATH)/,$(patsubst %.cc,%.bc,$(subst /,!,$(DYLIB_SRCS))))
ASMJS_DYLIBS = $(patsubst $(BUILD_PATH)/%.bc,$(BUILD_PATH)/dylib-%.js,$(DYLIB_OBJS))
ASMJS_OBJ_SYMBOLS = $(patsubst %.bc,%.nm,$(OBJS))
ASMJS_DYLIB_SYMBOLS = $(patsubst %.bc,%.nm,$(DYLIB_OBJS))

# Default targets
asmjs: runtime-asmjs $(addprefix $(ASMJS)/,$(SOURCE_MAP_RUNTIME))
wasm: runtime-wasm $(WASM)/$(MODULE_NAME).js $(WASM)/$(MODULE_NAME)-wasm.wasm
runtime-asmjs: $(addprefix $(ASMJS)/,$(SCREEPS_RUNTIME))
runtime-wasm: $(addprefix $(WASM)/,$(SCREEPS_RUNTIME))
.PHONY: asmjs wasm runtime-asmjs runtime-wasm

# Compress asmjs in debug builds, also include source map
ifeq ($(TARGET), debug)
asmjs: $(ASMJS)/$(MODULE_NAME)-asmjs.wasm
else
asmjs: $(ASMJS)/$(MODULE_NAME)-asmjs.js
endif

# Include header dependencies
include $(shell find build -name '*.d' 2>/dev/null)
%.h: %.h

# Directory creation
build/.:
	mkdir -p $@
build%/.:
	mkdir -p $@
.PRECIOUS: build/. build%/.

# Bytecode targets
$(BUILD_PATH)/%.bc: $$(subst !,/,$$*.cc) | $$(@D)/.
	$(CXX) -c -o $@ $< $(CXXFLAGS) -MD
.PRECIOUS: $(BUILD_PATH)/%.bc

# asmjs
$(BUILD_PATH)/_main.js: $$(OBJS) $(BUILD_PATH)/symbols.json | $$(@D)/.
	$(CXX) -o $@ $(OBJS) $(CXXFLAGS) $(EMFLAGS) $(ASMJS_EMFLAGS) --memory-init-file 0

dylib-%.js: %.bc
	$(CXX) -o $@ $^ $(CXXFLAGS) $(EMFLAGS) -s SIDE_MODULE=1 --memory-init-file 0
.PRECIOUS: dylib-%.js

$(BUILD_PATH)/%.js.map: $(BUILD_PATH)/%.js ;

$(ASMJS)/$(MODULE_NAME)-asmjs.js: $(BUILD_PATH)/_main.js | $$(@D)/.
	cp $< $@

$(ASMJS)/$(MODULE_NAME)-asmjs.wasm: $(BUILD_PATH)/_main.js $$(ASMJS_DYLIBS) | $$(@D)/.
	$(MAKE_ARCHIVE) $^ | $(DEFLATE) > $@

$(ASMJS)/dylib-%-map.wasm: $(BUILD_PATH)/dylib-%.js.map | $$(@D)/.
	cat $< | $(STRIP_MAP) | $(DEFLATE) > $@

%.nm: %.bc
	$(LLVM_NM) $^ > $@
.PRECIOUS: %.nm

$(BUILD_PATH)/symbols.json: $(BUILD_PATH)/symbols.json.marker
$(BUILD_PATH)/symbols.json.marker: $$(ASMJS_DYLIB_SYMBOLS) $$(ASMJS_OBJ_SYMBOLS) | $$(@D)/.
	touch $@
	$(MERGE_SYMBOLS) $(BUILD_PATH)/symbols.json $(ASMJS_OBJ_SYMBOLS) -- $(ASMJS_DYLIB_SYMBOLS)

# wasm
$(BUILD_PATH)/_main-wasm.js: $$(OBJS) $$(DYLIB_OBJS) | $$(@D)/.
	$(CXX) -o $@ $^ $(CXXFLAGS) $(EMFLAGS) $(WASM_EMFLAGS)
$(BUILD_PATH)/_main-wasm.wasm: $(BUILD_PATH)/_main-wasm.js ;

$(WASM)/$(MODULE_NAME)-wasm.wasm: $(BUILD_PATH)/_main-wasm.wasm | $$(@D)/.
	cp $< $@

$(WASM)/$(MODULE_NAME).js: $(BUILD_PATH)/_main-wasm.js | $$(@D)/.
	cp $< $@

# Copy screeps runtime JS files to build/{}/
$(ASMJS)/%.js $(WASM)/%.js: $(SCREEPS_PATH)/js/%.js | $$(@D)/.
	cp $< $@

# inflate.js wasm backend
$(ASMJS)/inflate-wasm.wasm $(WASM)/inflate-wasm.wasm: $(SCREEPS_PATH)/js/inflate-wasm.wasm | $$(@D)/.
	cp $< $@

# Cleanups
clean:
	$(RM) -rf build

clean-out:
	$(RM) -rf $(ASMJS) $(WASM)
.PHONY: clean clean-out
