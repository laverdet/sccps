# Paths
SHELL := bash # needed for $PATH expansion
TARGET ?= debug
BUILD_PATH := build/$(TARGET)
ASMJS := $(BUILD_PATH)/asmjs
WASM := $(BUILD_PATH)/wasm
NATIVE := $(BUILD_PATH)/native

# Utilities
EMCXX ?= em++
CLANG_TIDY ?= clang-tidy
LLVM_NM ?= llvm-nm
DEFLATE := $(SCREEPS_PATH)/deflate.js
MAKE_ARCHIVE := $(SCREEPS_PATH)/make-archive.js
MERGE_SYMBOLS := $(SCREEPS_PATH)/merge-symbols.js
TO_JSON := $(SCREEPS_PATH)/to-json.js
STRIP_MAP := $(SCREEPS_PATH)/strip-map.js
UGLIFY := $(SCREEPS_PATH)/node_modules/.bin/uglifyjs -b beautify=false,preserve_line=true
MAKEFILE_DEPS = $(filter-out %.d,$(MAKEFILE_LIST))
NOTHING :=
SPACE := $(NOTHING) $(NOTHING)
COMMA := ,

# Default compiler flags
CXXFLAGS += -xc++ -std=c++17 -Wall -Wextra -Wno-invalid-offsetof
EMFLAGS += -s STRICT=1 -s ABORTING_MALLOC=0
MAIN_EMFLAGS += -s MAIN_MODULE=2 -s ENVIRONMENT=shell -s NO_FILESYSTEM=1 -s TOTAL_MEMORY=201326592
SIDE_EMFLAGS += -s SIDE_MODULE=1 -s EXPORT_ALL=1
ASMJS_EMFLAGS += -s WASM=0 --memory-init-file 0
WASM_EMFLAGS += -s WASM=1 -s BINARYEN_ASYNC_COMPILATION=0

# Additional flags for debug or release
ifeq ($(TARGET), debug)
CXXFLAGS += -ftrapv
EM_CXXFLAGS += -g4 -O2
NATIVE_CXXFLAGS += -g
EMFLAGS += -s ASSERTIONS=1 -g4 -O2
MAIN_EMFLAGS += -s DEMANGLE_SUPPORT=1
else
ifeq ($(TARGET), release)
CXXFLAGS += -O3
EMFLAGS += --llvm-lto 3 -O3
else
$(error Invalid target. Must be "debug" or "release" $(TARGET))
endif
endif
