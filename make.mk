# Set paths & use `bash`
SHELL := bash
TARGET ?= debug
BUILD_PATH = build/$(TARGET)
SCREEPS_PATH ?= screeps
ASMJS = $(BUILD_PATH)/asmjs
WASM = $(BUILD_PATH)/wasm
UGLIFY := $(dir $(lastword $(MAKEFILE_LIST)))node_modules/.bin/uglifyjs -b beautify=false,preserve_line=true

# Utilities
DEFLATE = $(SCREEPS_PATH)/deflate.js
MAKE_ARCHIVE = $(SCREEPS_PATH)/make-archive.js
MERGE_SYMBOLS = $(SCREEPS_PATH)/merge-symbols.js
TO_JSON = $(SCREEPS_PATH)/to-json.js
STRIP_MAP = $(SCREEPS_PATH)/strip-map.js
NOTHING :=
SPACE := $(NOTHING) $(NOTHING)
COMMA := ,

# Default compiler flags
CXXFLAGS += -std=c++17 \
	-Wall -Wextra -Wno-invalid-offsetof \
	-isystem $(SCREEPS_PATH)/include -I.
EMFLAGS += -s STRICT=1 -s DISABLE_EXCEPTION_CATCHING=0 -s ABORTING_MALLOC=0
MAIN_EMFLAGS += -s MAIN_MODULE=2 -s ENVIRONMENT=shell -s NO_FILESYSTEM=1 -s TOTAL_MEMORY=201326592
SIDE_EMFLAGS += -s SIDE_MODULE=1 -s EXPORT_ALL=1
EXPORTED_FUNCTIONS += __Z4loopv
MISSING_SYMBOLS := ___cxa_find_matching_catch ___cxa_find_matching_catch_2 ___cxa_find_matching_catch_3 ___resumeException ___errno_location

# Additional flags for debug or release
ifeq ($(TARGET), debug)
CXXFLAGS += -ftrapv
EM_CXXFLAGS += -g4 -O2
NATIVE_CXXFLAGS += -g
EMFLAGS += -s ASSERTIONS=1 -g4 -O2
MAIN_EMFLAGS += -s DEMANGLE_SUPPORT=1
DYNAMIC_LINK = 1
else
ifeq ($(TARGET), release)
CXXFLAGS += -O3
EMFLAGS += --llvm-lto 3 -O3
else
$(error Invalid target. Must be "debug" or "release" $(TARGET))
endif
endif

# Object paths
.SECONDEXPANSION:
SCREEPS_SRCS := cpu.cc creep.cc game.cc memory.cc path-finder.cc position.cc resource.cc room.cc structure.cc terrain.cc visual.cc
SCREEPS_RUNTIME := array.js main.js object.js position.js string.js util.js vector.js inflate.js inflate.wasm.wasm

# Create objects from sources
COMMON_SRCS = $(addprefix $(SCREEPS_PATH)/src/common/,$(SCREEPS_SRCS))
EMASM_SRCS = $(COMMON_SRCS) $(addprefix $(SCREEPS_PATH)/src/emasm/,$(SCREEPS_SRCS))
NATIVE_SRCS = $(COMMON_SRCS) $(addprefix $(SCREEPS_PATH)/src/native/,$(SCREEPS_SRCS))
ifeq ($(DYNAMIC_LINK), 1)
DYLIB_SRCS = $(SRCS)
ASMJS_EMFLAGS += -s DEFAULT_LIBRARY_FUNCS_TO_INCLUDE=$$($(TO_JSON) < $(SCREEPS_PATH)/emscripten_symbols) \
	-s EXPORTED_FUNCTIONS=$$((cat $(DYLIB_LINKER_SYMBOLS); echo $(MISSING_SYMBOLS); egrep '[CD]1' $(DYLIB_LINKER_SYMBOLS) | perl -pe 's/([CD])1/$$1\x32/') | $(TO_JSON)) \
	-s EXTRA_EXPORTED_RUNTIME_METHODS="['loadDynamicLibrary']"
else
EMASM_SRCS += $(SRCS)
endif
OBJS = $(addprefix $(BUILD_PATH)/,$(patsubst %.cc,%.bc,$(EMASM_SRCS)))
NATIVE_OBJS = $(addprefix $(BUILD_PATH)/,$(patsubst %.cc,%.o,$(NATIVE_SRCS)))
NATIVE_USER_OBJS = $(addprefix $(BUILD_PATH)/,$(patsubst %.cc,%.o,$(SRCS)))
DYLIB_OBJS = $(addprefix $(BUILD_PATH)/,$(patsubst %.cc,%.bc,$(DYLIB_SRCS)))
DYLIB_ASMJS_FILES = $(patsubst %.bc,%.dylib.js.deflate,$(DYLIB_OBJS))
DYLIB_MAP_FILES = $(patsubst %.bc,%.dylib.map.deflate,$(DYLIB_OBJS))
DYLIB_WASM_FILES = $(patsubst %.bc,%.dylib.wasm.deflate,$(DYLIB_OBJS))
DYLIB_LINKER_SYMBOLS = $(BUILD_PATH)/dylib-symbols
DYLIB_SYMBOLS = $(patsubst %.bc,%.nm,$(DYLIB_OBJS))
ASMJS_EMFLAGS += -s WASM=0 --memory-init-file 0
WASM_EMFLAGS += -s WASM=1 -s BINARYEN_ASYNC_COMPILATION=0 -s EXPORTED_FUNCTIONS=$$(echo $(EXPORTED_FUNCTIONS) | $(TO_JSON))

# Default targets
asmjs: runtime-asmjs
wasm: runtime-wasm $(WASM)/$(MODULE_NAME).wasm
runtime-asmjs: $(addprefix $(ASMJS)/,$(SCREEPS_RUNTIME))
runtime-wasm: $(addprefix $(WASM)/,$(SCREEPS_RUNTIME))
.PHONY: asmjs wasm runtime-asmjs runtime-wasm

# Compress asmjs in debug builds, also include source map
ifeq ($(TARGET), debug)
asmjs: $(ASMJS)/$(MODULE_NAME).wasm $(ASMJS)/source-map.js
else
asmjs: $(ASMJS)/$(MODULE_NAME).asmjs.js
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
$(BUILD_PATH)/%.bc: %.cc | $$(@D)/.
	$(EMCXX) -c -o $@ $< -MD $(CXXFLAGS) $(EM_CXXFLAGS)
.PRECIOUS: $(BUILD_PATH)/%.bc

# Dynamic library symbol exports
%.nm: %.bc
	$(LLVM_NM) $^ > $@
$(DYLIB_LINKER_SYMBOLS): $(DYLIB_LINKER_SYMBOLS).marker
$(DYLIB_LINKER_SYMBOLS).marker: $$(DYLIB_SYMBOLS) | $$(@D)/.
	touch $@
	$(MERGE_SYMBOLS) $(DYLIB_LINKER_SYMBOLS) -- $(DYLIB_SYMBOLS)
.PRECIOUS: %.nm

# DEFLATE
%.js.deflate: %.js
	$(UGLIFY) $< | $(DEFLATE) > $@
%.map.deflate: %.js.map
	$(STRIP_MAP) < $< | $(DEFLATE) > $@
%.wasm.deflate: %.wasm
	$(DEFLATE) < $< > $@

# asmjs specifics
$(BUILD_PATH)/screeps/asmjs.js: $$(OBJS) $(DYLIB_LINKER_SYMBOLS) | $$(@D)/.
	$(EMCXX) -o $@ $(OBJS) $(EMFLAGS) $(MAIN_EMFLAGS) $(ASMJS_EMFLAGS)
$(ASMJS)/$(MODULE_NAME).wasm: $(BUILD_PATH)/screeps/asmjs.js.deflate $$(DYLIB_ASMJS_FILES) $$(DYLIB_MAP_FILES) | $$(@D)/.
	$(MAKE_ARCHIVE) $(BUILD_PATH) $@ $^

%.dylib.js: %.bc
	$(EMCXX) -o $@ $^ $(EMFLAGS) $(SIDE_EMFLAGS) -s WASM=0
%.js.map: %.js ;
.PRECIOUS: %.dylib.js %.js.map

# wasm specifics
$(BUILD_PATH)/screeps/wasm.js: $$(OBJS) $$(DYLIB_OBJS) | $$(@D)/.
	$(EMCXX) -o $@ $(OBJS) $(DYLIB_OBJS) $(EMFLAGS) $(MAIN_EMFLAGS) $(WASM_EMFLAGS)
$(BUILD_PATH)/screeps/wasm.wasm: $(BUILD_PATH)/screeps/wasm.js ;
$(WASM)/$(MODULE_NAME).wasm: $(BUILD_PATH)/screeps/wasm.js.deflate $(BUILD_PATH)/screeps/wasm.wasm.deflate | $$(@D)/.
	$(MAKE_ARCHIVE) $(BUILD_PATH) $@ $^

%.dylib.wasm: %.bc
	$(EMCXX) -o $@ $^ $(EMFLAGS) $(SIDE_EMFLAGS) -s WASM=1
.PRECIOUS: %.dylib.wasm

# Copy screeps runtime JS files output
$(ASMJS)/% $(WASM)/%: $(SCREEPS_PATH)/js/% | $$(@D)/.
	cp $< $@

# native specifics
$(BUILD_PATH)/%.o: %.cc | $$(@D)/.
	$(CXX) -c -o $@ $< -MD $(CXXFLAGS) $(NATIVE_CXXFLAGS)
$(BUILD_PATH)/screeps.ar: $$(NATIVE_OBJS)
	$(AR) rcs $@ $^
.PRECIOUS: $(BUILD_PATH)/%.o

$(MODULE_NAME): $(BUILD_PATH)/screeps.ar $$(NATIVE_USER_OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(NATIVE_CXXFLAGS)

# Cleanups
clean:
	$(RM) -rf build
clean-out:
	$(RM) -rf $(ASMJS) $(WASM)
.PHONY: clean clean-out
