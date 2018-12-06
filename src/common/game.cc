#include <screeps/game.h>

namespace screeps {

std::ostream& operator<<(std::ostream& os, const game_object_t& that) {
	return os <<"game_object_t[" <<that.id <<"]";
}

/**
 * game_state_t implementation
 */
construction_site_t* game_state_t::construction_site_by_id(const sid_t& id) {
	return const_cast<construction_site_t*>(const_cast<const game_state_t*>(this)->construction_site_by_id(id));
}

const construction_site_t* game_state_t::construction_site_by_id(const sid_t& id) const {
	auto ii = construction_sites_by_id.find(id);
	if (ii == construction_sites_by_id.end()) {
		return nullptr;
	} else {
		return ii->second;
	}
}

creep_t* game_state_t::creep_by_name(const creep_t::name_t& name) {
	return const_cast<creep_t*>(const_cast<const game_state_t*>(this)->creep_by_name(name));
}

const creep_t* game_state_t::creep_by_name(const creep_t::name_t& name) const {
	auto ii = creeps_by_name.find(name);
	if (ii == creeps_by_name.end()) {
		return nullptr;
	} else {
		return ii->second;
	}
}

dropped_resource_t* game_state_t::dropped_resource_by_id(const sid_t& id) {
	return const_cast<dropped_resource_t*>(const_cast<const game_state_t*>(this)->dropped_resource_by_id(id));
}

const dropped_resource_t* game_state_t::dropped_resource_by_id(const sid_t& id) const {
	auto ii = dropped_resources_by_id.find(id);
	if (ii == dropped_resources_by_id.end()) {
		return nullptr;
	} else {
		return ii->second;
	}
}

source_t* game_state_t::source_by_id(const sid_t& id) {
	return const_cast<source_t*>(const_cast<const game_state_t*>(this)->source_by_id(id));
}

const source_t* game_state_t::source_by_id(const sid_t& id) const {
	auto ii = sources_by_id.find(id);
	if (ii == sources_by_id.end()) {
		return nullptr;
	} else {
		return ii->second;
	}
}

structure_union_t* game_state_t::structure_by_id(const sid_t& id) {
	return const_cast<structure_union_t*>(const_cast<const game_state_t*>(this)->structure_by_id(id));
}

const structure_union_t* game_state_t::structure_by_id(const sid_t& id) const {
	auto ii = structures_by_id.find(id);
	if (ii == structures_by_id.end()) {
		return nullptr;
	} else {
		return ii->second;
	}
}

} // namespace screeps
