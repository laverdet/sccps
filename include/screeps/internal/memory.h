#pragma once
#include <vector>

namespace screeps::internal {

// Convert value initialization to default initialization
// http://stackoverflow.com/a/21028912/273767
template <class Type, class Allocator = std::allocator<Type>>
class default_init_allocator : public Allocator {
	private:
		using traits = std::allocator_traits<Allocator>;

	public:
		using Allocator::Allocator;

		template <class That>
		struct rebind {
			using other = default_init_allocator<That, class traits::template rebind_alloc<That>>;
		};

		template <class That>
		void construct(That* ptr) noexcept(std::is_nothrow_default_constructible<That>::value) {
			::new(static_cast<void*>(ptr)) That;
		}

		template <class That, class... Args>
		void construct(That* ptr, Args&&... args) {
			traits::construct(static_cast<Allocator&>(*this), ptr, std::forward<Args>(args)...);
		}
};

template <class Type>
struct memory_range_t {
	using container_t = std::vector<Type, default_init_allocator<Type>>;

	uint32_t size = 0;
	Type* data = nullptr;

	// JS will write how big it wants the vector to be to `size` and this function picks that up and
	// resizes the vector if needed. This may not be called on `load` if the room's vectors are all
	// already big enough
	bool ensure_capacity(container_t& container) {
		if (size > container.size()) {
			container.resize(0);
			container.resize(size);
			data = container.data();
			return true;
		} else {
			return false;
		}
	}

	// This is called before handing off control to JS for `load`. It notes the total capacity of the
	// vector in `size`, and maybe shrinks the vector if it's too big.
	void reset(container_t& container) {
		if (container.size() * 2 < container.capacity() && container.size() * sizeof(Type) > 128) {
			// If the container is more than 1kb and is more than twice as big as it needs to be it will
			// resize to be only 25% bigger than needed.
			container.resize(0);
			container.resize(container.size() + container.size() / 4);
			container.shrink_to_fit();
		} else {
			container.resize(0);
			container.resize(container.capacity());
		}
		data = container.data();
		size = container.size();
	}

	// Called after the load to resize the container to the amount of elements writen
	void shrink(container_t& container) {
		assert(container.size() >= size);
		container.resize(size);
	}
};

} // namespace screeps::internal
