#include <screeps/structure.h>
#include <iostream>

namespace screeps {

std::ostream& operator<<(std::ostream& os, const structure_t& that) {
	os <<"structure_t[";
	switch (that.type) {
		case structure_t::container: os <<"container"; break;
		case structure_t::controller: os <<"controller"; break;
		case structure_t::extension: os <<"extension"; break;
		case structure_t::extractor: os <<"extractor"; break;
		case structure_t::keeper_lair: os <<"keeper_lair"; break;
		case structure_t::lab: os <<"lab"; break;
		case structure_t::link: os <<"link"; break;
		case structure_t::nuker: os <<"nuker"; break;
		case structure_t::observer: os <<"observer"; break;
		case structure_t::portal: os <<"portal"; break;
		case structure_t::power_bank: os <<"power_bank"; break;
		case structure_t::power_spawn: os <<"power_spawn"; break;
		case structure_t::rampart: os <<"rampart"; break;
		case structure_t::road: os <<"road"; break;
		case structure_t::spawn: os <<"spawn"; break;
		case structure_t::storage: os <<"storage"; break;
		case structure_t::terminal: os <<"terminal"; break;
		case structure_t::tower: os <<"tower"; break;
		case structure_t::wall: os <<"wall"; break;
	}
	os <<", [" <<that.pos.room_location() <<" " <<that.pos.xx % 50 <<", " <<that.pos.yy % 50 <<"], " <<that.id <<"]";
	return os;
}

} // namespace screeps
