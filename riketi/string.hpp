#ifndef RIKETI_STRING
#define RIKETI_STRING

#include <cctype>
#include <string>

namespace rkt {
   // Check if two strings are equal, ignoring differences in case.
   inline bool equal_up_to_case(const std::string &s1, const std::string &s2) {
      auto n = s1.size();
      if (s2.size() != n) return false;

      for (std::string::size_type i{0}; i != n; ++i) {
         if (std::toupper(s1[i]) != std::toupper(s2[i])) return false;
      }

      return true;
   }
}

#endif
