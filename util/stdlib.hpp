#ifndef MAFIA_UTIL_STDLIB
#define MAFIA_UTIL_STDLIB

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace maf {
	// Typenames

	using string = std::string;
	using string_view = std::string_view;

	template <typename T> using optional = std::optional<T>;
	template <typename T1, typename T2> using pair = std::pair<T1, T2>;
	template <typename T> using unique_ptr = std::unique_ptr<T>;
	template <typename... Ts> using variant = std::variant<Ts...>;
	template <typename T> using vector = std::vector<T>;
	
	// Functions

	using std::make_unique;
	using std::move;
	using std::swap;

	// Operators

	using std::literals::operator""sv;
}

#endif
