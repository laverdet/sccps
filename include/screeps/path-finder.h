#pragma once
#include "./array.h"
#include "./constants.h"
#include "./position.h"
#include <memory>
#include <vector>

namespace screeps {

using cost_matrix_t = local_matrix_t<uint8_t>;

struct path_finder_t {
	using path_t = array_t<position_t, kMaximumPathLength>;
	using path_ptr_t = std::unique_ptr<path_t>;
	using callback_t = std::function<const cost_matrix_t*(room_location_t)>;

	struct options_t {
		int plain_cost = 1;
		int swamp_cost = 5;
		bool flee = false;
		int max_ops = 2000;
		int max_rooms = 16;
		int max_cost = 0x7fffffff;
		double heuristic_weight = 1.2;
		callback_t room_callback = nullptr;
	};

	struct result_t {
		path_ptr_t path;
		int32_t ops;
		int32_t cost;
		bool incomplete;
	};

	struct goal_t {
		position_t pos;
		int32_t range;
	};
	using goals_t = std::vector<goal_t>;

	static result_t search(const position_t origin, const goals_t& goals, const options_t& options);
	static const void* callback_trampoline(void* fn, int xx, int yy);
};

} // namespace screeps
