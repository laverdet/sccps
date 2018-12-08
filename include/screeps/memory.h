#pragma once
#include <cstdint>
#include <stdexcept>
#include <type_traits>

namespace serialization {

// `serialize` implementation for integral types
template <class Memory, typename Number>
typename std::enable_if<std::is_integral<Number>::value>::type serialize(Memory& memory, Number& number) {
	memory.copy(reinterpret_cast<typename std::conditional<std::is_const<Number>::value, const uint8_t, uint8_t>::type*>(&number), sizeof(Number));
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
typename std::enable_if<std::is_same<decltype(std::declval<Type>().read(std::declval<Memory&>())), void>::value>::type write(Memory& memory, Type&& value) {
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

		// Raw buffer read/write

	public:
		memory_t(size_t bytes) : memory(bytes), pos(memory.data()) {}

		int version() const {
			return _version;
		}

		// TODO: Read/write data to screeps RawMemory
		bool load();
		void save() const;
};

class memory_reader_t: public memory_t {
	public:
		using memory_t::memory_t;

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

		// Convert a memory_t instance to a reader
		static auto& reset(memory_t& memory) {
			auto& that = *static_cast<memory_reader_t*>(&memory);
			that.pos = that.memory.data();
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
		memory_writer_t(size_t bytes, int version) : memory_t(bytes) {
			reset(*this, version);
		}

		// Write raw memory
		void copy(const uint8_t* ptr, size_t size) {
			if (pos + size > memory.data() + memory.size()) {
				throw std::range_error("memory_writer_t::write");
			}
			std::memcpy(pos, ptr, size);
			pos += size;
		}

		// Operators
		template <class Type>
		auto& operator<<(Type&& value) {
			serialization::write(*this, std::forward<Type>(value));
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
			that <<k_magic <<k_internal_version <<version;
			that.internal_version = k_internal_version;
			that._version = version;
			return that;
		}
};

} // namespace screeps
