#ifndef MAFIA_CORE_ROLE_REF_H
#define MAFIA_CORE_ROLE_REF_H

#include "../util/stdlib.hpp"
#include "role.hpp"

namespace maf::core {
	class Rulebook;
	class Game;

	struct RoleRef {
		// Create a reference to roles with the given ID.
		RoleRef(Role::ID r_id): _param{r_id} { }

		// Create a reference to roles with the given alias.
		//
		// Note: This reference does not take ownership of the string being
		// viewed. Be careful about managing the scope of the `RoleRef` when
		// using this constructor: the `RoleRef` must not outlive the lifetime
		// of the string.
		RoleRef(string_view str_v): _param{str_v} { }

		// Check if the role can be found in a rulebook.
		bool member_of(Rulebook const& rulebook);

		// Check if the role can be found in the rulebook of a game.
		bool member_of(Game const& game);

		// Find and return the role within a rulebook.
		//
		// Throws `std::out_of_range` if the role cannot be found.
		Role const& resolve(Rulebook const& rulebook);

		// Find and return the role within the rulebook of a game.
		//
		// Throws `std::out_of_range` if the role cannot be found.
		Role const& resolve(Game const& game);

	private:
		using param_t = variant<Role::ID,     // ID in a rulebook
								string_view>; // role alias

		param_t _param;
	};
}

#endif
