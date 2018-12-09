#pragma once
#include "./array.h"
#include "./constants.h"
#include "./creep.h"
#include "./object.h"
#include "./position.h"
#include "./room.h"
#include "./structure.h"
#include "./internal/memory.h"
#include <cstdint>
#include <unordered_map>

namespace screeps {

/**
 * Game state
 */
class game_state_t {
	private:
		template <class Type>
		using container_t = typename internal::memory_range_t<Type>::container_t;

		// Index class for `_by_id` methods
		template <class Identifier, class Base, class Type, Identifier Base::*Property>
		class index_t {
			protected:
				mutable std::unordered_map<Identifier, Type*> index;
				mutable bool did_index;

				Type* find_in_map(const Identifier& id) {
					auto ii = index.find(id);
					if (ii == index.end()) {
						return nullptr;
					} else {
						return ii->second;
					}
				}

			public:
				void clear() {
					did_index = false;
					index.clear();
				}
		};

		template <class Identifier, class Base, class Type, Identifier Base::*Property>
		class vector_index_t: public index_t<Identifier, Base, Type, Property> {
			private:
				template <class Vector>
				void ensure_index(Vector& vector) const {
					if (!this->did_index) {
						this->did_index = true;
						for (auto& element : vector) {
							this->index.emplace(static_cast<const Base&>(element).*Property, const_cast<Type*>(&element));
						}
					}
				}

			public:
				template <class Vector>
				Type* find(Vector& vector, const Identifier& id) {
					ensure_index(vector);
					return this->find_in_map(id);
				}

				template <class Vector>
				const Type* find(Vector& vector, const Identifier& id) const {
					return const_cast<vector_index_t*>(this)->find(vector, id);
				}
		};

		template <class Identifier, class Base, class Type, Identifier Base::*Property, auto room_t::*Vector>
		class room_index_t: public index_t<Identifier, Base, Type, Property> {
			private:
				template <class Rooms>
				void ensure_index(Rooms& rooms) const {
					if (!this->did_index) {
						this->did_index = true;
						for (auto& [location, room] : rooms) {
							for (auto& element : room.*Vector) {
								this->index.emplace(static_cast<const Base&>(element).*Property, const_cast<Type*>(&element));
							}
						}
					}
				}

			public:
				template <class Rooms>
				Type* find(Rooms& rooms, const Identifier& id) {
					ensure_index(rooms);
					return this->find_in_map(id);
				}

				template <class Rooms>
				const Type* find(Rooms& rooms, const Identifier& id) const {
					return const_cast<room_index_t*>(this)->find(rooms, id);
				}
		};

		// Room pointers for JS
		container_t<room_t*> room_pointers;
		internal::memory_range_t<room_t*> room_pointers_memory;
		std::unordered_map<room_location_t, room_t> extra_rooms;
		int32_t extra_room_key = 0;

		// Memory ranges for JS
		internal::memory_range_t<construction_site_t> construction_sites_memory;
		internal::memory_range_t<flag_t> flags_memory;

		// Indices for `_by_id` functions
		vector_index_t<sid_t, game_object_t, construction_site_t, &construction_site_t::id> construction_sites_by_id;
		room_index_t<sid_t, game_object_t, creep_t, &creep_t::id, &room_t::creeps> creeps_by_id;
		room_index_t<creep_t::name_t, creep_t, creep_t, &creep_t::name, &room_t::creeps> creeps_by_name;
		room_index_t<sid_t, game_object_t, dropped_resource_t, &dropped_resource_t::id, &room_t::dropped_resources> dropped_resources_by_id;
		room_index_t<sid_t, game_object_t, source_t, &source_t::id, &room_t::sources> sources_by_id;
		room_index_t<sid_t, game_object_t, structure_union_t, &structure_t::id, &room_t::structures> structures_by_id;
		room_index_t<sid_t, game_object_t, tombstone_t, &tombstone_t::id, &room_t::tombstones> tombstones_by_id;

	public:
		int32_t gcl;
		int32_t time;

		std::unordered_map<room_location_t, room_t> rooms;

		container_t<construction_site_t> construction_sites;
		container_t<flag_t> flags;

	private:
		static void init_layout();
		static void ensure_capacity(game_state_t* game);
		void update_pointers();
		void write_room_pointers();

	public:
		template <class Memory>
		void serialize(Memory& memory) {
			memory & gcl & time;
			memory & rooms;
		}

		void load();

		construction_site_t* construction_site_by_id(const sid_t& id) {
			return construction_sites_by_id.find(construction_sites, id);
		}
		const construction_site_t* construction_site_by_id(const sid_t& id) const {
			return construction_sites_by_id.find(construction_sites, id);
		}

		creep_t* creep_by_name(const creep_t::name_t& name) {
			return creeps_by_name.find(rooms, name);
		}
		const creep_t* creep_by_name(const creep_t::name_t& name) const {
			return creeps_by_name.find(rooms, name);
		}

		dropped_resource_t* dropped_resource_by_id(const sid_t& id) {
			return dropped_resources_by_id.find(rooms, id);
		}
		const dropped_resource_t* dropped_resource_by_id(const sid_t& id) const {
			return dropped_resources_by_id.find(rooms, id);
		}

		source_t* source_by_id(const sid_t& id) {
			return sources_by_id.find(rooms, id);
		}
		const source_t* source_by_id(const sid_t& id) const {
			return sources_by_id.find(rooms, id);
		}

		structure_union_t* structure_by_id(const sid_t& id) {
			return structures_by_id.find(rooms, id);
		}
		const structure_union_t* structure_by_id(const sid_t& id) const {
			return structures_by_id.find(rooms, id);
		}
};

void* exception_what(void* ptr);

} // namespace screeps
