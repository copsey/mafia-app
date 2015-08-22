#include <sstream>
#include <stdexcept>

#include "styled_string.hpp"

mafia::Styled_text mafia::to_styled_text(const std::string &tagged_str) {
   Styled_text text{};

   std::string str{};
   std::string str_piece{};
   Styled_string::Style style{Styled_string::Style::game};
   std::istringstream ss{tagged_str};

   while (std::getline(ss, str_piece, '^')) {
      Styled_string::Style new_style{style};
      auto x = ss.get();
      switch (x) {
         case 'g':
            new_style = Styled_string::Style::game;
            break;

         case 'G':
            new_style = Styled_string::Style::game_title;
            break;

         case 'i':
            new_style = Styled_string::Style::game_italic;
            break;

         case 'h':
            new_style = Styled_string::Style::help;
            break;

         case 'H':
            new_style = Styled_string::Style::help_title;
            break;

         case 'c':
            new_style = Styled_string::Style::command;
            break;

         case '^':
            str_piece += '^';
            break;

         case std::istringstream::traits_type::eof():
            if (tagged_str.back() == '^') {
               std::ostringstream err_ss{};
               err_ss << "An odd number of '^' must not end a tagged string, "
                      << "and this happens in the following tagged string:\n"
                      << tagged_str;

               throw std::invalid_argument{err_ss.str()};
            }
            break;

         default: {
            std::ostringstream err_ss{};
            err_ss << "The tag ^" << static_cast<char>(x) << " is invalid, and "
            << "appears in the following tagged string:\n" << tagged_str;

            throw std::invalid_argument{err_ss.str()};
         }
      }

      str += str_piece;

      if (style != new_style) {
         if (!str.empty()) {
            text.emplace_back(str, style);
            str.clear();
         }

         style = new_style;
      }
   }
   
   if (!str.empty()) text.emplace_back(str, style);

   return text;
}
