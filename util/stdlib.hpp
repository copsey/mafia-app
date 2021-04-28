#ifndef MAFIA_UTIL_STDLIB
#define MAFIA_UTIL_STDLIB

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <gsl/pointers>

namespace maf {
	template <typename T> using optional = std::optional<T>;
	template <typename T1, typename T2> using pair = std::pair<T1, T2>;
	template <typename T> using unique_ptr = std::unique_ptr<T>;
	template <typename... Ts> using variant = std::variant<Ts...>;
	template <typename T> using vector = std::vector<T>;
	template <typename T> using vector_of_refs = std::vector<std::reference_wrapper<T>>;

	template <typename T> using not_null = gsl::not_null<T>;

	// Functions

	using std::make_unique;
	using std::move;
	using std::swap;
}

#endif
