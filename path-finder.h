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
		uint8_t plain_cost = 1;
		uint8_t swamp_cost = 5;
		bool flee = false;
		uint16_t max_ops = 2000;
		uint8_t max_rooms = 16;
		uint32_t max_cost = 0x7fffffff;
		double heuristic_weight = 1.2;
		callback_t room_callback = nullptr;
	};

	struct result_t {
		path_ptr_t path;
		uint32_t ops;
		uint32_t cost;
		bool incomplete;
	};

	struct goal_t {
		position_t pos;
		uint8_t range;
	};
	using goals_t = std::vector<goal_t>;

	static result_t search(const position_t origin, const goals_t& goals, const options_t& options);
	static const void* callback_trampoline(void* fn, int xx, int yy);
};

} // namespace screeps
