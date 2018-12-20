'use strict';
const StringLib = require('string');
let handleMap = new Map;
let handleId = 0;

const that = module.exports = {
	getObjectById(env, id) {
		return Game.getObjectById(StringLib.readOneByteString(env, id));
	},

	enumToMap(list) {
		return [
			Object.freeze(list.reduce(function(map, val, ii) {
				map.set(val, ii);
				return map;
			}, new Map)),
			Object.freeze(list.reduce(function(map, val, ii) {
				map.set(ii, val);
				return map;
			}, new Map)),
		];
	},

	handleCtor(value) {
		let ii = ++handleId;
		handleMap.set(handleId, value);
		return ii;
	},

	handleDtor(id) {
		handleMap.delete(id);
	},

	handleGet(id) {
		return handleMap.get(id);
	},

	toColor(color) {
		color = that.toUint(color);
		if (color === 0) {
			return;
		} else if ((color & 0xff000000) >>> 0 !== 0xff000000) {
			return `rgba(${(color >> 16) & 0xff},${(color >> 8) & 0xff},${color & 0xff},${((color >> 24) & 0xff) / 0xff})`;
		} else {
			let str = (color & ~0xff000000).toString(16);
			let pad = '#';
			for (let ii = 6 - str.length; ii > 0; --ii) {
				pad += '0';
			}
			return pad + str;
		}
	},

	toLineStyle(style) {
		switch (style) {
			case 1:
				return 'dashed';
			case 2:
				return 'dotted';
		}
	},

	toUint(number) {
		return (number < 0) ? 0x100000000 + number : number;
	},
};
