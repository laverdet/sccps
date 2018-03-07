#include "./path-finder.h"
#include <emscripten.h>

path_finder_t::result_t path_finder_t::search(const position_t origin, const std::vector<goal_t>& goals, const options_t& options) {
	result_t result;
	result.path = std::make_unique<path_t>();
	EM_ASM({
		var roomCallback;
		if ($10) {
			var tmp = Object.create(PathFinder.CostMatrix.prototype);
			roomCallback = function(roomName) {
				var room = Module.screeps.position.parseRoomName(roomName);
				var ptr = Module.__ZN13path_finder_t19callback_trampolineEPvii($10, room.rx, room.ry);
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
			Module.screeps.vector.map(Module, $1, $2, 4, Module.screeps.position.read),
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
		Module.HEAPU32[$12 >> 2] = result.ops;
		Module.HEAPU32[$13 >> 2] = result.cost;
		Module.HEAP8[$14 >> 0] = result.incomplete;
	},
		&origin.xx,
		goals.data(), goals.size(),
		options.plain_cost, options.swamp_cost,
		options.flee,
		options.max_ops, options.max_rooms, options.max_cost,
		options.heuristic_weight,
		options.room_callback,
		result.path.get(),
		&result.ops, &result.cost, &result.incomplete
	);
	return result;
}

EMSCRIPTEN_KEEPALIVE
void* path_finder_t::callback_trampoline(void* fn, int xx, int yy) {
	auto callback = reinterpret_cast<callback_t>(fn);
	std::shared_ptr<cost_matrix_t> cost_matrix = callback(room_location_t(xx, yy));
	return cost_matrix.get();
}
