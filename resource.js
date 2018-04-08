'use strict';
const ArrayLib = require('array');
const util = require('util');

const [ resourceEnum, resourceReverseEnum ] = util.enumToMap([
	null,
	RESOURCE_ENERGY,
	RESOURCE_POWER,

	RESOURCE_HYDROGEN,
	RESOURCE_OXYGEN,
	RESOURCE_UTRIUM,
	RESOURCE_LEMERGIUM,
	RESOURCE_KEANIUM,
	RESOURCE_ZYNTHIUM,
	RESOURCE_CATALYST,
	RESOURCE_GHODIUM,

	RESOURCE_HYDROXIDE,
	RESOURCE_ZYNTHIUM_KEANITE,
	RESOURCE_UTRIUM_LEMERGITE,

	RESOURCE_UTRIUM_HYDRIDE,
	RESOURCE_UTRIUM_OXIDE,
	RESOURCE_KEANIUM_HYDRIDE,
	RESOURCE_KEANIUM_OXIDE,
	RESOURCE_LEMERGIUM_HYDRIDE,
	RESOURCE_LEMERGIUM_OXIDE,
	RESOURCE_ZYNTHIUM_HYDRIDE,
	RESOURCE_ZYNTHIUM_OXIDE,
	RESOURCE_GHODIUM_HYDRIDE,
	RESOURCE_GHODIUM_OXIDE,

	RESOURCE_UTRIUM_ACID,
	RESOURCE_UTRIUM_ALKALIDE,
	RESOURCE_KEANIUM_ACID,
	RESOURCE_KEANIUM_ALKALIDE,
	RESOURCE_LEMERGIUM_ACID,
	RESOURCE_LEMERGIUM_ALKALIDE,
	RESOURCE_ZYNTHIUM_ACID,
	RESOURCE_ZYNTHIUM_ALKALIDE,
	RESOURCE_GHODIUM_ACID,
	RESOURCE_GHODIUM_ALKALIDE,

	RESOURCE_CATALYZED_UTRIUM_ACID,
	RESOURCE_CATALYZED_UTRIUM_ALKALIDE,
	RESOURCE_CATALYZED_KEANIUM_ACID,
	RESOURCE_CATALYZED_KEANIUM_ALKALIDE,
	RESOURCE_CATALYZED_LEMERGIUM_ACID,
	RESOURCE_CATALYZED_LEMERGIUM_ALKALIDE,
	RESOURCE_CATALYZED_ZYNTHIUM_ACID,
	RESOURCE_CATALYZED_ZYNTHIUM_ALKALIDE,
	RESOURCE_CATALYZED_GHODIUM_ACID,
	RESOURCE_CATALYZED_GHODIUM_ALKALIDE,
]);

let extendedResourceStorePtr;
let extendedResourceStoreSizeof;

module.exports = {
	resourceEnum,

	init(ptr, sizeof) {
		extendedResourceStorePtr = ptr;
		extendedResourceStoreSizeof = sizeof;
	},

	write(env, ptr, store) {
		let keys = Object.keys(store);
		if (keys.length == 0) {
			env.HEAPU32[ptr >> 2] = 0; // extended
			env.HEAPU32[(ptr + 8) >> 2] = 0; // single_amount
		} else if (keys.length == 1) {
			env.HEAPU32[ptr >> 2] = 0; // extended
			let type = keys[0];
			env.HEAPU32[(ptr + 4) >> 2] = resourceEnum.get(type) | 0; // single_type
			env.HEAPU32[(ptr + 8) >> 2] = store[type] | 0; // single_amount
		} else {
			let extended = ArrayLib.push(env, extendedResourceStorePtr, extendedResourceStoreSizeof);
			env.HEAPU32.fill(extended >> 2, (extended + extendedResourceStoreSizeof) >> 2, 0);
			env.HEAPU32[ptr >> 2] = extended | 0;
			for (let ii = keys.length - 1; ii >= 0; --ii) {
				let key = keys[ii];
				env.HEAPU32[(extended + resourceEnum.get(key) * 4) >> 2] = store[key] | 0;
			}
		}
	},

	jsToC(resourceType) {
		return resourceEnum.get(resourceType);
	},

	cToJs(resourceType) {
		return resourceReverseEnum.get(resourceType);
	}
};
