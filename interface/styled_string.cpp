#include <sstream>
#include <stdexcept>

#include "styled_string.hpp"

void maf::escape_style_codes(std::string& str) {
   for (std::string::size_type i = 0; i < str.size(); ++i) {
      if (str[i] == '^') {
         str.insert(i, 1, '^');
         ++i;
      }
   }
}

std::string maf::copy_with_escaped_style_codes(const std::string& str) {
   std::string str2 = str;
   escape_style_codes(str2);
   return str2;
}

maf::Styled_text maf::styled_text_from(const std::string &tagged_s) {
   std::istringstream iss{tagged_s};
   return styled_text_from(iss);
}

maf::Styled_text maf::styled_text_from(std::istream &is) {
   Styled_text text{};

   std::string str{};
   std::string str_piece{};
   Styled_string::Style style{Styled_string::Style::game};

   while (std::getline(is, str_piece, '^')) {
      Styled_string::Style new_style{style};

      switch (auto x = is.get()) {
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

         case std::istream::traits_type::eof(): {
            auto state = is.rdstate();
            is.clear();

            is.unget();
            if (is.get() == '^') {
               is.seekg(0);

               std::ostringstream err{};
               err << "There is a dangling '^' at the end of the following tagged string:\n"
                   << is.rdbuf();

               throw std::invalid_argument(err.str());
            }

            is.setstate(state);
            break;
         }

         default: {
            is.seekg(0);

            std::ostringstream err{};
            err << "The tag ^"
                << static_cast<char>(x)
                << " is invalid, and appears in the following tagged string:\n"
                << is.rdbuf();

            throw std::invalid_argument(err.str());
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
