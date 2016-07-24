#ifndef RIKETI_ENUM
#define RIKETI_ENUM

#include <type_traits>

namespace rkt {
   /// The underlying type of Enum.
   template <class Enum>
   using underlying_type = typename std::underlying_type<Enum>::type;

   /// The underlying value of x.
   template <class Enum>
   constexpr underlying_type<Enum> value (Enum x) {
      return static_cast<underlying_type<Enum>>(x);
   }
}

#endif
