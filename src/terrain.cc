#include "./javascript.h"
#include <screeps/terrain.h>
#include <memory>
#include <unordered_map>

namespace screeps {

namespace {
	static std::unordered_map<room_location_t, std::weak_ptr<terrain_t>> weak_terrain;
	static std::unordered_map<room_location_t, std::shared_ptr<terrain_t>> stored_terrain;

	class public_terrain_t : public terrain_t {
		public:
			public_terrain_t(room_location_t room) : terrain_t(room) {}
	};
}

void terrain_t::flush() {
	stored_terrain.clear();
	for (auto ii = weak_terrain.begin(); ii != weak_terrain.end(); ) {
		if (ii->second.lock()) {
			++ii;
		} else {
			ii = weak_terrain.erase(ii);
		}
	}
}

std::shared_ptr<terrain_t> terrain_t::load(room_location_t room) {
	auto ii = weak_terrain.find(room);
	if (ii != weak_terrain.end()) {
		auto ptr = ii->second.lock();
		if (ptr) {
			return ptr;
		} else {
			ptr = std::make_shared<public_terrain_t>(room);
			ii->second = ptr;
			return ptr;
		}
	}
	auto ptr = std::make_shared<public_terrain_t>(room);
	stored_terrain.emplace(room, ptr);
	weak_terrain.emplace(room, ptr);
	return ptr;
}

terrain_t::terrain_t(room_location_t room) {
	EM_ASM({
		var roomName = Module.screeps.position.generateRoomName($0);
		var view = Module.HEAP8.subarray($1, $1 + 625);
		view.fill(0);
		var terrain = Game.map.getRoomTerrain(roomName);
		for (var xx = 0; xx < 50; ++xx) {
			for (var yy = 0; yy < 50; ++yy) {
				var val;
				switch (terrain.get(xx, yy)) {
					case 0:
						val = 0;
						break;
					case TERRAIN_MASK_WALL:
						val = 1;
						break;
					case TERRAIN_MASK_SWAMP:
						val = 2;
						break;
					default:
						throw new Error('Failed to get terrain for '+ roomName+ '('+ xx+ ', '+ yy+ ')');
				}
				var index = xx * 50 + yy;
				view[index >> 2] |= val << ((index & 3) << 1);
			}
		}
	}, room.id, this);
}

void terrain_t::insert(room_location_t room, std::shared_ptr<terrain_t> terrain) {
	static std::unordered_map<room_location_t, std::shared_ptr<terrain_t>> eternal_terrain;
	weak_terrain.emplace(room, terrain);
	eternal_terrain.emplace(room, std::move(terrain));
}

} // namespace screeps
