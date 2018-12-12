#include <screeps/room.h>
#include <iostream>
using namespace screeps;

int room_t::create_construction_site(local_position_t pos, structure_t::type_t structure_type, const std::string& name) const {
	std::cerr <<location <<".create_construction_site(" <<pos <<", " <<structure_type <<", \"" <<name <<"\")\n";
	return 0;
}
