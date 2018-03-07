#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>
#include "game.h"

// Singleton!
const game_state_t game;

/**
 * game_t implementation
 */
void game_state_t::clear() {
	creeps.clear();
	structures.clear();
}

void init() {
	creep_t::init();
	resource_store_t::init();
	structure_t::init();
}

void preloop() {
	resource_store_t::preloop();
	const_cast<game_state_t&>(game).clear();
	EM_ASM({
		Module.screeps.game.read(Module, $0, $1);
	}, &game.creeps, &game.structures);
}

EMSCRIPTEN_BINDINGS(screeps) {
	emscripten::function("init", &init);
	emscripten::function("preloop", &preloop);
}
