# Screeps C++ and JS sources
SCREEPS_SRCS := cpu.cc creep.cc game.cc memory.cc path-finder.cc position.cc resource.cc room.cc structure.cc terrain.cc visual.cc
RUNTIME_SRCS := array.js main.js object.js position.js string.js util.js vector.js inflate.js inflate.wasm.wasm

SCREEPS_COMMON_SRCS := $(addprefix $(SCREEPS_PATH)/src/common/,$(SCREEPS_SRCS))
SCREEPS_EMASM_SRCS := $(COMMON_SRCS) $(addprefix $(SCREEPS_PATH)/src/emasm/,$(SCREEPS_SRCS))
SCREEPS_NATIVE_SRCS := $(COMMON_SRCS) $(addprefix $(SCREEPS_PATH)/src/native/,$(SCREEPS_SRCS))
