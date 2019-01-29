#pragma once
#include "./memory/utility.h"
#include <screeps/constants.h>
#include <bitset>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <vector>

namespace serialization {

// `serialize` implementation for numeric types
template <typename Number>
struct serialize_number_t;

template <> struct serialize_number_t<size_t> { using as = uint32_t; };
template <> struct serialize_number_t<int> { using as = int32_t; };
template <> struct serialize_number_t<unsigned int> { using as = uint32_t; };
template <> struct serialize_number_t<char> { using as = int8_t; };
template <> struct serialize_number_t<unsigned char> { using as = uint8_t; };
template <> struct serialize_number_t<bool> { using as = uint8_t; };

/*
// Simple, but can't test natively on 64-bit
template <class Memory, typename Number>
typename std::enable_if<std::is_arithmetic<typename std::remove_reference<Number>::type>::value>::type serialize(Memory& memory, Number&& number) {
	memory.copy(reinterpret_cast<uint8_t*>(&number), sizeof(Number));
}
*/

template <class Memory, class Number>
void read(Memory& memory, Number& number, typename serialize_number_t<std::decay_t<Number>>::as /* sfinae */ = 0) {
	typename serialize_number_t<std::decay_t<Number>>::as tmp;
	memory.copy(reinterpret_cast<uint8_t*>(&tmp), sizeof(tmp));
	number = tmp;
}

template <class Memory, class Number>
void write(Memory& memory, Number number, typename serialize_number_t<std::decay_t<Number>>::as /* sfinae */ = 0) {
	typename serialize_number_t<std::decay_t<Number>>::as tmp = number;
	memory.copy(reinterpret_cast<const uint8_t*>(&tmp), sizeof(tmp));
}

// enums
template <class Memory, class Enum>
std::enable_if_t<std::is_enum_v<std::remove_reference_t<Enum>>> serialize(Memory& memory, Enum&& value) {
	memory.copy(reinterpret_cast<uint8_t*>(&value), sizeof(Enum));
}

// Invoke `serialize` as an instance method on any type
template <class Memory, class Type>
std::enable_if_t<std::is_same_v<decltype(std::declval<Type>().serialize(std::declval<Memory&>())), void>> serialize(Memory& memory, Type&& value) {
	std::forward<Type>(value).serialize(memory);
}

// Invoke `serialize` when `read`/`write` are called
template <class Memory, class Type>
std::enable_if_t<std::is_same_v<decltype(serialize(std::declval<Memory&>(), std::declval<Type>())), void>> read(Memory& memory, Type&& value) {
	serialization::serialize(memory, std::forward<Type>(value));
}

template <class Memory, class Type>
std::enable_if_t<std::is_same_v<decltype(serialize(std::declval<Memory&>(), std::declval<Type>())), void>> write(Memory& memory, Type&& value) {
	serialization::serialize(memory, std::forward<Type>(value));
}

// Invoke `read`/`write` as an instance method on any type
template <class Memory, class Type>
std::enable_if_t<std::is_same_v<decltype(std::declval<Type>().read(std::declval<Memory&>())), void>> read(Memory& memory, Type&& value) {
	std::forward<Type>(value).read(memory);
}

template <class Memory, class Type>
 std::enable_if_t<std::is_same_v<decltype(std::declval<Type>().write(std::declval<Memory&>())), void>> write(Memory& memory, Type&& value) {
	std::forward<Type>(value).write(memory);
}

// Array types
template <class Memory, class Type, size_t Count>
void read(Memory& memory, Type (&value)[Count]) {
	for (size_t ii = 0; ii < Count; ++ii) {
		serialization::read(memory, value[ii]);
	}
}

template <class Memory, class Type, size_t Count>
void write(Memory& memory, Type (&value)[Count]) {
	for (size_t ii = 0; ii < Count; ++ii) {
		serialization::write(memory, value[ii]);
	}
}

} // namespace serialization

namespace screeps {
class raw_memory_t;

// Base class for memory access, shouldn't be used directly
class memory_t {
	friend raw_memory_t;
	public:
		explicit memory_t(size_t bytes) : memory(bytes), pos(memory.data()) {}

		size_t capacity() const {
			return memory.size();
		}

		uint8_t* data() {
			return memory.data();
		}

		size_t size() const {
			return end - pos;
		}

		int version() const {
			return _version;
		}

	protected:
		static constexpr int32_t k_internal_version = 2;
		static constexpr int32_t k_magic = 0x8af88ecd;
		std::vector<uint8_t> memory;
		uint8_t* pos;
		uint8_t* end;
		int32_t internal_version = 0;
		int32_t _version = 0;
};

class memory_reader_t : public memory_t {
	public:
		using memory_t::memory_t;
		explicit operator std::string_view() const { return {reinterpret_cast<const char*>(memory.data()), size()}; }

		// Read raw memory
		void copy(uint8_t* ptr, size_t size) {
			if (pos + size > end) {
				throw std::range_error("memory_reader_t::copy");
			}
			std::memcpy(ptr, pos, size);
			pos += size;
		}

		// Operators
		template <class Type>
		auto& operator>>(Type& value) {
			serialization::read(*this, value);
			return *this;
		}

		template <class Type>
		auto& operator&(Type&& value) {
			return *this >>std::forward<Type>(value);
		}

		auto read() {
			return reader_t(*this);
		}

		bool reset(size_t size) {
			// Confirm validity and setup pointers
			if (size < 16) {
				return false;
			}
			pos = data();
			end = pos + size;
			int32_t magic;
			uint32_t payload_size;
			*this >>magic >>payload_size >>internal_version >>_version;
			if (magic != memory_t::k_magic || payload_size > size || internal_version != k_internal_version) {
				return false;
			}
			end = data() + payload_size;
			return true;
		}

		static constexpr bool is_reader = true;
		static constexpr bool is_writer = false;

	private:
		// Convenience reader for emplace
		struct reader_t {
			memory_reader_t& reader;
			explicit reader_t(memory_reader_t& reader) : reader(reader) {}

			template <class Type>
			operator Type() { // NOLINT(hicpp-explicit-conversions)
				Type value;
				reader >>value;
				return value;
			}
		};
};

class memory_writer_t: public memory_t {
	friend raw_memory_t;
	public:
		memory_writer_t(size_t bytes, int version) : memory_t(bytes) {
			reset(version);
		}

		explicit operator std::string_view() {
			*(reinterpret_cast<uint32_t*>(data()) + 1) = end - pos;
			return {reinterpret_cast<const char*>(memory.data()), size()};
		}

		// Write raw memory
		void copy(const uint8_t* ptr, size_t size) {
			if (pos + size > end) {
				throw std::range_error("memory_writer_t::write");
			}
			std::memcpy(pos, ptr, size);
			pos += size;
		}

		// Operators
		template <class Type>
		auto& operator<<(Type&& value) {
			using non_const = typename serialization::remove_cv_under_ref<Type>::type;
			serialization::write(*this, std::forward<non_const>(const_cast<non_const&&>(value)));
			return *this;
		}

		template <class Type>
		auto& operator&(Type&& value) {
			return *this <<std::forward<Type>(value);
		}

		// Reset this writer
		void reset(int version) {
			pos = data();
			end = pos + capacity();
			*this <<k_magic <<(uint32_t)0 <<k_internal_version <<(int32_t)version;
		}

		static constexpr bool is_reader = false;
		static constexpr bool is_writer = true;
};

class raw_memory_t {
	friend class game_state_t;
	private:
		struct segments_t {
			friend raw_memory_t;
			public:
				const int32_t* begin() const {
					return &active_segments[0];
				}
				const int32_t* end() const {
					return &active_segments[active_segment_count + 1];
				}
				size_t size() const {
					return active_segment_count;
				}

			private:
				int32_t active_segment_count = 0;
				int32_t active_segments[k_memory_max_active_segments];
		};

	public:
		bool load(memory_reader_t& reader, int segment = -1) const;
		bool save(memory_writer_t& writer, int segment = -1) const;
		void set_active_segments(const int* begin, const int* end) const;
		segments_t segments;

		void reset() {
			segments.active_segment_count = 0;
			count_saved = 0;
			saved_segments.reset();
		}

		mutable int count_saved = 0;
		mutable std::bitset<k_memory_segment_count> saved_segments;
};

} // namespace screeps
