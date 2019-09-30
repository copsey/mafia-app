#ifndef MAFIA_LOGIC_WILDCARD
#define MAFIA_LOGIC_WILDCARD

#include <functional>
#include <map>
#include <random>
#include <vector>

#include "role.hpp"

namespace maf {
	struct Rulebook;

	struct Wildcard {
		enum class ID {
			any,
			village,
			village_basic,
			mafia,
			freelance
		};

		/// Function type for assigning weights to roles.
		using Role_evaluator = std::function<double(const Role &)>;

		/// Create a new wildcard with the given role evaluator.
		Wildcard(ID id, Role_evaluator evaluator) :
			_id{id},
			_evaluator{evaluator}
		{ }

		/// Create a new wildcard with the given role weights.
		///
		/// `weights` must consist entirely of non-negative values, with at least
		/// one strictly positive value.
		Wildcard(ID id, const std::map<Role::ID, double> & weights);

		/// The ID of the wildcard.
		ID id() const {
			return _id;
		}

		/// The alias of the wildcard.
		///
		/// Note that this is fully determined by its ID.
		const char * alias() const;

		/// Check whether the wildcard will only return roles of the given
		/// alignment from `rulebook`.
		bool matches_alignment(Alignment alignment, const Rulebook & rulebook) const;

		/// Choose a role from `rulebook`, using the wildcard's distribution.
		///
		/// If the wildcard uses an evaluator, it must be such that all of the
		/// roles in `rulebook` are assigned non-negative values, and at least one
		/// role in `rulebook` is assigned a strictly positive value.
		///
		/// If instead the wildcard uses weights, each role with a positive weight
		/// must be defined in `rulebook`.
		const Role & pick_role(const Rulebook & rulebook);

	private:
		ID _id;
		Role_evaluator _evaluator{};
		std::vector<Role::ID> _role_ids{};
		std::discrete_distribution<decltype(_role_ids)::size_type> _dist{};

		/// Whether the wildcard uses a role evaluator when picking a role.
		///
		/// If false, predefined weights are used instead.
		bool uses_evaluator() const {
			return static_cast<bool>(_evaluator);
		}
	};

	/// The alias corresponding to the given wildcard ID.
	const char * alias(Wildcard::ID id);
}

#endif
