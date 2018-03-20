#pragma once
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <vector>

namespace screeps {

class memory_t {
	private:
		// Helper class which detects if a type has a constructor that accepts memory_t& or if
		// operator>> should be used.
		template <typename Type>
		struct has_memory_ctor {
			template <typename Ctor>
			static bool has_ctor(decltype(Ctor(*reinterpret_cast<memory_t*>(0)))*);
			template <typename Ctor>
			static void has_ctor(...);
			static constexpr bool value = std::is_same<decltype(has_ctor<Type>(nullptr)), bool>::value;
		};

		static constexpr uint32_t k_internal_version = 1;
		static constexpr uint32_t k_magic = 0xd0414102;
		std::vector<uint8_t> memory;
		uint8_t* pos;
		uint32_t _version = 0;
		uint32_t _internal_version = 0;
	public:
		memory_t(size_t kilobytes) : memory(kilobytes * 1024), pos(memory.data()) {}
		memory_t(size_t kilobytes, uint32_t version) : memory(kilobytes * 1024), pos(memory.data()), _version(version) {
			*this <<k_magic <<k_internal_version <<version;
		}

		uint32_t version() const {
			return _version;
		}

		uint32_t internal_version() const {
			return _internal_version;
		}

		// Read/write data to screeps RawMemory
		bool load();
		void save() const;

		// Reset for writing
		void reset(uint32_t version) {
			pos = memory.data();
			_version = version;
			*this <<k_magic <<_internal_version <<version;
		}

		// Writing integral types
		template <typename Number>
		typename std::enable_if<std::is_integral<Number>::value, memory_t&>::type operator<<(Number value) {
			auto ptr = reinterpret_cast<Number*>(pos);
			*ptr = value;
			pos = reinterpret_cast<uint8_t*>(++ptr);
			return *this;
		}

		// Reading integral types
		template <typename Number>
		typename std::enable_if<std::is_integral<Number>::value, memory_t&>::type operator>>(Number& value) {
			auto ptr = reinterpret_cast<Number*>(pos);
			value = *ptr;
			pos = reinterpret_cast<uint8_t*>(++ptr);
			return *this;
		}

		// Writing raw data
		void write(const uint8_t* ptr, size_t size) {
			std::memcpy(pos, ptr, size);
			ptr += size;
		}

		// Reading raw data
		void read(uint8_t* ptr, size_t size) const {
			std::memcpy(ptr, pos, size);
			ptr += size;
		}

		// Vectors
		template <typename Type>
		memory_t& operator<<(const std::vector<Type>& vector) {
			*this <<vector.size();
			for (auto&& element : vector) {
				*this <<element;
			}
			return *this;
		}
		template <typename Type>
		memory_t& operator>>(std::vector<Type>& vector) {
			size_t size;
			*this >>size;
			vector.reserve(size);
			for (auto&& element : vector) {
				if constexpr (has_memory_ctor<Type>::value) {
					vector.emplace_back(*this);
				} else {
					*this >> vector.emplace_back();
				}
			}
			return *this;
		}
};

} // namespace screeps
