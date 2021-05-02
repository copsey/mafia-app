#include "names.hpp"

namespace maf {
	string_view full_name(const core::Role & r) {
		return full_name(r.id());
	}

	string_view full_name(core::Role::ID id) {
		switch (id) {
		using ID = core::Role::ID;
		case ID::peasant:
			return "Peasant";
		case ID::doctor:
			return "Doctor";
		case ID::detective:
			return "Detective";
		case ID::racketeer:
			return "Racketeer";
		case ID::godfather:
			return "Godfather";
		case ID::dealer:
			return "Dealer";
		case ID::coward:
			return "Coward";
		case ID::actor:
			return "Actor";
		case ID::serial_killer:
			return "Serial Killer";
		case ID::village_idiot:
			return "Village Idiot";
		case ID::musketeer:
			return "Musketeer";
		}
	}

	string_view category(const core::Wildcard & w) {
		return category(w.id());
	}

	string_view category(core::Wildcard::ID id) {
		switch (id) {
		using ID = core::Wildcard::ID;
		case ID::any:
			return "totally random";
		case ID::village:
			return "random village";
		case ID::village_basic:
			return "basic village";
		case ID::mafia:
			return "random mafia";
		case ID::freelance:
			return "random freelance";
		}
	}
}
