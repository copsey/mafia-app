#ifndef RIKETI_STRING
#define RIKETI_STRING

#include <string>

namespace rkt {
   /// Check if s1 and s2 are equal, ignoring differences in case.
   bool equal_up_to_case (const std::string & s1, const std::string & s2);

   /// Convert str into an int, using the given numeric base.
   ///
   /// In addition to throwing the same exceptions as std::stoi(str, 0, base),
   /// a std::invalid_argument is thrown if some unexpected superfluous chars
   /// were encountered at the end of str.
   int to_i (const std::string & str, int base = 10);

   /// Convert str into a long, using the given numeric base.
   ///
   /// In addition to throwing the same exceptions as std::stol(str, 0, base),
   /// a std::invalid_argument is thrown if some unexpected superfluous chars
   /// were encountered at the end of str.
   long to_l (const std::string & str, int base = 10);

   /// Convert str into a long long, using the given numeric base.
   ///
   /// In addition to throwing the same exceptions as std::stoll(str, 0, base),
   /// a std::invalid_argument is thrown if some unexpected superfluous chars
   /// were encountered at the end of str.
   long long to_ll (const std::string & str, int base = 10);

   /// Convert str into an unsigned long, using the given numeric base.
   ///
   /// In addition to throwing the same exceptions as std::stoul(str, 0, base),
   /// a std::invalid_argument is thrown if some unexpected superfluous chars
   /// were encountered at the end of str.
   unsigned long to_ul (const std::string & str, int base = 10);

   /// Convert str into an unsigned long long, using the given numeric base.
   ///
   /// In addition to throwing the same exceptions as std::stoull(str, 0, base),
   /// a std::invalid_argument is thrown if some unexpected superfluous chars
   /// were encountered at the end of str.
   unsigned long long to_ull (const std::string & str, int base = 10);
}

#endif
