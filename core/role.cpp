#include "role.hpp"

namespace maf::core {
	string_view Role::alias() const {
		return core::alias(_id);
	}

	string_view alias(Role::ID id) {
		switch (id) {
		case Role::ID::peasant:
			return "peasant";
		case Role::ID::doctor:
			return "doctor";
		case Role::ID::detective:
			return "detective";
		case Role::ID::racketeer:
			return "racketeer";
		case Role::ID::godfather:
			return "godfather";
		case Role::ID::dealer:
			return "dealer";
		case Role::ID::coward:
			return "coward";
		case Role::ID::actor:
			return "actor";
		case Role::ID::serial_killer:
			return "serial-killer";
		case Role::ID::village_idiot:
			return "village-idiot";
		case Role::ID::musketeer:
			return "musketeer";
		}
	}
}
