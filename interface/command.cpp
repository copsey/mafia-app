#include "command.hpp"

std::vector<std::string_view> maf::parse_input(std::string_view str) {
   std::vector<std::string_view> v = {};

   {
      auto i = str.data(), j = i;
      auto end = str.data() + str.size();

      for ( ; j != end; ) {
         if (*j == ' ' || *j == '\t') {
            if (i != j) v.emplace_back(i, j - i);
            i = ++j;
         } else {
            ++j;
         }
      }

      if (i != j) v.emplace_back(i, j - i);
   }

   return v;
}
