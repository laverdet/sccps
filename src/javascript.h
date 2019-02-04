#pragma once
#ifdef __EMSCRIPTEN__
#define JAVASCRIPT
#include <emscripten.h>
#elif BUILDING_NODE_EXTENSION
#define JAVASCRIPT
#define EMSCRIPTEN_KEEPALIVE
#define EM_ASM(code, ...) (inline_javascript(#code, ##__VA_ARGS__))
#define EM_ASM_INT(code, ...) (inline_javascript(#code, ##__VA_ARGS__))
#include <nan.h>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace screeps {
class js_error : public std::exception {};

template <class Type>
v8::Local<Type> unwrap_maybe(v8::MaybeLocal<Type> value) {
	v8::Local<Type> unwrapped;
	if (value.ToLocal(&unwrapped)) {
		return unwrapped;
	}
	throw std::runtime_error("Caught v8 exception");
}

template <class... Args>
int inline_javascript(const char* fragment, Args... args) {
	auto string = std::string("(function(");
	for (auto ii = 0u; ii < sizeof...(Args); ++ii) {
		if (ii != 0) {
			string += ", ";
		}
		string += "$" + std::to_string(ii);
	}
	string += ")";
	string += fragment;
	string += ")";

	Nan::TryCatch try_catch;
	try {
		auto fn = unwrap_maybe(Nan::RunScript(unwrap_maybe(Nan::CompileScript(Nan::New(string).ToLocalChecked())))).As<v8::Function>();
		v8::Local<v8::Value> argv[sizeof...(Args)];
		int ii = 0;
		([&](auto param) {
			if constexpr (std::is_same_v<decltype(param), float> || std::is_same_v<decltype(param), double>) {
				argv[ii++] = Nan::New<v8::Number>(static_cast<double>(param));
			} else if constexpr (std::is_pointer_v<decltype(param)>) {
				argv[ii++] = Nan::New<v8::Number>(static_cast<double>((size_t)param));
			} else {
				argv[ii++] = Nan::New<v8::Number>((int)param);
			}
		}(args), ...);
		auto value = unwrap_maybe(Nan::Call(fn, Nan::GetCurrentContext()->Global(), ii, argv));
		if (value->IsNumber()) {
			return Nan::To<int64_t>(value).ToChecked();
		} else {
			return -1;
		}
	} catch (const std::runtime_error& ex) {
		if (try_catch.HasCaught()) {
			Nan::Utf8String error(Nan::Get(Nan::To<v8::Object>(try_catch.Exception()).ToLocalChecked(), Nan::New("stack").ToLocalChecked()).ToLocalChecked());
			fprintf(stderr, "%s\n", *error);
			try_catch.ReThrow();
			throw js_error();
		}
		throw;
	}
}
} // namespace screeps
#else
#include <iostream>
namespace {
template <class... Args>
int tried_to_run_javascript(const char* code, Args&&... /* args */) {
	std::cerr <<"Tried to run JavaScript as standalone: " <<code <<"\n";
	std::terminate();
	return 0;
}
}
#define EMSCRIPTEN_KEEPALIVE
#define EM_ASM(code, args...) (tried_to_run_javascript(#code, ##__VA_ARGS__))
#define EM_ASM_INT(code, args...) (tried_to_run_javascript(#code, ##__VA_ARGS__))
#endif
