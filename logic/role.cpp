#include "role.hpp"

const char * maf::Role::alias() const {
   return maf::alias(_id);
}

const char * maf::alias(Role::ID id) {
   switch (id) {
      case Role::ID::peasant:
         return "peas";
      case Role::ID::doctor:
         return "doc";
      case Role::ID::detective:
         return "det";
      case Role::ID::racketeer:
         return "rack";
      case Role::ID::godfather:
         return "gf";
      case Role::ID::dealer:
         return "dealer";
      case Role::ID::coward:
         return "coward";
      case Role::ID::actor:
         return "actor";
      case Role::ID::serial_killer:
         return "sk";
      case Role::ID::village_idiot:
         return "vi";
      case Role::ID::musketeer:
         return "musk";
   }
}
