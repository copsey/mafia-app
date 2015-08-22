#include <map>

#include "alias.hpp"

using rID = mafia::Role::ID;

std::map<rID, std::string> role_id_to_alias_map{
   {rID::peasant, "peasant"},
   {rID::racketeer, "racketeer"},
   {rID::coward, "coward"}
};

std::map<std::string, rID> alias_to_role_id_map{
   {"peasant", rID::peasant},
   {"racketeer", rID::racketeer},
   {"coward", rID::coward}
};




std::string mafia::full_name(const Role &role) {
   return full_name(role.id());
}

std::string mafia::full_name(Role::ID id) {
   switch (id) {
      case Role::ID::peasant:
         return "Peasant";

      case Role::ID::racketeer:
         return "Racketeer";

      case Role::ID::coward:
         return "Coward";
   }
}

std::string mafia::alias(const mafia::Role &role) {
   return alias(role.id());
}

std::string mafia::alias(Role::ID id) {
   return role_id_to_alias_map[id];
}

mafia::Role::ID mafia::role_id_from_alias(const std::string &str) {
   return alias_to_role_id_map.at(str);
}
