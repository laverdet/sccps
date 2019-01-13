#include <screeps/creep.h>
#include <iostream>

namespace screeps {

int creep_t::static_method::build(const sid_t& creep, const sid_t& target) {
	std::cerr <<creep <<".build(" <<target <<")\n";
	return 0;
}

int creep_t::static_method::cancel_order(const sid_t& creep, const char* method) {
	std::cerr <<creep <<".cancel_order(" <<method <<")\n";
	return 0;
}

int creep_t::static_method::drop(const sid_t& creep, resource_t resource, int amount) {
	std::cerr <<creep <<".drop(" <<resource <<", " <<amount <<")\n";
	return 0;
}

int creep_t::static_method::harvest(const sid_t& creep, const sid_t& target) {
	std::cerr <<creep <<".harvest(" <<target <<")\n";
	return 0;
}

int creep_t::static_method::move(const sid_t& creep, direction_t direction) {
	std::cerr <<creep <<".move(" <<direction <<")\n";
	return 0;
}

int creep_t::static_method::pickup(const sid_t& creep, const sid_t& target) {
	std::cerr <<creep <<".pickup(" <<target <<")\n";
	return 0;
}

int creep_t::static_method::repair(const sid_t& creep, const sid_t& target) {
	std::cerr <<creep <<".repair(" <<target <<")\n";
	return 0;
}

int creep_t::static_method::suicide(const sid_t& creep) {
	std::cerr <<creep <<".suicide()\n";
	return 0;
}

int creep_t::static_method::transfer(const sid_t& creep, const sid_t& target, resource_t resource, int amount) {
	std::cerr <<creep <<".transfer(" <<target <<", " <<resource <<", " <<amount <<")\n";
	return 0;
}

int creep_t::static_method::upgrade_controller(const sid_t& creep, const sid_t& target) {
	std::cerr <<creep <<".upgrade_controller(" <<target <<")\n";
	return 0;
}

int creep_t::static_method::withdraw(const sid_t& creep, const sid_t& target, resource_t resource, int amount) {
	std::cerr <<creep <<".transfer(" <<target <<", " <<resource <<", " <<amount <<")\n";
	return 0;
}

} // namespace screeps
