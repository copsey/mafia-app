#ifndef MAFIA_RULEBOOK_H
#define MAFIA_RULEBOOK_H

#include "joker.hpp"

namespace mafia {
   struct Rulebook {
      static constexpr int latest_edition = 1;

      Rulebook(int edition = latest_edition);

      const std::vector<Role> &roles() const {
         return _roles;
      }

      bool contains_role(Role::ID id) const {
         return find_role(id) != _roles.end();
      }

      Role & get_role(Role::ID id);

      template <typename F>
      void for_roles(F f);

      Role & new_village_role(Role::ID id);
      Role & new_mafia_role(Role::ID id);
      Role & new_freelance_role(Role::ID id);

      bool contains_joker(Joker::ID id) const;

      Joker & get_joker(Joker::ID id);

      Joker & new_joker(Joker::ID id, Joker::Role_evaluator evaluator);
      Joker & new_joker(Joker::ID id, std::map<Role::ID, double> weights);

   private:
      int _edition;
      std::vector<Role> _roles{};
      std::vector<Joker> _jokers{};

      // Add a blank role with the given ID.
      Role & new_role(Role::ID id);

      // Obtain an iterator to the role in _roles with the given ID, or
      // _roles.end() if none could be found.
      std::vector<Role>::iterator find_role(Role::ID id);
      std::vector<Role>::const_iterator find_role(Role::ID id) const;

      // Obtain an iterator to the joker in _jokers with the given ID, or
      // _jokers.end() if none could be found.
      std::vector<Joker>::iterator find_joker(Joker::ID id);
      std::vector<Joker>::const_iterator find_joker(Joker::ID id) const;
   };
}




/* TEMPLATE IMPLEMENTATIONS */

template <typename F>
void mafia::Rulebook::for_roles(F f) {
   for (Role &role : _roles) {
      f(role);
   }
}

#endif
