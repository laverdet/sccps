#include <screeps/terrain.h>
#include <emscripten.h>

namespace screeps {

terrain_t::terrain_t(room_location_t room) {
	EM_ASM({
		var roomName = Module.screeps.position.generateRoomName($0);
		var view = new Uint8Array(Module.buffer, $1, 625);
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

} // namespace screeps
