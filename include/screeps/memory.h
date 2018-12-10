#pragma once
#include "./memory/utility.h"
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <type_traits>

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

template <class Memory, typename Number>
void read(Memory& memory, Number& number, typename serialize_number_t<typename std::decay<Number>::type>::as = 0) {
	typename serialize_number_t<typename std::decay<Number>::type>::as tmp;
	memory.copy(reinterpret_cast<uint8_t*>(&tmp), sizeof(tmp));
	number = tmp;
}

template <class Memory, typename Number>
void write(Memory& memory, Number number, typename serialize_number_t<typename std::decay<Number>::type>::as = 0) {
	typename serialize_number_t<typename std::decay<Number>::type>::as tmp = number;
	memory.copy(reinterpret_cast<const uint8_t*>(&tmp), sizeof(tmp));
}

// enums
template <class Memory, typename Enum>
typename std::enable_if<std::is_enum<typename std::remove_reference<Enum>::type>::value>::type serialize(Memory& memory, Enum&& value) {
	memory.copy(reinterpret_cast<uint8_t*>(&value), sizeof(Enum));
}

// Invoke `serialize` as an instance method on any type
template <class Memory, class Type>
typename std::enable_if<std::is_same<decltype(std::declval<Type>().serialize(std::declval<Memory&>())), void>::value>::type serialize(Memory& memory, Type&& value) {
	std::forward<Type>(value).serialize(memory);
}

// Invoke `serialize` when `read`/`write` are called
template <class Memory, class Type>
typename std::enable_if<std::is_same<decltype(serialize(std::declval<Memory&>(), std::declval<Type>())), void>::value>::type read(Memory& memory, Type&& value) {
	serialization::serialize(memory, std::forward<Type>(value));
}

template <class Memory, class Type>
typename std::enable_if<std::is_same<decltype(serialize(std::declval<Memory&>(), std::declval<Type>())), void>::value>::type write(Memory& memory, Type&& value) {
	serialization::serialize(memory, std::forward<Type>(value));
}

// Invoke `read`/`write` as an instance method on any type
template <class Memory, class Type>
typename std::enable_if<std::is_same<decltype(std::declval<Type>().read(std::declval<Memory&>())), void>::value>::type read(Memory& memory, Type&& value) {
	std::forward<Type>(value).read(memory);
}

template <class Memory, class Type>
typename std::enable_if<std::is_same<decltype(std::declval<Type>().write(std::declval<Memory&>())), void>::value>::type write(Memory& memory, Type&& value) {
	std::forward<Type>(value).write(memory);
}

} // namespace serialization

namespace screeps {

class memory_t {
	protected:
		static constexpr int k_internal_version = 1;
		static constexpr int k_magic = 0xd0414102;
		std::vector<uint8_t> memory;
		uint8_t* pos;
		uint8_t* end;
		int internal_version = 0;
		int _version = 0;

	public:
		memory_t(size_t bytes) : memory(bytes), pos(memory.data()) {}

		operator std::string_view() const {
			return {reinterpret_cast<const char*>(memory.data()), pos - memory.data()};
		}

		int version() const {
			return _version;
		}

		uint8_t* data() {
			return memory.data();
		}

		size_t size() const {
			return memory.size();
		}

		// TODO: Read/write data to screeps RawMemory
		bool load();
		void save() const;
};

class memory_reader_t: public memory_t {
	private:
		// Convenience reader for emplace
		struct reader_t {
			memory_reader_t& reader;
			reader_t(memory_reader_t& reader) : reader(reader) {}
			
			template <class Type>
			operator Type() {
				Type value;
				reader >>value;
				return value;
			}
		};

	public:
		using memory_t::memory_t;
		static constexpr bool is_reader = true;
		static constexpr bool is_writer = false;

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

		// Convert a memory_t instance to a reader
		static auto& reset(memory_t& memory) {
			auto& that = *static_cast<memory_reader_t*>(&memory);
			that.pos = that.memory.data();
			that.end = that.memory.data() + that.memory.size();
			int magic;
			that >>magic >>that.internal_version >>that._version;
			if (magic != k_magic || that.internal_version == 0) {
				that.end = that.pos;
			}
			return that;
		}
};

class memory_writer_t: public memory_t {
	public:
		using memory_t::memory_t;
		static constexpr bool is_reader = false;
		static constexpr bool is_writer = true;

		memory_writer_t(size_t bytes, int version) : memory_t(bytes) {
			reset(*this, version);
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

		// Convert a memory_t instance to a write
		static memory_writer_t& reset(memory_t& memory, int version) {
			auto& that = *static_cast<memory_writer_t*>(&memory);
			that.pos = that.memory.data();
			that.end = that.memory.data() + that.memory.size();
			that <<k_magic <<k_internal_version <<version;
			that.internal_version = k_internal_version;
			that._version = version;
			return that;
		}
};

} // namespace screeps
