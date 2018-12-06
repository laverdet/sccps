#include <screeps/resource.h>

namespace screeps {

std::ostream& operator<<(std::ostream& os, resource_t type) {
	os <<"resource_t::";
	switch (type) {
		case resource_t::none: return os <<"none";
		case resource_t::energy: return os <<"energy";
		case resource_t::power: return os <<"power";
		case resource_t::H: return os <<"H";
		case resource_t::O: return os <<"O";
		case resource_t::U: return os <<"U";
		case resource_t::L: return os <<"L";
		case resource_t::K: return os <<"K";
		case resource_t::Z: return os <<"Z";
		case resource_t::X: return os <<"X";
		case resource_t::G: return os <<"G";
		case resource_t::OH: return os <<"OH";
		case resource_t::ZK: return os <<"ZK";
		case resource_t::UL: return os <<"UL";
		case resource_t::UH: return os <<"UH";
		case resource_t::UO: return os <<"UO";
		case resource_t::KH: return os <<"KH";
		case resource_t::KO: return os <<"KO";
		case resource_t::LH: return os <<"LH";
		case resource_t::LO: return os <<"LO";
		case resource_t::ZH: return os <<"ZH";
		case resource_t::ZO: return os <<"ZO";
		case resource_t::GH: return os <<"GH";
		case resource_t::GO: return os <<"GO";
		case resource_t::UH2O: return os <<"UH2O";
		case resource_t::UHO2: return os <<"UHO2";
		case resource_t::KH2O: return os <<"KH2O";
		case resource_t::KHO2: return os <<"KHO2";
		case resource_t::LH2O: return os <<"LH2O";
		case resource_t::LHO2: return os <<"LHO2";
		case resource_t::ZH2O: return os <<"ZH2O";
		case resource_t::ZHO2: return os <<"ZHO2";
		case resource_t::GH2O: return os <<"GH2O";
		case resource_t::GHO2: return os <<"GHO2";
		case resource_t::XUH2O: return os <<"XUH2O";
		case resource_t::XUHO2: return os <<"XUHO2";
		case resource_t::XKH2O: return os <<"XKH2O";
		case resource_t::XKHO2: return os <<"XKHO2";
		case resource_t::XLH2O: return os <<"XLH2O";
		case resource_t::XLHO2: return os <<"XLHO2";
		case resource_t::XZH2O: return os <<"XZH2O";
		case resource_t::XZHO2: return os <<"XZHO2";
		case resource_t::XGH2O: return os <<"XGH2O";
		case resource_t::XGHO2: return os <<"XGHO2";
		case resource_t::size: return os <<"size";
	}
	return os <<static_cast<int>(type);
}

void resource_store_t::preloop() {
	extended_stores.clear();
}

} // namespace screeps
