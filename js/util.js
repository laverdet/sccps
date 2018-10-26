'use strict';
const StringLib = require('string');

module.exports = {
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

	toColor(color) {
		if (color === 0) {
			return;
		} else if (color & 0xff000000) {
			return `rgba(${(color >> 16) & 0xff},${(color >> 8) & 0xff},${color & 0xff},${((color >> 24) & 0xff) / 0xff})`;
		} else {
			let str = color.toString(16);
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
};
