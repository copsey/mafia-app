#include <cctype>
#include <cstddef>
#include <stdexcept>

#include "string.hpp"

bool rkt::equal_up_to_case (const std::string & s1, const std::string & s2) {
   auto n = s1.size();
   if (s2.size() != n) return false;

   for (std::string::size_type i = 0; i != n; ++i) {
      if (std::toupper(s1[i]) != std::toupper(s2[i])) return false;
   }

   return true;
}

int rkt::to_i (const std::string & str, int base) {
   std::size_t pos;
   auto i = std::stoi(str, &pos, base);

   if (pos < str.size()) {
      throw std::invalid_argument{"rkt::to_i: excess chars"};
   }

   return i;
}

long rkt::to_l (const std::string & str, int base) {
   std::size_t pos;
   auto i = std::stol(str, &pos, base);

   if (pos < str.size()) {
      throw std::invalid_argument{"rkt::to_l: excess chars"};
   }

   return i;
}

long long rkt::to_ll (const std::string & str, int base) {
   std::size_t pos;
   auto i = std::stoll(str, &pos, base);

   if (pos < str.size()) {
      throw std::invalid_argument{"rkt::to_ll: excess chars"};
   }

   return i;
}

unsigned long rkt::to_ul (const std::string & str, int base) {
   std::size_t pos;
   auto i = std::stoul(str, &pos, base);

   if (pos < str.size()) {
      throw std::invalid_argument{"rkt::to_ul: excess chars"};
   }

   return i;
}

unsigned long long rkt::to_ull (const std::string & str, int base) {
   std::size_t pos;
   auto i = std::stoull(str, &pos, base);

   if (pos < str.size()) {
      throw std::invalid_argument{"rkt::to_ull: excess chars"};
   }

   return i;
}
