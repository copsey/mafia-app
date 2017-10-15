#include <sstream>
#include <stdexcept>

#include "styled_string.hpp"

maf::Styled_text maf::styled_text_from(const std::string &tagged_s) {
   std::istringstream iss{tagged_s};
   return styled_text_from(iss);
}

maf::Styled_text maf::styled_text_from(std::istream &is) {
   auto text = Styled_text{};

   auto str = std::string{};
   auto str_piece = std::string{};

   auto style = Styled_string::Style::game;
   constexpr int style_stack_size{8};
   Styled_string::Style style_stack[style_stack_size] = {};
   auto style_stack_index = int{-1};

   auto new_style = Styled_string::Style::game;
   bool push_style;
   bool pop_style;

   while (std::getline(is, str_piece, '^')) {
      push_style = false;
      pop_style = false;

      switch (auto x = is.get()) {
         case 'g':
            push_style = true;
            new_style = Styled_string::Style::game;
            break;

         case 'h':
            push_style = true;
            new_style = Styled_string::Style::help;
            break;

         case 'i':
            push_style = true;
            new_style = Styled_string::Style::italic;
            break;

         case 'c':
            push_style = true;
            new_style = Styled_string::Style::command;
            break;

         case 'T':
            push_style = true;
            new_style = Styled_string::Style::title;
            break;

         case '/': {
            pop_style = true;
            break;
         }

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
               err << "There is a dangling '^' at the end of the following tagged string:\n";
               err << is.rdbuf();

               throw std::invalid_argument(err.str());
            }

            is.setstate(state);
            break;
         }

         default: {
            is.seekg(0);

            std::ostringstream err{};
            err << "The tag ^";
            err << static_cast<char>(x);
            err << " is invalid, and appears in the following tagged string:\n";
            err << is.rdbuf();

            throw std::invalid_argument(err.str());
         }
      }

      str += str_piece;

      if (push_style) {
         style_stack_index ++;

         if (style_stack_index == style_stack_size) {
            is.seekg(0);

            std::ostringstream err{};
            err << "Attempted to push too many style tags onto the stack, in the following tagged string:\n";
            err << is.rdbuf();

            throw std::invalid_argument(err.str());
         }

         if (new_style != style && !str.empty()) {
            text.emplace_back(str, style);
            str.clear();
         }

         style_stack[style_stack_index] = style;
         style = new_style;
      } else if (pop_style) {
         if (style_stack_index == -1) {
            is.seekg(0);

            std::ostringstream err{};
            err << "Attempted to pop too many style tags from the stack, in the following tagged string:\n";
            err << is.rdbuf();

            throw std::invalid_argument(err.str());
         }

         new_style = style_stack[style_stack_index];

         if (new_style != style && !str.empty()) {
            text.emplace_back(str, style);
            str.clear();
         }

         style = new_style;
         style_stack_index --;
      }
   }

   if (!str.empty()) text.emplace_back(str, style);

   return text;
}
