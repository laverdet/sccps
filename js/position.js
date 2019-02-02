'use strict';
const kWorldSize = 256;
const kWorldSize2 = kWorldSize >> 1;
let visualCache = new Map;
let roomNameCache = new Map;

const that = module.exports = {
	generateRoomName(bits) {
		let roomName = roomNameCache.get(bits);
		if (roomName === undefined) {
			if (bits === 0x8080) {
				roomName = 'sim';
			} else {
				let rx = bits & 0xff;
				let ry = bits >> 8;
				roomName = (
					(rx > 0x7f ? `W${0xff - rx}` : `E${rx}`)+
					(ry > 0x7f ? `N${0xff - ry}` : `S${ry}`)
				);
			}
			roomNameCache.set(bits, roomName);
		}
		return roomName;
	},

	parseRoomName(roomName) {
		if (roomName === 'sim') {
			return 0x8080;
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
				rx = 0xff - rx;
			}
			if (verticalDir === 'N') {
				ry = 0xff - ry;
			}
			return rx | (ry << 8);
		}
	},

	getVisual(bits) {
		let visual = visualCache.get(bits);
		if (visual === undefined) {
			visualCache.set(bits, visual = new RoomVisual(that.generateRoomName(bits)));
		}
		return visual;
	},
};
