#include <screeps/memory.h>
#include "./javascript.h"

namespace screeps {

bool raw_memory_t::load(memory_reader_t& reader, int segment) const {
	// Load data from RawMemory
	int size = EM_ASM_INT({
		let data;
		if ($0 === -1) {
			data = RawMemory.get();
		} else {
			if ($0 in RawMemory.segments) {
				data = RawMemory.segments[$0];
			} else {
				return -1;
			}
		}
		if (data.length > $2) {
			return -1;
		}
		Module.screeps.string.writeTwoByteStringData(Module, $1, data);
		return data.length * 2;
	}, segment, reader.data(), reader.capacity());
	if (size == -1) {
		return false;
	}
	return reader.reset(size);
}

bool raw_memory_t::save(memory_writer_t& writer, int segment) const {
	// Can only save 10 segments per tick
	if (segment != -1 && !saved_segments[segment]) {
		if (count_saved >= k_memory_max_active_segments) {
			return false;
		}
		++count_saved;
		saved_segments[segment] = true;
	}

	// Update `size`
	uint32_t size = writer.pos - writer.data();
	*(reinterpret_cast<uint32_t*>(writer.data()) + 1) = size;

	// Save to RawMemory
	EM_ASM({
		let length = ($2 >> 1) + ($2 % 2);
		let data = Module.screeps.string.readTwoByteStringData(Module, $1, $2);
		if ($0 === -1) {
			RawMemory.set(data);
		} else {
			RawMemory.segments[$0] = data;
		}
	}, segment, writer.data(), size);
	return true;
}

} // namespace screeps
