#ifndef MAFIA_LOGIC_RULEBOOK
#define MAFIA_LOGIC_RULEBOOK

#include <map>
#include <string>
#include <vector>

#include "../riketi/ref.hpp"

#include "role.hpp"
#include "wildcard.hpp"

namespace maf {
   struct Rulebook {
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
         std::string alias;
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
         std::string alias;
      };

      /// Make a rulebook with the latest edition.
      Rulebook()
       : Rulebook{latest_edition} { }

      /// Make a rulebook with the specified edition.
      Rulebook(Edition edition);

      /// The edition of the rules being used.
      Edition edition() const {
         return _edition;
      }

      /// A vector containing every role defined in the rulebook.
      const std::vector<Role> & roles() const {
         return _roles;
      }

      /// A vector containing every village role defined in the rulebook.
      std::vector<rkt::ref<const Role>> village_roles() const;

      /// A vector containing every mafia role defined in the rulebook.
      std::vector<rkt::ref<const Role>> mafia_roles() const;

      /// A vector containing every freelance role defined in the rulebook.
      std::vector<rkt::ref<const Role>> freelance_roles() const;

      /// A vector containing every wildcard defined in the rulebook.
      const std::vector<Wildcard> & wildcards() const {
         return _wildcards;
      }

      /// A vector containing every village-only wildcard defined in the
      /// rulebook.
      std::vector<rkt::ref<const Wildcard>> village_wildcards() const;

      /// A vector containing every mafia-only wildcard defined in the rulebook.
      std::vector<rkt::ref<const Wildcard>> mafia_wildcards() const;

      /// A vector containing every freelance-only wildcard defined in the
      /// rulebook.
      std::vector<rkt::ref<const Wildcard>> freelance_wildcards() const;

      /// Whether the rulebook contains a role with the given ID.
      bool contains_role(Role::ID id) const;

      /// Whether the rulebook contains a role with the given alias.
      bool contains_role(const std::string & alias) const;

      /// Whether the rulebook contains a wildcard with the given ID.
      bool contains_wildcard(Wildcard::ID id) const;

      /// Whether the rulebook contains a wildcard with the given alias.
      bool contains_wildcard(const std::string & alias) const;

      /// Get the role with the given ID.
      ///
      /// @throws `Missing_role_ID` if none could be found.
      Role & get_role(Role::ID id);

      /// Get the role with the given ID.
      ///
      /// @throws `Missing_role_ID` if none could be found.
      const Role & get_role(Role::ID id) const;

      /// Get the role with the given alias.
      ///
      // @throws `Missing_role_alias` if none could be found.
      Role & get_role(const std::string & alias);

      /// Get the role with the given alias.
      ///
      // @throws `Missing_role_alias` if none could be found.
      const Role & get_role(const std::string & alias) const;

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
      Wildcard & get_wildcard(const std::string & alias);

      /// Get the wildcard with the given alias.
      ///
      /// @throws `Missing_wildcard_alias` if none could be found.
      const Wildcard & get_wildcard(const std::string & alias) const;

      /// Create and store a new role with the given ID.
      ///
      /// The traits of the role are set to their default values.
      ///
      /// @returns a reference to the new role.
      Role & new_role(Role::ID id);

      /// Create and store a new role with the given ID, and default village traits.
      ///
      /// @returns a reference to the new role.
      Role & new_village_role(Role::ID id);

      /// Create and store a new role with the given ID, and default mafia traits.
      ///
      /// @returns a reference to the new role.
      Role & new_mafia_role(Role::ID id);

      /// Create and store a new role with the given ID, and default freelance traits.
      ///
      /// @returns a reference to the new role.
      Role & new_freelance_role(Role::ID id);

      /// Create and store a new wildcard with the given ID and role evaluator.
      ///
      /// @returns a reference to the new wildcard.
      Wildcard & new_wildcard(Wildcard::ID id, Wildcard::Role_evaluator evaluator);

      /// Create and store a new wildcard with the given ID and role weights.
      ///
      /// @returns a reference to the new wildcard.
      Wildcard & new_wildcard(Wildcard::ID id, const std::map<Role::ID, double> & weights);

   private:
      Edition _edition;
      std::vector<Role> _roles{};
      std::vector<Wildcard> _wildcards{};
   };
}

#endif
