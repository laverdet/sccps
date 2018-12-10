#pragma once
#include "./utility.h"
#include <utility>

namespace serialization {

template <class Memory, class First, class Second>
void serialize(Memory& memory, std::pair<First, Second>& pair) {
	memory & const_cast<typename remove_cv_under_ref<First>::type&>(pair.first);
	memory & const_cast<typename remove_cv_under_ref<Second>::type&>(pair.second);
}

} // namespace serialization
