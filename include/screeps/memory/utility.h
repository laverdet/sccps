#pragma once

namespace serialization {

// I can't believe I have to write this helper..
template <class Type, class Enable = void>
struct remove_cv_under_ref;

template <class Type>
struct remove_cv_under_ref<Type, typename std::enable_if<!std::is_reference<Type>::value>::type> {
	using type = typename std::remove_cv<Type>::type;
};

template <class Type>
struct remove_cv_under_ref<Type, typename std::enable_if<std::is_rvalue_reference<Type>::value>::type> {
	using type = typename std::add_rvalue_reference<typename std::remove_cv<typename std::remove_reference<Type>::type>::type>::type;
};

template <class Type>
struct remove_cv_under_ref<Type, typename std::enable_if<std::is_lvalue_reference<Type>::value>::type> {
	using type = typename std::add_lvalue_reference<typename std::remove_cv<typename std::remove_reference<Type>::type>::type>::type;
};

} // namespace serialization
