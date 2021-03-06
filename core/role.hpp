#ifndef MAFIA_CORE_ROLE_H
#define MAFIA_CORE_ROLE_H

#include "../util/optional.hpp"
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

	class Role {
	public:
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

		/// Create a role with the given ID and alignment. Set all other
		/// traits to the default for that alignment.
		Role(ID id, Alignment alignment) : _id{id}, _alignment{alignment} {
			_set_defaults_for_alignment();
		}

		/// The ID of the role.
		ID id() const { return _id; }

		/// The alias of the role.
		/// Equivalent to `alias(this->id())`.
		string_view alias() const;

		/// The alignment of the role.
		Alignment alignment() const { return _alignment; }

		/// The ability of the role, if it has one.
		///
		/// @throws `std::bad_optional_access` if the role has no ability.
		Ability ability() const { return _ability.value(); }

		/// Whether the role has an ability.
		bool has_ability() const { return _ability.has_value(); }

		/// Whether the role has an ability with the given ID.
		bool has_ability(Ability::ID id) const {
			return _ability.has_value() && (*_ability).id == id;
		}

		/// The condition that the role needs to satisfy to win.
		Win_condition win_condition() const { return _win_condition; }

		/// The peace condition of the role.
		///
		/// The game cannot end until all active players simultaneously have
		/// their peace conditions satisfied.
		Peace_condition peace_condition() const { return _peace_condition; }

		/// Whether the role appears as suspicious when investigated.
		bool is_suspicious() const { return _suspicious; }

		/// Whether the role becomes a ghost and haunts another player upon
		/// death.
		bool is_troll() const { return _troll; }

		/// Whether the role must pretend to be another role.
		bool is_role_faker() const { return _role_faker; }

		/// The strength of the role in duels.
		double duel_strength() const { return _duel_strength; }

	private:
		ID _id;
		Alignment _alignment;
		optional<Ability> _ability{};
		Win_condition _win_condition{Win_condition::survive};
		Peace_condition _peace_condition{Peace_condition::always_peaceful};
		bool _suspicious{false};
		bool _troll{false};
		bool _role_faker{false};
		double _duel_strength{1};

		void _set_defaults_for_alignment();

		friend class Rulebook;
	};

	string_view alias(Role::ID id);
}

#endif
