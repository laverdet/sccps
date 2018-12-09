#include <screeps/path-finder.h>

namespace screeps {

path_finder_t::result_t path_finder_t::search(const position_t /* origin */, const std::vector<goal_t>& /* goals */, const options_t& /* options */) {
	std::terminate();
	return {};
}

} // namespace screeps
