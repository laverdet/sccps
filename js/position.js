'use strict';
const kWorldSize = 255;
let visualCache = new Map;

const that = module.exports = {
	read(env, ptr) {
		let xx = env.HEAPU16[ptr >> 1];
		let yy = env.HEAPU16[(ptr + 2) >> 1];
		return new RoomPosition(xx % 50, yy % 50, that.generateRoomName(xx / 50 | 0, yy / 50 | 0));
	},

	generateRoomName(rx, ry) {
		if (rx === 0 && ry === 0) {
			return 'sim';
		} else {
			return (
				(rx <= kWorldSize >> 1 ? 'W'+ ((kWorldSize >> 1) - rx) : 'E'+ (rx - (kWorldSize >> 1) - 1))+
				(ry <= kWorldSize >> 1 ? 'N'+ ((kWorldSize >> 1) - ry) : 'S'+ (ry - (kWorldSize >> 1) - 1))
			);
		}
	},

	parseRoomName(roomName) {
		if (roomName === 'sim') {
			return { rx: 0, ry: 0 };
		} else {
			let room = /^([WE])([0-9]+)([NS])([0-9]+)$/.exec(roomName);
			return {
				rx: (kWorldSize >> 1) + (room[1] === 'W' ? -Number(room[2]) : Number(room[2]) + 1),
				ry: (kWorldSize >> 1) + (room[3] === 'N' ? -Number(room[4]) : Number(room[4]) + 1),
			};
		}
	},

	getVisual(xx, yy) {
		let visual = visualCache.get(xx * kWorldSize + yy);
		if (visual === undefined) {
			visualCache.set(xx * kWorldSize + yy, visual = new RoomVisual(that.generateRoomName(xx, yy)));
		}
		return visual;
	},

	write(env, ptr, pos) {
		let room = that.parseRoomName(pos.roomName);
		env.HEAPU16[(ptr + 0) >> 1] = pos.x + room.rx * 50;
		env.HEAPU16[(ptr + 2) >> 1] = pos.y + room.ry * 50;
	},
};
