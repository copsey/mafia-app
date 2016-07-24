#ifndef RIKETI_CHAR
#define RIKETI_CHAR

#include <cctype>
#include <stdexcept>

namespace rkt {
   /// Check if c1 and c2 are equal, ignoring differences in case.
   inline bool equal_up_to_case(char c1, char c2) {
      return std::toupper(c1) == std::toupper(c2);
   }

   /// Convert the decimal digit c into a T.
   ///
   /// If c is not a decimal digit, a std::invalid_argument is thrown.
   template <class T>
   T dec_digit_to(char c) {
      switch (c) {
       case '0':
         return T{0};
       case '1':
         return T{1};
       case '2':
         return T{2};
       case '3':
         return T{3};
       case '4':
         return T{4};
       case '5':
         return T{5};
       case '6':
         return T{6};
       case '7':
         return T{7};
       case '8':
         return T{8};
       case '9':
         return T{9};
       default:
         throw std::invalid_argument{"rkt::dec_digit_to: non-decimal digit"};
      }
   }

   /// Convert the hexadecimal digit c into a T.
   ///
   /// If c is not a hexadecimal digit, a std::invalid_argument is thrown.
   template <class T>
   T hex_digit_to(char c) {
      switch (c) {
       case '0':
         return T{0};
       case '1':
         return T{1};
       case '2':
         return T{2};
       case '3':
         return T{3};
       case '4':
         return T{4};
       case '5':
         return T{5};
       case '6':
         return T{6};
       case '7':
         return T{7};
       case '8':
         return T{8};
       case '9':
         return T{9};
       case 'a':
       case 'A':
         return T{10};
       case 'b':
       case 'B':
         return T{11};
       case 'c':
       case 'C':
         return T{12};
       case 'd':
       case 'D':
         return T{13};
       case 'e':
       case 'E':
         return T{14};
       case 'f':
       case 'F':
         return T{15};
       default:
         throw std::invalid_argument{"rkt::hex_digit_to: non-hexadecimal digit"};
      }
   }
}

#endif
