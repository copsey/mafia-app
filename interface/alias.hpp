#ifndef MAFIA_ALIAS_H
#define MAFIA_ALIAS_H

#include "../logic/logic.hpp"

namespace mafia {
   // The full name of the given role, in English.
   std::string full_name(const Role &role);

   // The full name of the role with the given ID, in English.
   std::string full_name(Role::ID id);

   // The alias of the given role.
   std::string alias(const Role &role);

   // The alias of the role with the given ID.
   std::string alias(Role::ID id);

   // The ID of the role with the given alias.
   Role::ID role_id_from_alias(const std::string &str);
}

#endif
