#include <algorithm>
#include <stdexcept>

#include "styled_string.hpp"

std::string maf::escape_tags(std::string_view str) {
   std::string esc_str = {};

   for (auto ch: str) {
      if (ch == '^' || ch == '{' || ch == '}') esc_str.push_back('^');
      esc_str.push_back(ch);
   }

   return esc_str;
}

maf::Styled_text maf::styled_text_from(std::string_view tagged_str) {
   std::string str = {};
   Styled_text text = {};

   Styled_string::Style style_stack[8] = {Styled_string::Style::game};
   auto style_i = std::begin(style_stack);

   for (auto i = tagged_str.begin(), j = i, end = tagged_str.end(); i != end; ) {
      j = std::find(j, end, '^');
      
      if (j == end) {
         str.append(i, j);

         i = j;
      }
      else {
         // e.g.
         //      i                  j
         //      |                  |
         // ...^^ some example text ^cand so on...

         ++j;

         if (j == end) {
            std::string err_msg = {};
            err_msg.append("There is a dangling '^' at the end of the following tagged string:\n");
            err_msg.append(tagged_str);

            throw std::invalid_argument(err_msg);
         }
         else {
            // e.g.
            //      i                   j
            //      |                   |
            // ...^^ some example text ^cand so on...

            auto new_style = Styled_string::Style::game;
            bool push_style = false;
            bool pop_style = false;

            switch (auto ch = *j) {
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

               case '/':
                  pop_style = true;
                  break;

               // replace "^^" with "^" in output
               case '^': {
                  str.append(i, j-1);
                  str.append('^');
                  i = ++j;
                  break;
               }
               
               // replace "^{" with "{" in output
               case '{': {
                  str.append(i, j-1);
                  std.append('{');
                  i = ++j;
                  break;
               }
               
               // replace "^}" with "}" in output
               case '}': {
                  str.append(i, j-1);
                  std.append('}');
                  i = ++j;
                  break;
               }

               default: {
                  std::string err_msg = {};
                  err_msg.append("The tag ^");
                  err_msg += ch;
                  err_msg.append(" is invalid, and appears in the following tagged string:\n");
                  err_msg.append(tagged_str);

                  throw std::invalid_argument(err_msg);
               }
            }

            if (push_style) {
               str.append(i, j - 1); // [i,j-1) excludes the tag "^x"

               if (*style_i != new_style) {
                  if (!str.empty()) {
                     text.emplace_back(str, *style_i);
                     str.clear();
                  }

                  ++style_i;

                  if (style_i == std::end(style_stack)) {
                     std::string err_msg = {};
                     err_msg.append("Attempted to push too many style tags onto the stack, in the following tagged string:\n");
                     err_msg.append(tagged_str);

                     throw std::invalid_argument(err_msg);
                  } else {
                     *style_i = new_style;
                  }
               }

               i = ++j;
            }
            else if (pop_style) {
               str.append(i, j - 1); // [i,j-1) excludes the tag "^/"

               if (!str.empty()) {
                  text.emplace_back(str, *style_i);
                  str.clear();
               }

               if (style_i == std::begin(style_stack)) {
                  std::string err_msg = {};
                  err_msg.append("Attempted to pop too many style tags from the stack, in the following tagged string:\n");
                  err_msg.append(tagged_str);

                  throw std::invalid_argument(err_msg);
               } else {
                  --style_i;
               }

               i = ++j;
            }
         }
      }
   }

   if (!str.empty()) {
      text.emplace_back(str, *style_i);
   }

   return text;
}
