#include "role.hpp"

mafia::Role::Role(ID id): _id{id} { }

mafia::Role::ID mafia::Role::id() const {
   return _id;
}

std::string mafia::Role::alias() const {
   return mafia::alias(_id);
}

std::string mafia::alias(Role::ID id) {
   switch (id) {
      case Role::ID::peasant:
         return "peas";
      case Role::ID::racketeer:
         return "rack";
      case Role::ID::coward:
         return "coward";
      case Role::ID::serial_killer:
         return "sk";
      case Role::ID::musketeer:
         return "musk";
   }
}