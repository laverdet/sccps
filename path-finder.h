#pragma once
#include "./array.h"
#include "./constants.h"
#include "./position.h"
#include <memory>
#include <vector>

struct cost_matrix_t {
	uint8_t bits[2500];

	uint8_t get(uint8_t xx, uint8_t yy) {
		return bits[xx * 50 + yy];
	}

	void set(uint8_t xx, uint8_t yy, uint8_t cost) {
		bits[xx * 50 + yy] = cost;
	}
};

struct path_finder_t {
	using path_t = array_t<position_t, kMaximumPathLength>;
	using path_ptr_t = std::unique_ptr<path_t>;
	using callback_t = std::shared_ptr<cost_matrix_t>(*)(room_location_t);

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

	static result_t search(const position_t origin, const std::vector<goal_t>& goals, const options_t& options);
	static void* callback_trampoline(void* fn, int xx, int yy);
};
