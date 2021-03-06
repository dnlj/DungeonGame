#pragma once


// Engine
#include <Engine/Detail/Detail.hpp>
#include <Engine/GlobalConfig.hpp>
#include <Engine/FatalException.hpp>
#include <Engine/Constants.hpp>
#include <Engine/Types.hpp>

#define ENGINE_SIDE_SERVER 1
#define ENGINE_SIDE_CLIENT 2
#define ENGINE_SIDE_BOTH (ENGINE_SIDE_SERVER | ENGINE_SIDE_CLIENT)
#define ENGINE_SERVER (ENGINE_SIDE == ENGINE_SIDE_SERVER)
#define ENGINE_CLIENT (ENGINE_SIDE == ENGINE_SIDE_CLIENT)

#ifdef DEBUG
	#define ENGINE_DEBUG true
#else
	#define ENGINE_DEBUG false
#endif

// TODO: look into other inline options
// GCC: https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/Function-Attributes.html
// CLANG: https://clang.llvm.org/docs/AttributeReference.html
// MSVC: https://www.reddit.com/r/programming/comments/g9inos/smarter_cc_inlining_with_attribute_flatten/foyoq62/
// 
// We have always had __forceinline, and I just added four attributes in this area,
// 
//     [[msvc::forceinline]] - same as __forceinline on a function definition),
//     [[msvc::noinline]] - same as declspec(noinline) on a function,
//     [[msvc::forceinline_calls]] - goes on statements or statement blocks, and attempts to forceinline all calls in that block,
//     [[msvc::noinline_calls]] - don't inline any function calls in this block
// 
// Nothing like flatten (msvc::forceinline_calls isn't recursive, it only does "one level"), although that would be easy to implement
//

/**
 * Attempts to force a function to be inlined.
 */
#define ENGINE_INLINE [[msvc::forceinline]] // TODO: cross platform: [[gnu::always_inline]]

/**
 * Attempts to force inline all function calls in a block or statement.
 */
#define ENGINE_INLINE_CALLS [[msvc::forceinline_calls]] // TODO: cross platform: [[gnu::flatten]]

#define ENGINE_BUILD_BIN_OP(T, O) \
	ENGINE_INLINE constexpr decltype(auto) operator O(const T& a, const T& b) noexcept { \
		return static_cast<T>(static_cast<std::underlying_type_t<T>>(a) O static_cast<std::underlying_type_t<T>>(b)); \
	}

#define ENGINE_BUILD_UNARY_OP(T, O) \
	ENGINE_INLINE constexpr decltype(auto) operator O(const T& a) noexcept { \
		return static_cast<T>(O static_cast<std::underlying_type_t<T>>(a)); \
	}

#define ENGINE_BUILD_UNARY_OP_REF(T, O) \
	ENGINE_INLINE constexpr decltype(auto) operator O(T& a) noexcept { \
		auto tmp = static_cast<std::underlying_type_t<T>>(a);\
		return a = static_cast<T>(O tmp);\
	}

#define ENGINE_BUILD_UNARY_OP_INT(T, O) \
	ENGINE_INLINE constexpr decltype(auto) operator O(T& a, int) noexcept { \
		const auto tmp = a;\
		O a;\
		return tmp;\
	}

#define ENGINE_BUILD_ASSIGN_BIN_OP(T, O) \
	ENGINE_INLINE constexpr decltype(auto) operator O=(T& a, const T& b) noexcept { return a = a O b; }

#define ENGINE_BUILD_ALL_OPS(T) \
	ENGINE_BUILD_UNARY_OP(T, ~); \
	ENGINE_BUILD_UNARY_OP(T, !); \
	ENGINE_BUILD_UNARY_OP(T, +); \
	ENGINE_BUILD_UNARY_OP(T, -); \
	ENGINE_BUILD_UNARY_OP_REF(T, ++); \
	ENGINE_BUILD_UNARY_OP_REF(T, --); \
	ENGINE_BUILD_UNARY_OP_INT(T, ++); \
	ENGINE_BUILD_UNARY_OP_INT(T, --); \
	ENGINE_BUILD_BIN_OP(T, |); \
	ENGINE_BUILD_BIN_OP(T, &); \
	ENGINE_BUILD_BIN_OP(T, ^); \
	ENGINE_BUILD_BIN_OP(T, +); \
	ENGINE_BUILD_BIN_OP(T, -); \
	ENGINE_BUILD_BIN_OP(T, *); \
	ENGINE_BUILD_BIN_OP(T, /); \
	ENGINE_BUILD_BIN_OP(T, %); \
	ENGINE_BUILD_ASSIGN_BIN_OP(T, |); \
	ENGINE_BUILD_ASSIGN_BIN_OP(T, &); \
	ENGINE_BUILD_ASSIGN_BIN_OP(T, ^); \
	ENGINE_BUILD_ASSIGN_BIN_OP(T, +); \
	ENGINE_BUILD_ASSIGN_BIN_OP(T, -); \
	ENGINE_BUILD_ASSIGN_BIN_OP(T, *); \
	ENGINE_BUILD_ASSIGN_BIN_OP(T, /); \
	ENGINE_BUILD_ASSIGN_BIN_OP(T, %); \

/**
 * @def ENGINE_EMPTY_BASE
 * Enables more broad use of empty base optmization on MSVC
 * @see https://devblogs.microsoft.com/cppblog/optimizing-the-layout-of-empty-base-classes-in-vs2015-update-2-3/
 */
#if ENGINE_OS_WINDOWS
	#define ENGINE_EMPTY_BASE __declspec(empty_bases)
#else
	#define ENGINE_EMPTY_BASE
#endif

// TODO: replace macros with source_location?
#define _ENGINE_CREATE_LOG_LAMBDA(Prefix, Decorate, Color, Other)\
	([](auto&&... args){\
		::Engine::Detail::log<Decorate>(Prefix, Color,\
			(__FILE__ + sizeof(ENGINE_BASE_PATH)), __LINE__,\
			std::forward<decltype(args)>(args) ...\
		);\
		Other;\
	})

#define _ENGINE_CREATE_ASSERT_LAMBDA(Prefix, Decorate, Color, Other)\
	([](auto cond, auto&&... args){\
		if (!cond) {\
			_ENGINE_CREATE_LOG_LAMBDA(Prefix, Decorate, Color, Other)(std::forward<decltype(args)>(args)...);\
		}\
	})

#if ENGINE_DEBUG
	#define ENGINE_DIE __debugbreak(); ::std::terminate();
#else
	#define ENGINE_DIE ::std::terminate();
#endif
	
#define ENGINE_RAW_TEXT _ENGINE_CREATE_LOG_LAMBDA("[TEXT]", false, Engine::ASCII_FG2, 0)
#define ENGINE_LOG _ENGINE_CREATE_LOG_LAMBDA("[LOG]", true, Engine::ASCII_FG2, 0)
#define ENGINE_INFO _ENGINE_CREATE_LOG_LAMBDA("[INFO]", true, Engine::ASCII_INFO, 0)
#define ENGINE_SUCCESS _ENGINE_CREATE_LOG_LAMBDA("[SUCCESS]", true, Engine::ASCII_SUCCESS, 0)
#define ENGINE_WARN _ENGINE_CREATE_LOG_LAMBDA("[WARN]", true, Engine::ASCII_WARN, 0)
#define ENGINE_FAIL _ENGINE_CREATE_LOG_LAMBDA("[FAIL]", true, Engine::ASCII_ERROR, 0)
#define ENGINE_ERROR _ENGINE_CREATE_LOG_LAMBDA("[ERROR]", true, Engine::ASCII_ERROR, ENGINE_DIE)

#define ENGINE_ASSERT _ENGINE_CREATE_ASSERT_LAMBDA("[ERROR][ASSERT]", true, Engine::ASCII_ERROR, ENGINE_DIE)
#define ENGINE_ASSERT_WARN _ENGINE_CREATE_ASSERT_LAMBDA("[WARN][ASSERT]", true, Engine::ASCII_WARN, 0)

#if ENGINE_DEBUG
	#define ENGINE_DEBUG_ASSERT ENGINE_ASSERT
#else
	#define ENGINE_DEBUG_ASSERT(...)
#endif
