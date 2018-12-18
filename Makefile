SCREEPS_PATH=.
include make/globals.mk
.SECONDEXPANSION:
include make/patterns.mk

# Screeps C++ sources and object files
SRCS := cpu.cc creep.cc game.cc memory.cc path-finder.cc position.cc resource.cc room.cc structure.cc terrain.cc visual.cc
COMMON_SRCS := $(addprefix src/common/,$(SRCS))
EMASM_SRCS := $(COMMON_SRCS) $(addprefix src/emasm/,$(SRCS))
EMASM_OBJS := $(addprefix $(BUILD_PATH)/,$(patsubst %.cc,%.bc,$(EMASM_SRCS)))
NATIVE_SRCS := $(COMMON_SRCS) $(addprefix src/native/,$(SRCS))
NATIVE_OBJS := $(addprefix $(BUILD_PATH)/,$(patsubst %.cc,%.o,$(NATIVE_SRCS)))

# Compilation options
CXXFLAGS += -Iinclude

# asmjs main module used for loading dynamic libraries
MAIN2_EMFLAGS := -s DEFAULT_LIBRARY_FUNCS_TO_INCLUDE=$$($(TO_JSON) < emscripten_symbols) \
	-s EXPORTED_FUNCTIONS=$$((cat dylib-symbols; egrep '[CD]1' dylib-symbols | perl -pe 's/([CD])1/$$1\x32/'; sed 's/^/_/' emscripten_symbols) | $(TO_JSON)) \
	-s EXTRA_EXPORTED_RUNTIME_METHODS="['loadDynamicLibrary']"
$(BUILD_PATH)/asmjs.js: $(EMASM_OBJS) dylib-symbols | compile_flags.txt nothing
	$(EMCXX) -o $@ $(EMASM_OBJS) $(EMFLAGS) $(MAIN_EMFLAGS) $(ASMJS_EMFLAGS) $(MAIN2_EMFLAGS)

# llvm bytecode targets
$(BUILD_PATH)/emasm-bc-files.txt: $(EMASM_OBJS) | compile_flags.txt nothing
	echo $^ > $@

# native archive
$(BUILD_PATH)/screeps.a: $(NATIVE_OBJS) | compile_flags.txt nothing
	$(AR) rcs $@ $^

# Linter. nb: we can't lint emasm sources because of EMASM macros
.PHONY: tidy
tidy: $(NATIVE_SRCS)
	echo $^ | xargs -n1 $(CLANG_TIDY) -header-filter='.*'

# Cleanups. Exported symbols for dynamic library main are left alone by default because they only
# affect debug builds and keeping them around makes incremental builds more robust to changes
.PHONY: clean very-clean
clean:
	$(RM) -rf build
very-clean: clean
	$(RM) -rf $(DYLIB_LINKER_SYMBOLS) $(DYLIB_LINKER_SYMBOLS).marker
