#include <screeps/path-finder.h>
#include "./javascript.h"

namespace screeps {

path_finder_t::result_t path_finder_t::search(const position_t origin, const std::vector<goal_t>& goals, const options_t& options) {
	result_t result;
	result.path = std::make_unique<path_t>();
	EM_ASM({
		var roomCallback;
		if ($10) {
			var tmp = Object.create(PathFinder.CostMatrix.prototype);
			roomCallback = function(roomName) {
				var room = Module.screeps.position.parseRoomName(roomName);
				var ptr = Module.__ZN7screeps13path_finder_t19callback_trampolineEPvii($10, room.rx, room.ry);
				if (ptr === 0) {
					return false;
				} else {
					tmp._bits = new Uint8Array(Module.buffer, ptr, 2500);
					return tmp;
				}
			};
		}
		var result = PathFinder.search(
			Module.screeps.position.read(Module, $0),
			Module.screeps.vector.map(Module, $1, $2, 4, function(env, ptr) {
				return { pos: Module.screeps.position.read(env, ptr), range: env.readInt32(ptr + 4) };
			}),
			{
				plainCost: $3,
				swampCost: $4,
				flee: $5,
				maxOps: $6,
				maxRooms: $7,
				maxCost: $8,
				heuristicWeight: $9,
				roomCallback: roomCallback,
			}
		);
		Module.screeps.array.write(Module, $11, 4, 1500, result.path, Module.screeps.position.write);
		Module.writeInt32($12, result.ops);
		Module.writeInt32($13, result.cost);
		Module.writeInt8($14, result.incomplete);
	},
		&origin.xx,
		goals.data(), goals.size(),
		options.plain_cost, options.swamp_cost,
		options.flee,
		options.max_ops, options.max_rooms, options.max_cost,
		options.heuristic_weight,
		options.room_callback ? &options : nullptr,
		result.path.get(),
		&result.ops, &result.cost, &result.incomplete
	);
	return result;
}

EMSCRIPTEN_KEEPALIVE
const void* path_finder_t::callback_trampoline(void* fn, int xx, int yy) {
	options_t& options = *reinterpret_cast<options_t*>(fn);
	return reinterpret_cast<const void*>(options.room_callback(room_location_t(xx, yy)));
}

} // namespace screeps
