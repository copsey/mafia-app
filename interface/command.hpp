#ifndef MAFIA_INTERFACE_COMMAND
#define MAFIA_INTERFACE_COMMAND

#include <string_view>
#include <vector>

#include "../riketi/algorithm.hpp"

namespace maf {
   // Decide whether or not sequence `seq` of string-like objects matches the
   // array `arr` of commands, which is true exactly when `std::size(seq) == std::size(arr)`
   // and at each position `i`, either `std::empty(arr[i])` or `seq[i] == arr[i]`.
   template <typename Seq, std::size_t N>
   bool commands_match(const Seq & seq, const std::string_view (&arr)[N]);

   template <typename Str, std::size_t N>
   bool commands_match(const std::vector<Str> & v, const std::string_view (&arr)[N]);

   // Split `str` into a sequence of commands, delimited by whitespace (' ' and '\t').
   //    e.g. "do X   with Y" -> {"do", "X", "with", "Y"}
   //
   // @returns a vector of views into `str`.
   std::vector<std::string_view> parse_input(std::string_view str);
}



template <typename Seq, std::size_t N>
bool maf::commands_match(const Seq & seq, const std::string_view (&arr)[N])
{
   auto eq = [](auto& s1, std::string_view s2) {
      return std::empty(s2) || s1 == s2;
   };

   return rkt::matches(seq, arr, eq);
}

template <typename Str, std::size_t N>
bool maf::commands_match(const std::vector<Str> & v, const::std::string_view (&arr)[N])
{
   auto eq = [](auto& s1, std::string_view s2) {
      return std::empty(s2) || s1 == s2;
   };

   if constexpr(N == 0) {
      return std::size(v) == 0;
   } else if constexpr(N == 1) {
      return std::size(v) == 1 && eq(v[0], arr[0]);
   } else if constexpr(N == 2) {
      return std::size(v) == 2 && eq(v[0], arr[0]) && eq(v[1], arr[1]);
   } else if constexpr(N == 3) {
      return std::size(v) == 3 && eq(v[0], arr[0]) && eq(v[1], arr[1]) && eq(v[2], arr[2]);
   } else if constexpr(N == 4) {
      return std::size(v) == 4 && eq(v[0], arr[0]) && eq(v[1], arr[1]) && eq(v[2], arr[2]) && eq(v[3], arr[3]);
   } else if constexpr(N == 5) {
      return std::size(v) == 5 && eq(v[0], arr[0]) && eq(v[1], arr[1]) && eq(v[2], arr[2]) && eq(v[3], arr[3]) && eq(v[4], arr[4]);
   } else {
      return rkt::matches(v, arr, eq);
   }
}

#endif
