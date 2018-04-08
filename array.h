#pragma once
#include <array>
#include <cstdint>

namespace screeps {

template<typename Type, int Capacity>
struct array_t {
	public:
		using A = std::array<Type, Capacity>;
		using value_type = typename A::value_type;
		using size_type = uint32_t;
		using difference_type = typename A::difference_type;
		using reference = typename A::reference;
		using const_reference = typename A::const_reference;
		using pointer = typename A::pointer;
		using const_pointer = typename A::const_pointer;
		using iterator = typename A::iterator;
		using const_iterator = typename A::const_iterator;
		using reverse_iterator = typename A::reverse_iterator;
		using const_reverse_iterator = typename A::const_reverse_iterator;

	private:
		uint32_t size_ = 0;
		A store{};

	public:
		array_t() = default;

		/*
		array_t(const array_t& that) : size_(that.size_) {
			std::copy(that.begin(), that.end(), begin());
		}

		array_t& operator=(const array_t& that) {
			size_ = that.size_;
			std::copy(that.begin(), that.end(), begin());
		}
		*/

		array_t(size_type size, const_pointer data) : size_(size) {
			std::copy_n(data, size, store.begin());
		}

		array_t(std::initializer_list<Type> list) : size_(list.size()), store(reinterpret_cast<A const&>(*(list.begin()))) {
		}

		size_type size() const {
			return size_;
		}

		void resize(size_type size) {
			size_ = size;
		}

		void clear() {
			size_ = 0;
		}

		template <class... Args>
		reference emplace_back(Args&&... args) {
			return store[size_++] = Type(std::forward<Args>(args)...);
		}

		constexpr Type* data() noexcept {
			return store.data();
		}

		constexpr const Type* data() const noexcept {
			return store.data();
		}

		static constexpr size_type capacity() {
			return Capacity;
		}

		constexpr reference operator[](size_type pos) {
			return store[pos];
		}

		constexpr const_reference operator[](size_type pos) const {
			return store[pos];
		}

		constexpr iterator begin() noexcept {
			return store.begin();
		}

		constexpr const_iterator begin() const noexcept {
			return store.begin();
		}

		constexpr const_iterator cbegin() const noexcept {
			return store.cbegin();
		}

		constexpr iterator end() noexcept {
			return store.begin() + size_;
		}

		constexpr const_iterator end() const noexcept {
			return store.begin() + size_;
		}

		constexpr const_iterator cend() const noexcept {
			return store.cbegin() + size_;
		}
};

} // namespace screeps
