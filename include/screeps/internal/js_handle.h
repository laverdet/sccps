#pragma once
#include <memory>

namespace screeps::internal {

class js_handle_t {
	private:
		struct js_handle_ref_t {
			int ref;
#ifndef __EMSCRIPTEN__
			void* isolate;
#endif
			explicit js_handle_ref_t(int ref);
			js_handle_ref_t(const js_handle_ref_t&) = default;
			js_handle_ref_t& operator=(const js_handle_ref_t&) = default;
			~js_handle_ref_t();
		};
		std::shared_ptr<js_handle_ref_t> _ref;

	protected:
		js_handle_t() = default;
		explicit js_handle_t(int ref) : _ref(std::make_shared<js_handle_ref_t>(ref)) {}

	public:
		static int get_ref(const js_handle_t& handle) {
			return handle._ref->ref;
		}
};

} // namespace screeps::internal
