#ifndef MAFIA_LOGIC_RULEBOOK
#define MAFIA_LOGIC_RULEBOOK

#include "../riketi/ref.hpp"

#include "wildcard.hpp"

namespace mafia {
   struct Rulebook {
      // An edition of the rules.
      using Edition = int;
      // The most recent edition of the rules.
      static constexpr Edition latest_edition{1};

      // An exception signifying that no rulebook with the given edition exists.
      struct Bad_edition {
         Edition edition;
      };

      // Signifies that the rulebook doesn't contain a role with the given ID.
      struct Missing_role_ID {
         Role::ID id;
      };

      // Signifies that the rulebook doesn't contain a role with the given
      // alias.
      // (this could be because the alias doesn't correspond to a role ID.)
      struct Missing_role_alias {
         std::string alias;
      };

      // Signifies that the rulebook doesn't contain a wildcard with the given
      // ID.
      struct Missing_wildcard_ID {
         Wildcard::ID id;
      };

      // Signifies that the rulebook doesn't contain a wildcard with the given
      // alias.
      // (this could be because the alias doesn't correspond to a wildcard ID.)
      struct Missing_wildcard_alias {
         std::string alias;
      };

      // Creates a rulebook of the latest edition.
      Rulebook();
      // Creates a rulebook of the specified edition.
      Rulebook(Edition edition);

      // Gets the edition of the rules being used.
      Edition edition() const;

      // A vector containing every role defined in the rulebook.
      const std::vector<Role> & roles() const;
      // A vector containing every village role defined in the rulebook.
      std::vector<rkt::ref<const Role>> village_roles() const;
      // A vector containing every mafia role defined in the rulebook.
      std::vector<rkt::ref<const Role>> mafia_roles() const;
      // A vector containing every freelance role defined in the rulebook.
      std::vector<rkt::ref<const Role>> freelance_roles() const;

      // A vector containing every wildcard defined in the rulebook.
      const std::vector<Wildcard> & wildcards() const;
      // A vector containing every village-only wildcard defined in the
      // rulebook.
      std::vector<rkt::ref<const Wildcard>> village_wildcards() const;
      // A vector containing every mafia-only wildcard defined in the rulebook.
      std::vector<rkt::ref<const Wildcard>> mafia_wildcards() const;
      // A vector containing every freelance-only wildcard defined in the
      // rulebook.
      std::vector<rkt::ref<const Wildcard>> freelance_wildcards() const;

      // Whether the rulebook contains a role with the given ID.
      bool contains_role(Role::ID id) const;
      // Whether the rulebook contains a role with the given alias.
      bool contains_role(const std::string &alias) const;
      // Whether the rulebook contains a wildcard with the given ID.
      bool contains_wildcard(Wildcard::ID id) const;
      // Whether the rulebook contains a wildcard with the given alias.
      bool contains_wildcard(const std::string &alias) const;

      // Gets the role with the given ID.
      // Throws an exception if none could be found.
      Role & get_role(Role::ID id);
      const Role & get_role(Role::ID id) const;
      // Gets the role with the given alias.
      // Throws an exception if none could be found.
      Role & get_role(const std::string &alias);
      const Role & get_role(const std::string &alias) const;
      // Gets the wildcard with the given ID.
      // Throws an exception if none could be found.
      Wildcard & get_wildcard(Wildcard::ID id);
      const Wildcard & get_wildcard(Wildcard::ID id) const;
      // Gets the wildcard with the given alias.
      // Throws an exception if none could be found.
      Wildcard & get_wildcard(const std::string &alias);
      const Wildcard & get_wildcard(const std::string &alias) const;

      // Creates and stores a blank role with the given ID.
      // A reference to the role is returned.
      Role & new_role(Role::ID id);
      // Creates and stores a role with the given ID and default village traits.
      // A reference to the role is returned.
      Role & new_village_role(Role::ID id);
      // Creates and stores a role with the given ID and default Mafia traits.
      // A reference to the role is returned.
      Role & new_mafia_role(Role::ID id);
      // Creates and stores a role with the given ID and default freelance
      // traits.
      // A reference to the role is returned.
      Role & new_freelance_role(Role::ID id);
      // Creates and stores a wildcard with the given ID and role evaluator.
      // A reference to the wildcard is returned.
      Wildcard & new_wildcard(Wildcard::ID id,
                              Wildcard::Role_evaluator evaluator);
      // Creates and stores a wildcard with the given ID and role weights.
      // A reference to the wildcard is returned.
      Wildcard & new_wildcard(Wildcard::ID id,
                              const std::map<Role::ID, double> &weights);

   private:
      Edition _edition;
      std::vector<Role> _roles{};
      std::vector<Wildcard> _wildcards{};
   };
}

#endif
