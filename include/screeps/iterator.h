#pragma once
#include <iterator>
#include <type_traits>

namespace screeps {

template <typename T>
class forward_iterator_t {
	private:
		constexpr T& that() {
			static_assert(std::is_default_constructible<T>::value, "must be default constructible");
			static_assert(std::is_copy_constructible<T>::value, "Must be copy constructible");
			static_assert(std::is_copy_assignable<T>::value, "Must be copy assignable");
			static_assert(std::is_destructible<T>::value, "Must be destructible");
			static_assert(std::is_swappable<T>::value, "Must be swappable");
			static_assert(std::is_same<typename std::iterator_traits<T>::iterator_category, std::forward_iterator_tag>::value, "Must have iterator traits");
			return static_cast<T&>(*this);
		}

		constexpr const T& that() const {
			return static_cast<const T&>(*this);
		}

	public:
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;

		constexpr bool operator==(const T& rhs);
		constexpr T& operator++();

		constexpr bool operator!=(const T& rhs) const {
			return !(that() == rhs);
		}

		constexpr T operator++(int) {
			T copy(that());
			++that();
			return copy;
		}
};

template <typename T>
class random_access_iterator_t {
	private:
		constexpr T& that() {
			static_assert(std::is_default_constructible<T>::value, "must be default constructible");
			static_assert(std::is_copy_constructible<T>::value, "Must be copy constructable");
			static_assert(std::is_copy_assignable<T>::value, "Must be copy assignable");
			static_assert(std::is_destructible<T>::value, "Must be destructible");
			static_assert(std::is_swappable<T>::value, "Must be swappable");
			static_assert(std::is_class<std::iterator_traits<T>>::value, "Must have iterator traits");
			return static_cast<T&>(*this);
		}

		constexpr const T& that() const {
			return static_cast<const T&>(*this);
		}

	public:
		using iterator_category = std::random_access_iterator_tag;
		using difference_type = std::ptrdiff_t;

		constexpr bool operator==(const T& rhs) const;
		constexpr bool operator<(const T& rhs) const;
		constexpr T& operator+=(int val);
		constexpr T operator+(int val) const;

		constexpr bool operator!=(const T& rhs) const {
			return !(that() == rhs);
		}

		constexpr bool operator>(const T& rhs) const {
			return rhs < that();
		}

		constexpr bool operator<=(const T& rhs) const {
			return !(that() > rhs);
		}

		constexpr bool operator>=(const T& rhs) const {
			return !(that() < rhs);
		}

		constexpr auto&& operator[](int index) const {
			return *(*this + index);
		}

		constexpr T& operator++() {
			return that() += 1;
		}

		constexpr T operator++(int) {
			T copy(that());
			that() += 1;
			return copy;
		}

		constexpr T& operator--() {
			return that() += -1;
		}

		constexpr T operator--(int) {
			T copy(that());
			that() += -1;
			return copy;
		}

		constexpr T operator-(int val) const {
			return that() + -val;
		}

		friend inline constexpr T operator+(const int lhs, const T& rhs) {
			return rhs + lhs;
		}

		friend inline constexpr T operator-(const int lhs, const T& rhs) {
			return rhs + -lhs;
		}
};

template <typename T>
class pointer_container_t {
	private:
		T* _begin;
		T* _end;

	public:
		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using reference = T&;
		using const_reference = const T&;
		using pointer = T*;
		using const_pointer = const T*;
		using iterator = T*;
		using const_iterator = const T*;

		pointer_container_t() {}
		constexpr pointer_container_t(T* begin, T* end) : _begin(begin), _end(end) {}

		constexpr reference operator[](size_type index) {
			return *(_begin + index);
		}

		constexpr const_reference operator[](size_type index) const {
			return *(_begin + index);
		}

		constexpr reference front() {
			return *_begin;
		}

		constexpr const_reference front() const {
			return *_begin;
		}

		constexpr reference back() {
			return *(_end - 1);
		}

		constexpr const_reference back() const {
			return *(_end - 1);
		}

		constexpr pointer data() {
			return _begin;
		}

		constexpr const_pointer data() const {
			return _begin;
		}

		constexpr bool empty() const {
			return _begin == _end;
		}

		constexpr size_type size() const {
			return _end - _begin;
		}

		constexpr pointer begin() {
			return _begin;
		}

		constexpr pointer end() {
			return _end;
		}

		constexpr const_pointer begin() const {
			return _begin;
		}

		constexpr const_pointer end() const {
			return _end;
		}
};

} // namespace screeps
