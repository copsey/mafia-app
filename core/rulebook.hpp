#ifndef MAFIA_CORE_RULEBOOK_H
#define MAFIA_CORE_RULEBOOK_H

#include <map>
#include <string>
#include <vector>

#include "../util/algorithm.hpp"
#include "../util/stdlib.hpp"

#include "role.hpp"
#include "wildcard.hpp"

namespace maf::core {
	class RoleRef;

	class Rulebook {
	public:
		/// Edition of the rules.
		///
		/// Although not used at the moment, this primarily exists for supporting
		/// saved games which may have been created with a rulebook incompatible
		/// with the most recent rulebook.
		using Edition = int;

		/// The most recent edition of the rules.
		static constexpr Edition latest_edition{1};

		/// Exception signifying that no rulebook with the given edition exists.
		struct Bad_edition {
			Edition edition;
		};

		/// Exception signifying that no role could be found with the given ID.
		struct Missing_role_ID {
			Role::ID id;
		};

		/// Exception signifying that no role could be found with the given alias.
		///
		/// This could be because the alias doesn't correspond to a role ID.
		struct Missing_role_alias {
			string alias;
		};

		/// Exception signifying that a role with the given ID already exists.
		struct Preexisting_role_ID {
			Role::ID id;
		};

		/// Exception signifying that no wildcard could be found with
		/// the given ID.
		struct Missing_wildcard_ID {
			Wildcard::ID id;
		};

		/// Exception signifying that no wildcard could be found with the
		/// given alias.
		///
		/// This could be because the alias doesn't correspond to a wildcard ID.
		struct Missing_wildcard_alias {
			string alias;
		};

		/// Exception signifying that a wildcard with the given ID already exists.
		struct Preexisting_wildcard_ID {
			Wildcard::ID id;
		};

		/// Type for iterators used to define sequences of roles within rulebooks.
		using Role_Iterator = vector<Role>::const_iterator;

		/// Make a rulebook with the latest edition.
		Rulebook()
			: Rulebook{latest_edition}
		{ }

		/// Make a rulebook with the specified edition.
		Rulebook(Edition edition);

		/// The edition of the rules being used.
		Edition edition() const {
			return _edition;
		}

		/// An iterator defining the beginning of a sequence containing all
		/// roles present in this rulebook.
		Role_Iterator roles_begin() const {
			return _roles.begin();
		}

		/// An iterator defining the end of a sequence containing all roles
		/// present in this rulebook.
		Role_Iterator roles_end() const {
			return _roles.end();
		}

		/// A vector containing every role defined in the rulebook.
		vector_of_refs<const Role> roles() const {
			return util::get_crefs(_roles);
		}

		/// Evaluate the function `f` on each role present in this rulebook.
		template <typename F>
		void for_each_role(F f) const {
			for (const Role & r: _roles) f(r);
		}

		/// A vector containing every wildcard defined in the rulebook.
		vector_of_refs<const Wildcard> wildcards() const {
			return util::get_crefs(_wildcards);
		}

		/// Whether the rulebook contains a given role.
		bool contains(RoleRef r_ref) const;

		/// Whether the rulebook contains a wildcard with the given ID.
		bool contains_wildcard(Wildcard::ID id) const;

		/// Whether the rulebook contains a wildcard with the given alias.
		bool contains_wildcard(string_view alias) const;

		/// Get the role with the given ID.
		///
		/// @throws `Missing_role_ID` if none could be found.
		Role & get_role(Role::ID id);

		/// Get the role with the given alias.
		///
		// @throws `Missing_role_alias` if none could be found.
		Role & get_role(string_view alias);

		/// Get the role with the given alias.
		///
		/// Throw `std::out_of_range` if none could be found.
		Role const& look_up(RoleRef r_ref) const;

		/// Get the wildcard with the given ID.
		///
		/// @throws `Missing_wildcard_ID` if none could be found.
		Wildcard & get_wildcard(Wildcard::ID id);

		/// Get the wildcard with the given ID.
		///
		/// @throws `Missing_wildcard_ID` if none could be found.
		const Wildcard & get_wildcard(Wildcard::ID id) const;

		/// Get the wildcard with the given alias.
		///
		/// @throws `Missing_wildcard_alias` if none could be found.
		Wildcard & get_wildcard(string_view alias);

		/// Get the wildcard with the given alias.
		///
		/// @throws `Missing_wildcard_alias` if none could be found.
		const Wildcard & get_wildcard(string_view alias) const;

		/// Create and store a new role with the given ID.
		///
		/// The traits of the role are set to their default values.
		///
		/// @returns a reference to the new role.
		///
		/// @throws `Preexisting_role_ID` if a role with the given ID is already
		/// defined in the rulebook.
		Role & new_role(Role::ID id);

		/// Create and store a new role with the given ID, and default village traits.
		///
		/// @returns a reference to the new role.
		///
		/// @throws `Preexisting_role_ID` if a role with the given ID is already
		/// defined in the rulebook.
		Role & new_village_role(Role::ID id);

		/// Create and store a new role with the given ID, and default mafia traits.
		///
		/// @returns a reference to the new role.
		///
		/// @throws `Preexisting_role_ID` if a role with the given ID is already
		/// defined in the rulebook.
		Role & new_mafia_role(Role::ID id);

		/// Create and store a new role with the given ID, and default freelance traits.
		///
		/// @returns a reference to the new role.
		///
		/// @throws `Preexisting_role_ID` if a role with the given ID is already
		/// defined in the rulebook.
		Role & new_freelance_role(Role::ID id);

		/// Create and store a new wildcard with the given ID and role evaluator.
		///
		/// @returns a reference to the new wildcard.
		///
		/// @throws `Preexisting_wildcard_ID` if a wildcard with the given ID is
		/// already defined in the rulebook.
		Wildcard & new_wildcard(Wildcard::ID id, Wildcard::Role_evaluator evaluator);

		/// Create and store a new wildcard with the given ID and role weights.
		///
		/// @returns a reference to the new wildcard.
		///
		/// @throws `Preexisting_wildcard_ID` if a wildcard with the given ID is
		/// already defined in the rulebook.
		Wildcard & new_wildcard(Wildcard::ID id, const std::map<Role::ID, double> & weights);

	private:
		Edition _edition;
		vector<Role> _roles{};
		vector<Wildcard> _wildcards{};
	};
}

#endif
