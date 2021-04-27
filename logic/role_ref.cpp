#include <algorithm>

#include "../util/type_traits.hpp"

#include "game.hpp"
#include "role_ref.hpp"
#include "rulebook.hpp"


bool maf::RoleRef::member_of(Rulebook const& rulebook) {
	return std::visit([&](auto&& x) {
		using T = std::decay_t<decltype(x)>;
		if constexpr(is_same<T, Role::ID>) {
			return std::any_of(rulebook.roles_begin(), rulebook.roles_end(),
			                   [&](auto& role){ return role.id() == x; });
		} else if constexpr(is_same<T, string_view>) {
			return std::any_of(rulebook.roles_begin(), rulebook.roles_end(),
			                   [&](auto& role){ return role.alias() == x; });
		}
	}, _param);
}

bool maf::RoleRef::member_of(Game const& game) {
	return this->member_of(game.rulebook());
}

maf::Role const& maf::RoleRef::resolve(Rulebook const& rulebook)
{
	auto p = std::visit([&](auto&& x) {
		using T = std::decay_t<decltype(x)>;
		if constexpr(is_same<T, Role::ID>) {
			auto p = std::find_if(rulebook.roles_begin(), rulebook.roles_end(),
			                      [&](auto& role){ return role.id() == x; });
			if (p == rulebook.roles_end()) {
				throw std::out_of_range("role ref could not be resolved, when searching by ID");
			}
			return p;
		} else if constexpr(is_same<T, string_view>) {
			auto p = std::find_if(rulebook.roles_begin(), rulebook.roles_end(),
			                      [&](auto& role){ return role.alias() == x; });
			if (p == rulebook.roles_end()) {
				throw std::out_of_range("role ref could not be resolved, when searching by alias");
			}
			return p;
		}
	}, _param);

	return *p;
}

maf::Role const& maf::RoleRef::resolve(Game const& game) {
	return this->resolve(game.rulebook());
}
