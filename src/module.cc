#include "./javascript.h"
#include <screeps/game.h>
#include <stdexcept>
#include <typeinfo>

// Keep loop function alive
EMSCRIPTEN_KEEPALIVE void loop();

// Called in the case of an uncaught exception. This does the `what()` virtual function call and
// also returns RTTI.
EMSCRIPTEN_KEEPALIVE
void* exception_what(void* ptr) {
	struct hole_t {
		const char* name;
		const char* what;
	};
	static hole_t hole;
	auto& err = *reinterpret_cast<const std::exception*>(ptr);
	hole.name = typeid(err).name();
	hole.what = err.what();
	return reinterpret_cast<void*>(&hole);
}

// Native module loader
#ifdef BUILDING_NODE_EXTENSION
#include <nan.h>
#include <isolated_vm.h>
using namespace v8;

NAN_METHOD(mod_make_array_buffer) {
	size_t addr = Nan::To<int64_t>(info[0]).ToChecked();
	auto ab = ArrayBuffer::New(Isolate::GetCurrent(), reinterpret_cast<void*>(addr == 0 ? 0xdeadbeef : addr), 0x100000000);
	info.GetReturnValue().Set(ab);
	if (addr == 0) {
		// v8 won't let you create a buffer starting at address 0 so I cheat.
		auto hack = **(char***)&ab;
		for (int ii = 0; ii < 100; ++ii) {
			auto& value = *(unsigned int*)(hack + ii);
			if (value == 0xdeadbeef) {
				value = 0;
				return;
			}
		}
		throw std::runtime_error("Failed to be sneaky");
	}
}

NAN_METHOD(mod_game_state_init_layout) {
	screeps::game_state_t::init_layout();
}

NAN_METHOD(mod_game_state_ensure_capacity) {
	screeps::game_state_t::ensure_capacity(reinterpret_cast<screeps::game_state_t*>(Nan::To<int64_t>(info[0]).ToChecked()));
}

NAN_METHOD(mod_room_ensure_capacity) {
	screeps::room_t::ensure_capacity(reinterpret_cast<screeps::room_t*>(Nan::To<int64_t>(info[0]).ToChecked()));
}

NAN_METHOD(mod_loop) {
	Nan::TryCatch try_catch;
	try {
		loop();
	} catch (const screeps::js_error&) {
		assert(try_catch.HasCaught());
		try_catch.ReThrow();
	}
}

ISOLATED_VM_MODULE void InitForContext(Isolate* isolate, Local<Context> context, Local<Object> target) {
	Nan::SetMethod(target, "makeArrayBuffer", mod_make_array_buffer);
	Nan::SetMethod(target, "__ZN7screeps12game_state_t11init_layoutEv", mod_game_state_init_layout);
	Nan::SetMethod(target, "__ZN7screeps12game_state_t15ensure_capacityEPS0_", mod_game_state_ensure_capacity);
	Nan::SetMethod(target, "__ZN7screeps6room_t15ensure_capacityEPS0_", mod_room_ensure_capacity);
	Nan::SetMethod(target, "__Z4loopv", mod_loop);
}
#endif
