#include "names.hpp"


maf::string_view maf::full_name(const Role &r) {
	return full_name(r.id());
}

maf::string_view maf::full_name(Role::ID id) {
	switch (id) {
		case Role::ID::peasant:
			return "Peasant";
		case Role::ID::doctor:
			return "Doctor";
		case Role::ID::detective:
			return "Detective";
		case Role::ID::racketeer:
			return "Racketeer";
		case Role::ID::godfather:
			return "Godfather";
		case Role::ID::dealer:
			return "Dealer";
		case Role::ID::coward:
			return "Coward";
		case Role::ID::actor:
			return "Actor";
		case Role::ID::serial_killer:
			return "Serial Killer";
		case Role::ID::village_idiot:
			return "Village Idiot";
		case Role::ID::musketeer:
			return "Musketeer";
	}
}

maf::string_view maf::category(const Wildcard &w) {
	return category(w.id());
}

maf::string_view maf::category(Wildcard::ID id) {
	switch (id) {
		case Wildcard::ID::any:
			return "totally random";
		case Wildcard::ID::village:
			return "random village";
		case Wildcard::ID::village_basic:
			return "basic village";
		case Wildcard::ID::mafia:
			return "random mafia";
		case Wildcard::ID::freelance:
			return "random freelance";
	}
}
