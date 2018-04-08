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

	pad(str, num, char) {
		while (str.length < num) {
			str = char + str;
		}
		return str;
	},
};
