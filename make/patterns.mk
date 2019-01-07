# Include header dependencies
include $(shell find build -name '*.d' 2>/dev/null)

# Directory creation
.PRECIOUS: . %/.
.:
	mkdir -p $@
%/.:
	mkdir -p $@

# Bytecode targets
.PRECIOUS: $(BUILD_PATH)/%.bc
$(BUILD_PATH)/%.bc: %.cc | $$(@D)/. compile_flags.txt
	$(EMCXX) -c -o $@ $< -MD $(CXXFLAGS) $(EM_CXXFLAGS)

# Native object files
.PRECIOUS: $(BUILD_PATH)/%.o
$(BUILD_PATH)/%.o: %.cc | $$(@D)/. compile_flags.txt
	$(CXX) -c -o $@ $< -MD $(CXXFLAGS) $(NATIVE_CXXFLAGS)

# Dynamic library symbol exports
.PRECIOUS: %.nm
%.nm: %.bc
	$(LLVM_NM) $^ > $@

# DEFLATE
%.js.deflate: %.js | $(SCREEPS_PATH)/node_modules
	$(UGLIFY) $< | $(DEFLATE) > $@
%.map.deflate: %.js.map
	$(STRIP_MAP) < $< | $(DEFLATE) > $@
%.wasm.deflate: %.wasm
	$(DEFLATE) < $< > $@

# asmjs specifics
.PRECIOUS: %.dylib.js %.js.map
%.dylib.js: %.bc
	$(EMCXX) -o $@ $^ $(EMFLAGS) $(SIDE_EMFLAGS) -s WASM=0
%.js.map: %.js ;

# wasm specifics (unused since wasm dylibs are problematic)
.PRECIOUS: %.dylib.wasm
%.dylib.wasm: %.bc
	$(EMCXX) -o $@ $^ $(EMFLAGS) $(SIDE_EMFLAGS) -s WASM=1

# Install npm dependencies
$(SCREEPS_PATH)/node_modules: $(SCREEPS_PATH)/package.json
	cd $(SCREEPS_PATH); npm install && touch $@

# Used to silence "... is up to date" messages. Also used to force recursive targets to run.
.PHONY: nothing
nothing:
	@:

# llvm tools compile flags
compile_flags.txt: $(filter-out %.d,$(MAKEFILE_LIST))
	echo $(CXXFLAGS) | xargs -n1 echo > $@
	CPP="$$($(CXX) -E -xc++ - -v < /dev/null 2>&1)"; \
	echo "$$CPP" | grep -A100 '#include "..." search starts here:' | \
	sed -ne 's/^ /-isystem/gp' | sed -e 's/ (framework directory)//' >> $@
