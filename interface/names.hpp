#ifndef MAFIA_NAMES_H
#define MAFIA_NAMES_H

#include "../util/stdlib.hpp"
#include "../core/core.hpp"

namespace maf {
	// The full name of the given role.
	string_view full_name(const Role &role);
	string_view full_name(Role::ID id);

	// Function object comparing the full names of two roles with the given IDs.
	struct Role_ID_full_name_compare {
		bool operator()(Role::ID id1, Role::ID id2) const {
			return full_name(id1) < full_name(id2);
		}
	};

	/* FIXME */
	// The category of the given wildcard.
	string_view category(const Wildcard &wildcard);
	string_view category(Wildcard::ID id);
}

#endif
