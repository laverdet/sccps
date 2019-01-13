#include <screeps/game.h>
#include <algorithm>
#include <deque>
#include "./javascript.h"

namespace screeps {

/**
 * game_state_t implementation
 */
void game_state_t::load() {

	// Reset memory for flags and sites
	construction_sites_memory.reset(construction_sites);
	flags_memory.reset(flags);
	for (auto& [location, room] : rooms) {
		room.reset();
	}
	for (auto& [location, room] : extra_rooms) {
		room.reset();
	}

	// Setup pointers for existing rooms
	room_pointers_memory.reset(room_pointers);
	write_room_pointers();
	clear_indices();

	// Pass off to JS
	EM_ASM({
		Module.screeps.object.writeGame(Module, $0);
	}, this);

	// Shrink memory ranges
	construction_sites_memory.shrink(construction_sites);
	flags_memory.shrink(flags);
	room_pointers_memory.shrink(room_pointers);

	// Update room map keys
	// TODO: This can be made faster with unordered_map::extract when that lands in libcxx
	int count = 0;
	for (auto ii = rooms.begin(); ii != rooms.end(); ) {
		if (++count > 10) throw std::runtime_error("uh oh");
		ii->second.shrink();
		if (ii->first == ii->second.location) {
			ii->second.update_pointers();
			++ii;
		} else {
			auto tmp = std::move(ii->second);
			auto location = tmp.location;
			ii = rooms.erase(ii);
			if (location == room_location_t(0)) {
				extra_rooms.emplace(std::piecewise_construct, std::forward_as_tuple(++extra_room_key), std::forward_as_tuple(std::move(tmp)));
			} else {
				auto [it, did_insert] = rooms.emplace(std::piecewise_construct, std::forward_as_tuple(location), std::forward_as_tuple(std::move(tmp)));
				it->second.update_pointers();
			}
		}
	}
	for (auto ii = extra_rooms.begin(); ii != extra_rooms.end(); ) {
		if (++count > 10) throw std::runtime_error("uh oh");
		ii->second.shrink();
		if (ii->second.location == room_location_t(0)) {
			++ii;
		} else {
			auto tmp = std::move(ii->second);
			auto location = tmp.location;
			ii = extra_rooms.erase(ii);
			auto [it, did_insert] = rooms.emplace(std::piecewise_construct, std::forward_as_tuple(location), std::forward_as_tuple(std::move(tmp)));
			it->second.update_pointers();
		}
	}

	// Finalize room state pointers
	update_pointers();
}

void game_state_t::write_room_pointers() {
	// Write out existing room pointers
	auto ii = room_pointers.begin();
	for (auto& [location, room] : rooms) {
		*ii++ = &room;
	}
	// Fill or shrink extra_rooms
	int count = room_pointers_memory.size - rooms.size() - extra_rooms.size();
	while (count > 0) {
		extra_rooms.emplace(std::piecewise_construct, std::forward_as_tuple(++extra_room_key), std::forward_as_tuple());
		--count;
	}
	while (count < 0) {
		extra_rooms.erase(extra_rooms.begin());
		++count;
	}
	// Write out extra room pointers
	for (auto& [location, room] : extra_rooms) {
		*ii++ = &room;
		room.location = room_location_t(0);
	}
	std::sort(room_pointers.begin(), room_pointers.end(), [](room_t* left, room_t* right) {
		return left->location.id < right->location.id;
	});
}

EMSCRIPTEN_KEEPALIVE
void game_state_t::init_layout() {
	EM_ASM({
		Module.screeps.object.initGameLayout({
			'constructionSites': $0,
			'flags': $1,
			'rooms': $2,

			'gcl': $3,
			'time': $4,
		});
	},
		offsetof(game_state_t, construction_sites_memory),
		offsetof(game_state_t, flags_memory),
		offsetof(game_state_t, room_pointers_memory),

		offsetof(game_state_t, gcl),
		offsetof(game_state_t, time)
	);
	creep_t::init();
	room_t::init();
	structure_t::init();
}

EMSCRIPTEN_KEEPALIVE
void game_state_t::ensure_capacity(game_state_t* game) {
	if (game->room_pointers_memory.ensure_capacity(game->room_pointers)) {
		game->write_room_pointers();
	}
	game->construction_sites_memory.ensure_capacity(game->construction_sites);
	game->flags_memory.ensure_capacity(game->flags);
}

// Called in the case of an uncaught exception. This does the `what()` virtual function call and
// also returns RTTI.
EMSCRIPTEN_KEEPALIVE
void* exception_what(void* ptr) {
	struct hole_t {
		const char* name;
		const char* what;
	};
	static hole_t hole;
	auto& err = *reinterpret_cast<const std::exception*>(ptr);
	hole.name = typeid(err).name();
	hole.what = err.what();
	return reinterpret_cast<void*>(&hole);
}

} // namespace screeps

// Keep loop function alive
EMSCRIPTEN_KEEPALIVE void loop();

// Native module loader
#ifndef __EMSCRIPTEN__
#include <nan.h>
#include <isolated_vm.h>
using namespace v8;

NAN_METHOD(mod_make_array_buffer) {
	size_t addr = Nan::To<int64_t>(info[0]).ToChecked();
	auto ab = ArrayBuffer::New(Isolate::GetCurrent(), reinterpret_cast<void*>(addr == 0 ? 0xdeadbeef : addr), 0x100000000);
	info.GetReturnValue().Set(ab);
	if (addr == 0) {
		// v8 won't let you create a buffer starting at address 0 so I cheat.
		auto hack = **(char***)&ab;
		for (int ii = 0; ii < 100; ++ii) {
			auto& value = *(unsigned int*)(hack + ii);
			if (value == 0xdeadbeef) {
				value = 0;
				return;
			}
		}
		throw std::runtime_error("Failed to be sneaky");
	}
}

NAN_METHOD(mod_game_state_init_layout) {
	screeps::game_state_t::init_layout();
}

NAN_METHOD(mod_game_state_ensure_capacity) {
	screeps::game_state_t::ensure_capacity(reinterpret_cast<screeps::game_state_t*>(Nan::To<int64_t>(info[0]).ToChecked()));
}

NAN_METHOD(mod_room_ensure_capacity) {
	screeps::room_t::ensure_capacity(reinterpret_cast<screeps::room_t*>(Nan::To<int64_t>(info[0]).ToChecked()));
}

NAN_METHOD(mod_loop) {
	loop();
}

ISOLATED_VM_MODULE void InitForContext(Isolate* isolate, Local<Context> context, Local<Object> target) {
	Nan::SetMethod(target, "makeArrayBuffer", mod_make_array_buffer);
	Nan::SetMethod(target, "__ZN7screeps12game_state_t11init_layoutEv", mod_game_state_init_layout);
	Nan::SetMethod(target, "__ZN7screeps12game_state_t15ensure_capacityEPS0_", mod_game_state_ensure_capacity);
	Nan::SetMethod(target, "__ZN7screeps6room_t15ensure_capacityEPS0_", mod_room_ensure_capacity);
	Nan::SetMethod(target, "__Z4loopv", mod_loop);
}
#endif
