#pragma once

#ifndef MAGIC_VTABLE_LIB
#define MAGIC_VTABLE_LIB

#if defined(_MSC_VER) && defined(__clang__)
	#define MAGIC_VTABLE_CLANG_CL
	#define MAGIC_VTABLE_PREFIX std::string_view{"@@$"}
	#define MAGIC_VTABLE_SUFFIX std::string_view{"@@"}
#elif defined(_MSC_VER)
	#define MAGIC_VTABLE_MSVC
	#define MAGIC_VTABLE_PREFIX std::string_view{"1@$"}
	#define MAGIC_VTABLE_SUFFIX std::string_view{"@@"}
#else
	#error "Unsupported compiler"
#endif

#include <string_view>

namespace magic_vft
{
	namespace detail
	{
		consteval size_t decode_microsoft_value(std::string_view str)
		{
			// weird cases that i'm not sure how to handle
			if (str.starts_with("B3A"))
				return 4;
			if (str.starts_with("B7A"))
				return 8;

			// Skip the leading 'B'
			if (str.front() != 'B')
			{
				// invoke UB to stop constexpr evaluation
				std::unreachable();
			}
			str.remove_prefix(1);

			size_t value{};
			while (!str.empty() && str.front() != '@')
			{
				value *= 16;
				value += str.front() - 'A';
				str.remove_prefix(1);
			}
			return value;
		}
	}

	template<auto>
	consteval size_t vtable_index()
	{
		constexpr std::string_view mangled{__FUNCDNAME__};
		constexpr auto first = mangled.find(MAGIC_VTABLE_PREFIX) + MAGIC_VTABLE_PREFIX.size();
		constexpr auto last = mangled.find(MAGIC_VTABLE_SUFFIX, first);
		constexpr auto value = detail::decode_microsoft_value(mangled.substr(first, last - first));
		return value / sizeof(size_t);
	}
}

#endif // MAGIC_VTABLE_LIB