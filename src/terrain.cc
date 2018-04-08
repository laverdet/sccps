#include <screeps/terrain.h>
#include <emscripten.h>

namespace screeps {

terrain_t::terrain_t(room_location_t room) {
	EM_ASM({
		var roomName = Module.screeps.position.generateRoomName($0, $1);
		var view = new Uint8Array(Module.buffer, $2, 625);
		for (var xx = 0; xx < 50; ++xx) {
			for (var yy = 0; yy < 50; ++yy) {
				var val;
				switch (Game.map.getTerrainAt(xx, yy, roomName)) {
					case 'plain':
						val = 0;
						break;
					case 'wall':
						val = 1;
						break;
					case 'swamp':
						val = 2;
						break;
					default:
						throw new Error('Failed to get terrain for '+ roomName+ '('+ xx+ ', '+ yy+ ')');
				}
				var index = xx * 50 + yy;
				view[index >> 2] |= val << ((index & 3) << 1);
			}
		}
	}, room.xx, room.yy, this);
}

} // namespace screeps
