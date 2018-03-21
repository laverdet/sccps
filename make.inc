TARGET ?= debug

# Object paths
SCREEPS_PATH ?= screeps
SCREEPS_OBJS = creep.bc game.bc memory.bc path-finder.bc position.bc resource.bc room.bc structure.bc terrain.bc
SCREEPS_RUNTIME = array.js creep.js game.js main.js object.js position.js resource.js room.js string.js structure.js util.js vector.js
BYTE_CODE_OBJS = $(addprefix build/$(TARGET)/$(SCREEPS_PATH)/,$(SCREEPS_OBJS)) $(addprefix build/$(TARGET)/,$(OBJS))

# Default compiler flags
CXXFLAGS += -std=c++1z --bind -Wall -Wno-invalid-offsetof
EMFLAGS += \
	-s STRICT=1 -s MODULARIZE=1 -s NO_FILESYSTEM=1 -s BINARYEN_ASYNC_COMPILATION=0 \
	-s DISABLE_EXCEPTION_CATCHING=0 \
	-s ALLOW_MEMORY_GROWTH=0 -s TOTAL_MEMORY=201326592

# Additional flags for debug or release
ifeq ($(TARGET), debug)
CXXFLAGS += -g4 -O2
else
ifeq ($(TARGET), release)
CXXFLAGS += -O3
EMFLAGS += --llvm-lto 1
else
$(error Invalid target. Must be "debug" or "release" $(TARGET))
endif
endif

# Default targets
wasm: build/$(TARGET)/out/$(MODULE_NAME).wasm screeps-js
asmjs: build/$(TARGET)/out/$(MODULE_NAME).js screeps-js
screeps-js: $(addprefix build/$(TARGET)/out/,$(SCREEPS_RUNTIME))

# Include header dependencies
include $(shell find build -name '*.d')
%.h: %.h

# Copy JS files to build/.../
build/$(TARGET)/out/%.js: $(SCREEPS_PATH)/%.js
	mkdir -p $(shell dirname $@)
	cp $< $@

# Bytecode targets
build/$(TARGET)/%.bc: %.cc
	mkdir -p $(shell dirname $@)
	$(CXX) $< $(CXXFLAGS) -MM | sed 's/^[^:]*://' | echo '$@:'"$$(cat)" > $@.d
	$(CXX) -c -o $@ $< $(CXXFLAGS)

# wasm
build/$(TARGET)/out/$(MODULE_NAME).wasm: $(BYTE_CODE_OBJS)
	mkdir -p $(shell dirname $@)
	$(CXX) -o $(shell echo $@ | sed 's/wasm$$/js/') $^ $(CXXFLAGS) $(EMFLAGS) -s WASM=1
	mv $(shell echo $@ | sed 's/wasm$$/js/') $(shell dirname $@)/runtime.js
	$(RM) $(shell echo $@ | sed 's/wasm$$/wast/') $(shell echo $@ | sed 's/wasm$$/wasm.map/')

# asmjs
build/$(TARGET)/out/$(MODULE_NAME).js: $(BYTE_CODE_OBJS)
	mkdir -p $(shell dirname $@)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(EMFLAGS) --memory-init-file 0

clean:
	$(RM) -rf build

.PHONY: wasm asmjs screeps-js clean
