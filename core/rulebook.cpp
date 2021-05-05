#include "../util/algorithm.hpp"

#include "role_ref.hpp"
#include "rulebook.hpp"

namespace maf::core {
	Rulebook::Rulebook(Edition edition) : _edition{edition} {
		if (edition != 1) throw Bad_edition{edition};

		create_role(Role::ID::peasant, Alignment::village, [](Role & role) {
			role._duel_strength = 0.333333333;
		});

		create_role(Role::ID::doctor, Alignment::village, [](Role & role) {
			role._ability = {Ability::ID::heal};
			role._duel_strength = 0.1;
		});

		create_role(Role::ID::detective, Alignment::village, [](Role & role) {
			role._ability = {Ability::ID::investigate};
			role._duel_strength = 4;
		});

		create_role(Role::ID::racketeer, Alignment::mafia, [](Role & role) {
			role._duel_strength = 9;
		});

		create_role(Role::ID::godfather, Alignment::mafia, [](Role & role) {
			role._suspicious = false;
			role._duel_strength = 0.4;
		});

		create_role(Role::ID::dealer, Alignment::mafia, [](Role & role) {
			role._ability = {Ability::ID::peddle};
		});

		create_role(Role::ID::coward, Alignment::freelance, [](Role & role) {
			role._suspicious = true;
			role._duel_strength = 0.000000001;
		});

		create_role(Role::ID::actor, Alignment::freelance, [](Role & role) {
			role._role_faker = true;
			role._duel_strength = 0.333333333;
		});

		create_role(Role::ID::serial_killer, Alignment::freelance, [](Role & role) {
			role._ability = {Ability::ID::kill};
			role._peace_condition = Peace_condition::last_survivor;
			role._suspicious = true;
			role._duel_strength = 999999999;
		});

		create_role(Role::ID::village_idiot, Alignment::freelance, [](Role & role) {
			role._win_condition = Win_condition::be_lynched;
			role._troll = true;
			role._duel_strength = 0.001;
		});

		create_role(Role::ID::musketeer, Alignment::freelance, [](Role & role) {
			role._ability = {Ability::ID::duel};
			role._win_condition = Win_condition::win_duel;
		});

		new_wildcard(Wildcard::ID::any, [](const Role &) {
			return 1;
		});

		new_wildcard(Wildcard::ID::village, [](const Role & r) {
			return (r.alignment() == Alignment::village) ? 1 : 0;
		});

		new_wildcard(Wildcard::ID::village_basic, {
			{Role::ID::peasant, 5},
			{Role::ID::doctor, 2},
			{Role::ID::detective, 2}
		});

		new_wildcard(Wildcard::ID::mafia, [](const Role & r) {
			return (r.alignment() == Alignment::mafia) ? 1 : 0;
		});

		new_wildcard(Wildcard::ID::freelance, [](const Role & r) {
			return (r.alignment() == Alignment::freelance) ? 1 : 0;
		});
	}

	bool Rulebook::contains(RoleRef r_ref) const {
		return r_ref.member_of(*this);
	}

	bool Rulebook::contains_wildcard(Wildcard::ID id) const {
		for (auto & w: _wildcards) {
			if (w.id() == id) return true;
		}

		return false;
	}

	bool Rulebook::contains_wildcard(string_view alias) const {
		for (auto & w: _wildcards) {
			if (w.alias() == alias) return true;
		}

		return false;
	}

	Role & Rulebook::get_role(Role::ID id) {
		for (auto & r: _roles) {
			if (r.id() == id) return r;
		}

		throw Missing_role_ID{id};
	}

	Role & Rulebook::get_role(string_view alias) {
		for (auto & r: _roles) {
			if (r.alias() == alias) return r;
		}

		throw Missing_role_alias{string{alias}};
	}

	const Role & Rulebook::look_up(RoleRef r_ref) const {
		return r_ref.resolve(*this);
	}

	Wildcard & Rulebook::get_wildcard(Wildcard::ID id) {
		for (auto & w: _wildcards) {
			if (w.id() == id) return w;
		}

		throw Missing_wildcard_ID{id};
	}

	const Wildcard & Rulebook::get_wildcard(Wildcard::ID id) const {
		for (auto & w: _wildcards) {
			if (w.id() == id) return w;
		}

		throw Missing_wildcard_ID{id};
	}

	Wildcard & Rulebook::get_wildcard(string_view alias) {
		for (auto & w: _wildcards) {
			if (w.alias() == alias) return w;
		}

		throw Missing_wildcard_alias{string{alias}};
	}

	const Wildcard & Rulebook::get_wildcard(string_view alias) const {
		for (auto & w: _wildcards) {
			if (w.alias() == alias) return w;
		}

		throw Missing_wildcard_alias{string{alias}};
	}

	Role & Rulebook::add_role(Role role) {
		if (this->contains(role.id()))
			throw Preexisting_role_ID{role.id()};

		_roles.push_back(move(role));
		return _roles.back();
	}

	Wildcard & Rulebook::new_wildcard(Wildcard::ID id, Wildcard::Role_evaluator evaluator) {
		if (contains_wildcard(id)) {
			throw Preexisting_wildcard_ID{id};
		}

		_wildcards.emplace_back(id, evaluator);
		return _wildcards.back();
	}

	Wildcard & Rulebook::new_wildcard(Wildcard::ID id, const std::map<Role::ID, double> & weights) {
		if (contains_wildcard(id)) {
			throw Preexisting_wildcard_ID{id};
		}

		_wildcards.emplace_back(id, weights);
		return _wildcards.back();
	}
}
