#include "../util/algorithm.hpp"

#include "role_ref.hpp"
#include "rulebook.hpp"

namespace maf::core {
	Rulebook::Rulebook(Edition edition) : _edition{edition} {
		if (edition != 1) throw Bad_edition{edition};

		Role & peasant = new_village_role(Role::ID::peasant);
		peasant._duel_strength = 0.333333333;

		Role & doctor = new_village_role(Role::ID::doctor);
		doctor._ability_or_none = {Ability::ID::heal};
		doctor._duel_strength = 0.1;

		Role & detective = new_village_role(Role::ID::detective);
		detective._ability_or_none = {Ability::ID::investigate};
		detective._duel_strength = 4;

		Role & racketeer = new_mafia_role(Role::ID::racketeer);
		racketeer._duel_strength = 9;

		Role & godfather = new_mafia_role(Role::ID::godfather);
		godfather._suspicious = false;
		godfather._duel_strength = 0.4;

		Role & dealer = new_mafia_role(Role::ID::dealer);
		dealer._ability_or_none = {Ability::ID::peddle};

		Role & coward = new_freelance_role(Role::ID::coward);
		coward._suspicious = true;
		coward._duel_strength = 0.000000001;

		Role & actor = new_freelance_role(Role::ID::actor);
		actor._role_faker = true;
		actor._duel_strength = 0.333333333;

		Role & serial_killer = new_freelance_role(Role::ID::serial_killer);
		serial_killer._ability_or_none = {Ability::ID::kill};
		serial_killer._peace_condition = Peace_condition::last_survivor;
		serial_killer._suspicious = true;
		serial_killer._duel_strength = 999999999;

		Role & village_idiot = new_freelance_role(Role::ID::village_idiot);
		village_idiot._win_condition = Win_condition::be_lynched;
		village_idiot._troll = true;
		village_idiot._duel_strength = 0.001;

		Role & musketeer = new_freelance_role(Role::ID::musketeer);
		musketeer._ability_or_none = {Ability::ID::duel};
		musketeer._win_condition = Win_condition::win_duel;

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

	Role & Rulebook::new_role(Role::ID id) {
		if (contains(id)) {
			throw Preexisting_role_ID{id};
		}

		_roles.emplace_back(id);
		return _roles.back();
	}

	Role & Rulebook::new_village_role(Role::ID id) {
		Role & role = new_role(id);
		role._alignment = Alignment::village;
		role._peace_condition = Peace_condition::mafia_eliminated;
		return role;
	}

	Role & Rulebook::new_mafia_role(Role::ID id) {
		Role & role = new_role(id);
		role._alignment = Alignment::mafia;
		role._peace_condition = Peace_condition::village_eliminated;
		role._suspicious = true;
		role._duel_strength = 4;
		return role;
	}

	Role & Rulebook::new_freelance_role(Role::ID id) {
		return new_role(id);
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
