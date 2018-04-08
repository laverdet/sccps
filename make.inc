TARGET ?= debug

# Object paths
.SECONDEXPANSION:
SCREEPS_PATH ?= screeps
SCREEPS_OBJS = creep.bc game.bc memory.bc path-finder.bc position.bc resource.bc room.bc structure.bc terrain.bc
SCREEPS_RUNTIME = array.js creep.js game.js main.js object.js position.js resource.js room.js string.js structure.js util.js vector.js
BYTE_CODE_OBJS = $(addprefix build/$(TARGET)/$(SCREEPS_PATH)/src/,$(SCREEPS_OBJS)) $(addprefix build/$(TARGET)/,$(patsubst %.cc,%.bc,$(SRCS)))

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
CXXFLAGS += -g4 -O2 -ftrapv
EMFLAGS += -s ASSERTIONS=1
else
ifeq ($(TARGET), release)
CXXFLAGS += -O3
EMFLAGS += --llvm-lto 1
else
$(error Invalid target. Must be "debug" or "release" $(TARGET))
endif
endif

# Default targets
wasm: build/$(TARGET)/out/$$(MODULE_NAME).wasm screeps-js
asmjs: build/$(TARGET)/out/$$(MODULE_NAME).asm.js screeps-js
screeps-js: $(addprefix build/$(TARGET)/out/,$(SCREEPS_RUNTIME))

# Include header dependencies
include $(shell find build -name '*.d' 2>/dev/null)
%.h: %.h

# Directory creation
.PRECIOUS: build/. build%/.
build/.:
	mkdir -p $@

build%/.:
	mkdir -p $@

# Copy JS files to build/../
build/$(TARGET)/out/%.js: $(SCREEPS_PATH)/js/%.js | $$(@D)/.
	cp $< $@

# Bytecode targets
build/$(TARGET)/%.bc: %.cc | $$(@D)/.
	$(CXX) -c -o $@ $< $(CXXFLAGS) -MD

# wasm
build/$(TARGET)/out/%.wasm: $$(BYTE_CODE_OBJS)
	$(CXX) -o $(shell echo $@ | sed 's/wasm$$/js/') $^ $(CXXFLAGS) $(EMFLAGS) -s WASM=1
	mv $(shell echo $@ | sed 's/wasm$$/js/') $(shell dirname $@)/runtime.js
	$(RM) $(shell echo $@ | sed 's/wasm$$/wast/') $(shell echo $@ | sed 's/wasm$$/wasm.map/')

# asmjs
build/$(TARGET)/out/%.asm.js: $$(BYTE_CODE_OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(EMFLAGS) --memory-init-file 0

clean:
	$(RM) -rf build

.PHONY: clean wasm asmjs screeps-js
