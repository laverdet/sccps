'use strict';
const kWorldSize = 256;
const kWorldSize2 = kWorldSize >> 1;
let visualCache = new Map;
let roomNameCache = new Map;

const that = module.exports = {
	read(env, ptr) {
		let xx = env.readUint16(ptr);
		let yy = env.readUint16(ptr + 2);
		return new RoomPosition(xx % 50, yy % 50, that.generateRoomName(xx / 50 | 0, yy / 50 | 0));
	},

	generateRoomName(bits) {
		let roomName = roomNameCache.get(bits);
		if (roomName === undefined) {
			if (bits === 0) {
				roomName = 'sim';
			} else {
				let rx = bits >> 16;
				let ry = bits & 0xff;
				roomName = (
					(rx < kWorldSize2 ? `W${kWorldSize2 - rx - 1}` : `E${rx - kWorldSize2}`)+
					(ry < kWorldSize2 ? `N${kWorldSize2 - ry - 1}` : `S${ry - kWorldSize2}`)
				);
			}
			roomNameCache.set(bits, roomName);
		}
		return roomName;
	},

	parseRoomName(roomName) {
		if (roomName === 'sim') {
			return 0;
		} else {
			let rx = parseInt(roomName.substr(1), 10);
			let verticalPos = 2;
			if (rx >= 100) {
				verticalPos = 4;
			} else if (rx >= 10) {
				verticalPos = 3;
			}
			let ry = parseInt(roomName.substr(verticalPos + 1), 10);
			let horizontalDir = roomName.charAt(0);
			let verticalDir = roomName.charAt(verticalPos);
			if (horizontalDir === 'W') {
				rx = -rx - 1;
			}
			if (verticalDir === 'N') {
				ry = -ry - 1;
			}
			return ((rx + kWorldSize2) << 16) | (ry + kWorldSize2);
		}
	},

	getVisual(xx, yy) {
		let visual = visualCache.get(xx * kWorldSize + yy);
		if (visual === undefined) {
			visualCache.set(xx * kWorldSize + yy, visual = new RoomVisual(that.generateRoomName(xx, yy)));
		}
		return visual;
	},
};
