# Include globals and implicit patterns
SCREEPS_PATH := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
include $(SCREEPS_PATH)/make/globals.mk
.SECONDEXPANSION:
include $(SCREEPS_PATH)/make/patterns.mk

# Compiler flags and JS runtime
CXXFLAGS += -I$(SCREEPS_PATH)/include
EXPORTED_FUNCTIONS += __Z4loopv
WASM_EMFLAGS += -s EXPORTED_FUNCTIONS=$$(echo $(EXPORTED_FUNCTIONS) | $(TO_JSON))
RUNTIME := array.js console.js error.js main.js object.js position.js string.js util.js vector.js inflate.js inflate.wasm.wasm

# Bytecode targets
BC_FILES = $(addprefix $(BUILD_PATH)/,$(patsubst %.cc,%.bc,$(SRCS)))
ifeq ($(TARGET), debug)
DYLIB_BC = $(BC_FILES)
else
EMASM_BC = $(BC_FILES)
endif

# Various targets
DYLIB_LINKER_SYMBOLS = $(SCREEPS_PATH)/dylib-symbols
DYLIB_ASMJS_OBJS = $(patsubst %.bc,%.dylib.js.deflate,$(DYLIB_BC))
DYLIB_MAPS = $(patsubst %.bc,%.dylib.map.deflate,$(DYLIB_BC))
DYLIB_WASM_OBJS = $(patsubst %.bc,%.dylib.wasm.deflate,$(DYLIB_BC))
DYLIB_SYMBOLS = $(patsubst %.bc,%.nm,$(DYLIB_BC))
NATIVE_OBJS = $(addprefix $(BUILD_PATH)/,$(patsubst %.cc,%.o,$(SRCS)))
SCREEPS_BC_FILES := $(SCREEPS_PATH)/$(BUILD_PATH)/emasm-bc-files.txt

# Output targets
.PHONY: asmjs wasm
asmjs: $(addprefix $(ASMJS)/,$(RUNTIME))
wasm: $(WASM)/$(MODULE_NAME).wasm $(addprefix $(WASM)/,$(RUNTIME))
ifeq ($(TARGET), debug)
asmjs: $(ASMJS)/$(MODULE_NAME).wasm $(ASMJS)/source-map.js
else
asmjs: $(ASMJS)/$(MODULE_NAME).js
endif
native: $(NATIVE)/$(MODULE_NAME).js $(addprefix $(NATIVE)/,$(RUNTIME))

# Dynamic library symbol exports
$(DYLIB_LINKER_SYMBOLS): $(DYLIB_LINKER_SYMBOLS).marker
$(DYLIB_LINKER_SYMBOLS).marker: $$(DYLIB_SYMBOLS) | $$(@D)/.
	touch $@
	$(MERGE_SYMBOLS) $(DYLIB_LINKER_SYMBOLS) -- $(DYLIB_SYMBOLS)

# Copy screeps runtime JS files output
$(ASMJS)/% $(WASM)/% $(NATIVE)/%: $(SCREEPS_PATH)/js/% | $$(@D)/.
	cp $< $@

# static build
$(ASMJS)/$(MODULE_NAME).js: EMFLAGS += $(ASMJS_EMFLAGS)
$(BUILD_PATH)/$(SCREEPS_PATH)/wasm.js: EMFLAGS += $(WASM_EMFLAGS)
$(ASMJS)/$(MODULE_NAME).js $(BUILD_PATH)/$(SCREEPS_PATH)/wasm.js: $(SCREEPS_BC_FILES) $$(EMASM_BC) $$(DYLIB_BC) | $$(@D)/.
	$(EMCXX) -o $@ $$(xargs -n1 echo < $(SCREEPS_BC_FILES) | sed 's/^/$(SCREEPS_PATH)\//') $(EMASM_BC) $(DYLIB_BC) $(EMFLAGS) $(MAIN_EMFLAGS)

# asmjs dylib package
$(SCREEPS_PATH)/$(BUILD_PATH)/asmjs.js.deflate: $(DYLIB_LINKER_SYMBOLS)
$(ASMJS)/$(MODULE_NAME).wasm: $(SCREEPS_PATH)/$(BUILD_PATH)/asmjs.js.deflate $$(DYLIB_ASMJS_OBJS) $$(DYLIB_MAPS) | $$(@D)/.
	$(MAKE_ARCHIVE) $@ --relative $(SCREEPS_PATH)/$(BUILD_PATH) $(SCREEPS_PATH)/$(BUILD_PATH)/asmjs.js.deflate \
		--relative $(BUILD_PATH) $(DYLIB_ASMJS_OBJS) $(DYLIB_MAPS)

# wasm package
$(BUILD_PATH)/$(SCREEPS_PATH)/wasm.wasm: $(BUILD_PATH)/$(SCREEPS_PATH)/wasm.js
$(WASM)/$(MODULE_NAME).wasm: $(BUILD_PATH)/$(SCREEPS_PATH)/wasm.js.deflate $(BUILD_PATH)/$(SCREEPS_PATH)/wasm.wasm.deflate | $$(@D)/.
	$(MAKE_ARCHIVE) $@ --relative $(BUILD_PATH)/$(SCREEPS_PATH) $^

# native module build
NATIVE_MODULE_TARGET := $(SCREEPS_PATH)/$(BUILD_PATH)/gyp/build/Debug/module.node
$(SCREEPS_PATH)/$(BUILD_PATH)/module.a: $$(NATIVE_OBJS) | $$(@D)/.
	$(AR) rcs $@ $^
$(NATIVE)/$(MODULE_NAME).js: $(NATIVE_MODULE_TARGET) | $$(@D)/.
	echo 'module.exports = ivm_dlopen.applySync(undefined, ["'$$(pwd)/$^'"]).deref();' > $@

$(NATIVE_MODULE_TARGET): $(SCREEPS_PATH)/$(BUILD_PATH)/module.a

# native build
$(MODULE_NAME): $(SCREEPS_PATH)/$(BUILD_PATH)/screeps.a $$(NATIVE_OBJS)
	$(CXX) -o $@ $^ $(SCREEPS_CXXFLAGS) $(CXXFLAGS) $(NATIVE_CXXFLAGS)

# Recursive screeps/% targets
SCREEPS_TARGETS_RELATIVE := $(BUILD_PATH)/asmjs.js.deflate $(BUILD_PATH)/screeps.a compile_flags.txt clean very-clean
SCREEPS_TARGETS := $(SCREEPS_BC_FILES) $(NATIVE_MODULE_TARGET) $(addprefix $(SCREEPS_PATH)/,$(SCREEPS_TARGETS_RELATIVE))
$(SCREEPS_TARGETS): $(SCREEPS_PATH)/%: nothing
	$(MAKE) -C $(SCREEPS_PATH) $*

# Cleanups
.PHONY: clean very-clean
clean: $(SCREEPS_PATH)/clean
	$(RM) -rf build
very-clean: $(SCREEPS_PATH)/very-clean
	$(RM) -rf build
