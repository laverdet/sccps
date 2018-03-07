#pragma once
#include <cstdint>

#include "./array.h"
#include "./string.h"

#include "./constants.h"

#include "./creep.h"
#include "./object.h"
#include "./path-finder.h"
#include "./position.h"
#include "./resource.h"
#include "./structure.h"

/**
 * Game state
 */
struct game_state_t {
	array_t<creep_t, kMaximumCreeps> creeps;
	array_t<structure_union_t, kMaximumStructures> structures;

	void clear();
};

extern const game_state_t game;
