#include <algorithm>

#include "../util/type_traits.hpp"

#include "game.hpp"
#include "role_ref.hpp"
#include "rulebook.hpp"


namespace maf::core {
	bool RoleRef::member_of(const Rulebook & rulebook) {
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

	bool RoleRef::member_of(const Game & game) {
		return this->member_of(game.rulebook());
	}

	const Role & RoleRef::resolve(const Rulebook & rulebook) {
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

	const Role & RoleRef::resolve(const Game & game) {
		return this->resolve(game.rulebook());
	}
}
