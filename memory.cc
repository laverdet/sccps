#include "memory.h"
#include <emscripten.h>

namespace screeps {

bool memory_t::load() {
	EM_ASM({
		Module.screeps.string.writeOneByteStringData(Module, $0, RawMemory.get());
	}, memory.data());
	pos = memory.data();
	uint32_t magic;
	*this >>magic;
	if (magic != k_magic) {
		return false;
	}
	*this >>_internal_version >>_version;
	return true;
}

void memory_t::save() const {
	EM_ASM({
		Module.screeps.string.readOneByteStringData(Module, $0, $1);
	}, memory.data(), pos - memory.data());
}

} // namespace screeps
