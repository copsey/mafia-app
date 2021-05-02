#ifndef MAFIA_CORE_ROLE_H
#define MAFIA_CORE_ROLE_H

#include "../util/stdlib.hpp"
#include "../util/string.hpp"

namespace maf::core {
	class Rulebook;

	enum class Alignment {
		village,
		mafia,
		freelance
	};

	struct Ability {
		enum class ID {
			kill,
			heal,
			investigate,
			peddle,
			duel
		};

		ID id;
	};

	enum class Win_condition {
		survive,
		village_remains,
		mafia_remains,
		be_lynched,
		win_duel
	};

	enum class Peace_condition {
		always_peaceful,
		village_eliminated,
		mafia_eliminated,
		last_survivor
	};

	struct Role {
		enum class ID {
			peasant,
			doctor,
			detective,
			racketeer,
			godfather,
			dealer,
			coward,
			actor,
			serial_killer,
			village_idiot,
			musketeer
		};

		/// Create a role with the given ID.
		///
		/// All other traits are set to their default values.
		Role(ID id) : _id{id}
		{ }

		/// The ID of the role.
		ID id() const {
			return _id;
		}

		/// The alias of the role.
		/// Equivalent to `alias(this->id())`.
		string_view alias() const;

		/// The alignment of the role.
		Alignment alignment() const {
			return _alignment;
		}

		/// The ability of the role, if it has one.
		///
		/// @throws `std::bad_optional_access` if the role has no ability.
		Ability ability() const {
			return _ability_or_none.value();
		}

		/// Whether the role has an ability.
		bool has_ability() const {
			return _ability_or_none.has_value();
		}

		/// The condition that the role needs to satisfy to win.
		Win_condition win_condition() const {
			return _win_condition;
		}

		/// The peace condition of the role.
		///
		/// The game cannot end until all active players simultaneously have their
		/// peace conditions satisfied.
		Peace_condition peace_condition() const {
			return _peace_condition;
		}

		/// Whether the role appears as suspicious when investigated.
		bool is_suspicious() const {
			return _suspicious;
		}

		/// Whether the role becomes a ghost and haunts another player upon death.
		bool is_troll() const {
			return _troll;
		}

		/// Whether the role must pretend to be another role.
		bool is_role_faker() const {
			return _role_faker;
		}

		/// The strength of the role in duels.
		double duel_strength() const {
			return _duel_strength;
		}

	private:
		ID _id;
		Alignment _alignment{Alignment::freelance};
		optional<Ability> _ability_or_none{};
		Win_condition _win_condition{Win_condition::survive};
		Peace_condition _peace_condition{Peace_condition::always_peaceful};
		bool _suspicious{false};
		bool _troll{false};
		bool _role_faker{false};
		double _duel_strength{1};

		friend class Rulebook;
	};

	/// The alias corresponding to the given role ID.
	string_view alias(Role::ID id);
}

#endif
