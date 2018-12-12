#pragma once

namespace screeps {

// Constants which affect memory layout and maximum entities
constexpr int kMaximumIdLength = 24;
constexpr int kMaximumPathLength = 1500;

// Screeps constants
constexpr int kMaxCreepSize = 50;
constexpr int kMaxFlagNameLength = 60;
constexpr int k_world_size = 256;
constexpr int k_world_size2 = k_world_size >> 1;

// Error codes
constexpr int k_ok = 0;
constexpr int k_err_not_owner = -1;
constexpr int k_err_no_path = -2;
constexpr int k_err_name_exists = -3;
constexpr int k_err_busy = -4;
constexpr int k_err_not_found = -5;
constexpr int k_err_not_enough_energy = -6;
constexpr int k_err_not_enough_resources = -6;
constexpr int k_err_invalid_target = -7;
constexpr int k_err_full = -8;
constexpr int k_err_not_in_range = -9;
constexpr int k_err_invalid_args = -10;
constexpr int k_err_tired = -11;
constexpr int k_err_no_bodypart = -12;
constexpr int k_err_not_enough_extensions = -6;
constexpr int k_err_rcl_not_enough = -14;
constexpr int k_err_gcl_not_enough = -15;

constexpr int k_controller_levels[] = {-1, 200, 45000, 135000, 405000, 1215000, 3645000, 10935000, -1};
constexpr int k_controller_structures[][9] = {
	// none
	{-1, -1, -1, -1, -1, -1, -1, -1, -1},
	// container
	{5, 5, 5, 5, 5, 5, 5, 5, 5},
	// controller
	{1, 1, 1, 1, 1, 1, 1, 1, 1},
	// extension
	{0, 0, 5, 10, 20, 30, 40, 50, 60},
	// extractor
	{0, 0, 0, 0, 0, 0, 1, 1, 1},
	// keeper_lair
	{1, 1, 1, 1, 1, 1, 1, 1, 1},
	// lab
	{0, 0, 0, 0, 0, 0, 3, 6, 10},
	// link
	{0, 0, 0, 0, 0, 2, 3, 4, 6},
	// nuker
	{0, 0, 0, 0, 0, 0, 0, 0, 1},
	// observer
	{0, 0, 0, 0, 0, 0, 0, 0, 1},
	// portal
	{0, 0, 0, 0, 0, 0, 0, 0, 0},
	// power_bank
	{0, 0, 0, 0, 0, 0, 0, 0, 0},
	// power_spawn
	{0, 0, 0, 0, 0, 0, 0, 0, 1},
	// rampart
	{0, 0, 2500, 2500, 2500, 2500, 2500, 2500, 2500},
	// road
	{2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500, 2500},
	// spawn
	{0, 1, 1, 1, 1, 1, 1, 2, 3},
	// storage
	{0, 0, 0, 0, 1, 1, 1, 1, 1},
	// terminal
	{0, 0, 0, 0, 0, 0, 1, 1, 1},
	// tower
	{0, 0, 0, 1, 1, 2, 2, 3, 6},
	// wall
	{0, 0, 2500, 2500, 2500, 2500, 2500, 2500, 2500},
};

} // namespace screeps
