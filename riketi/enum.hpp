#ifndef RIKETI_ENUM
#define RIKETI_ENUM

#include <type_traits>

namespace rkt {
   // Get the underlying value of x.
   template <class Enum>
   constexpr std::underlying_type_t<Enum> value(Enum x) {
      return static_cast<std::underlying_type_t<Enum>>(x);
   }
}

#endif
