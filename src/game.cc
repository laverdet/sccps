#include <screeps/game.h>
#include <algorithm>
#include "./javascript.h"

namespace screeps {

//
// game_state_t implementation
EMSCRIPTEN_KEEPALIVE
void game_state_t::init_layout() {
	EM_ASM({
		Module.screeps.object.initGameLayout({
			'constructionSites': $0,
			'flags': $1,
			'rooms': $2,
			'memory': $3,

			'gcl': $4,
			'time': $5,
		});
	},
		offsetof(game_state_t, construction_sites_memory),
		offsetof(game_state_t, flags_memory),
		offsetof(game_state_t, room_pointers_memory),
		offsetof(game_state_t, memory),

		offsetof(game_state_t, gcl),
		offsetof(game_state_t, time)
	);
	creep_t::init();
	flag_t::init();
	room_t::init();
	structure_t::init();
}

void game_state_t::clear_indices() {
	construction_sites_by_id.clear();
	creeps_by_id.clear();
	creeps_by_name.clear();
	dropped_resources_by_id.clear();
	flags_by_name.clear();
	sources_by_id.clear();
	structures_by_id.clear();
	tombstones_by_id.clear();
	memory.reset();
}

EMSCRIPTEN_KEEPALIVE
void game_state_t::ensure_capacity(game_state_t* game) {
	if (game->room_pointers_memory.ensure_capacity(game->room_pointers)) {
		game->write_room_pointers();
	}
	game->construction_sites_memory.ensure_capacity(game->construction_sites);
	game->flags_memory.ensure_capacity(game->flags);
}

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
			if (location == room_location_t::null) {
				++extra_room_key;
				extra_rooms.emplace(std::piecewise_construct, std::tuple(extra_room_key % 0xff, extra_room_key / 0xff), std::forward_as_tuple(std::move(tmp)));
			} else {
				auto [it, did_insert] = rooms.emplace(std::piecewise_construct, std::forward_as_tuple(location), std::forward_as_tuple(std::move(tmp)));
				it->second.update_pointers();
			}
		}
	}
	for (auto ii = extra_rooms.begin(); ii != extra_rooms.end(); ) {
		if (++count > 10) throw std::runtime_error("uh oh");
		ii->second.shrink();
		if (ii->second.location == room_location_t::null) {
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

void game_state_t::update_pointers() {
	update_pointer_container<&room_t::construction_sites>(construction_sites);
	update_pointer_container<&room_t::flags>(flags);
}

template <auto Property, class Container>
void game_state_t::update_pointer_container(Container& container) {
	typename Container::value_type* current = nullptr;
	room_location_t current_location;
	for (auto& object : container) {
		auto location = object.pos.room;
		if (current == nullptr) {
			current = &object;
			current_location = location;
		}
		if (&object == &container.back() || location != current_location) {
			auto ii = rooms.find(object.pos.room);
			if (ii != rooms.end()) {
				auto& [location, room] = *ii;
				room.*Property = {current, &object + 1};
				current = nullptr;
			}
		}
	}
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
		++extra_room_key;
		extra_rooms.emplace(std::piecewise_construct, std::tuple(extra_room_key % 0xff, extra_room_key / 0xff), std::forward_as_tuple());
		--count;
	}
	while (count < 0) {
		extra_rooms.erase(extra_rooms.begin());
		++count;
	}
	// Write out extra room pointers
	for (auto& [location, room] : extra_rooms) {
		*ii++ = &room;
		room.location = room_location_t::null;
	}
	std::sort(room_pointers.begin(), room_pointers.end(), [](room_t* left, room_t* right) {
		return left->location < right->location;
	});
}

std::ostream& operator<<(std::ostream& os, const sid_t& that) {
	int nibble = 32 - that.length;
	do {
		int val = ((that.bytes[(nibble >> 3) - 1]) >> ((nibble & 0x07) << 2)) & 0x0f;
		os <<static_cast<char>(val + (val > 9 ? 'a' - 10 : '0'));
	} while (++nibble < 32);
	return os;
}

std::ostream& operator<<(std::ostream& os, const game_object_t& that) {
	return os <<"game_object_t[" <<that.id <<"]";
}

} // namespace screeps
