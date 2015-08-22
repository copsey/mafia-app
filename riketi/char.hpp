#ifndef RIKETI_CHAR
#define RIKETI_CHAR

#include <cctype>
#include <sstream>
#include <stdexcept>

namespace rkt {
   // Check if two chars are equal, ignoring differences in case.
   inline bool equal_up_to_case(char c1, char c2) {
      return std::toupper(c1) == std::toupper(c2);
   }

   // Convert a decimal digit into a T.
   // Throws an exception if c is not a decimal digit.
   template <typename T = int>
   T dec_digit(char c) {
      switch (c) {
         case '0':
            return 0;
         case '1':
            return 1;
         case '2':
            return 2;
         case '3':
            return 3;
         case '4':
            return 4;
         case '5':
            return 5;
         case '6':
            return 6;
         case '7':
            return 7;
         case '8':
            return 8;
         case '9':
            return 9;
         default: {
            std::ostringstream os{};
            os << "Attempted to convert the character '" << c
               << "' into a decimal digit, via rkt::dec_digit.";

            throw std::invalid_argument{os.str()};
         }
      }
   }

   // Convert a hexadecimal digit into a T.
   // Throws an exception if c is not a hexadecimal digit.
   template <typename T = int>
   T hex_digit(char c) {
      switch (c) {
         case '0':
            return 0;
         case '1':
            return 1;
         case '2':
            return 2;
         case '3':
            return 3;
         case '4':
            return 4;
         case '5':
            return 5;
         case '6':
            return 6;
         case '7':
            return 7;
         case '8':
            return 8;
         case '9':
            return 9;
         case 'a':
         case 'A':
            return 10;
         case 'b':
         case 'B':
            return 11;
         case 'c':
         case 'C':
            return 12;
         case 'd':
         case 'D':
            return 13;
         case 'e':
         case 'E':
            return 14;
         case 'f':
         case 'F':
            return 15;
         default: {
            std::ostringstream os{};
            os << "Attempted to convert the character '" << c
               << "' into a hexadecimal digit, via rkt::hex_digit.";
            
            throw std::invalid_argument{os.str()};
         }
      }
   }
}

#endif
