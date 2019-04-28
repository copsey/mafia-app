#include <algorithm>
#include <stdexcept>

#include "styled_string.hpp"

using std::invalid_argument;
using std::string;
using std::string_view;

string maf::escape_tags(string_view str)
{
   string esc_str = {};

   for (auto ch: str) {
      if (ch == '^' || ch == '{' || ch == '}') esc_str.push_back('^');
      esc_str.push_back(ch);
   }

   return esc_str;
}

bool is_param_name(string_view param)
{
   if (param.empty()) return false;
   
   for (auto ch: param) {
      switch (ch) {
         case 'a':
         case 'b':
         case 'c':
         case 'd':
         case 'e':
         case 'f':
         case 'g':
         case 'h':
         case 'i':
         case 'j':
         case 'k':
         case 'l':
         case 'm':
         case 'n':
         case 'o':
         case 'p':
         case 'q':
         case 'r':
         case 's':
         case 't':
         case 'u':
         case 'v':
         case 'w':
         case 'x':
         case 'y':
         case 'z':
         case 'A':
         case 'B':
         case 'C':
         case 'D':
         case 'E':
         case 'F':
         case 'G':
         case 'H':
         case 'I':
         case 'J':
         case 'K':
         case 'L':
         case 'M':
         case 'N':
         case 'O':
         case 'P':
         case 'Q':
         case 'R':
         case 'S':
         case 'T':
         case 'U':
         case 'V':
         case 'W':
         case 'X':
         case 'Y':
         case 'Z':
         case '0':
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
         case '8':
         case '9':
         case '_':
            break;
         
         default:
            return false;
      }
   }
   
   return true;
}

maf::Styled_text maf::styled_text_from(string_view paramed_str, TextParams const& params)
{
   string tagged_str = "";
   string str = "";
   Styled_text text = {};

   Styled_string::Style style_stack[8] = {Styled_string::Style::game};
   auto style_i = std::begin(style_stack);
   
   for (auto i = paramed_str.begin(), j = i, end = paramed_str.end(); i != end; ) {
      j = std::find(j, end, '{');
      tagged_str.append(i,j);
      
      if (j != end) {
         ++j;
         auto k = std::find(j, end, '}');
         
         if (k == end) {
            string err_msg = "Too many '{' chars in the following tagged string:\n";
            err_msg.append(paramed_str);
            
            throw invalid_argument(err_msg);
         }
         
         auto key = string(j,k);
         
         if (!is_param_name(key)) {
            string err_msg = "Invalid parameter name \"";
            err_msg.append(key);
            err_msg.append("\" in the following tagged string:\n");
            err_msg.append(paramed_str);
            
            throw invalid_argument(err_msg);
         }
         
         auto val_it = params.find(key);
         
         if (val_it == params.end()) {
            string err_msg = "Unrecognised parameter with name \"";
            err_msg.append(key);
            err_msg.append("\" in the following tagged string:\n");
            err_msg.append(paramed_str);
            
            throw invalid_argument(err_msg);
         }
         
         tagged_str.append(escape_tags(*val_it));
         
         j = k + 1;
      }
      
      i = j;
   }

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
            string err_msg = "There is a dangling '^' at the end of the following tagged string:\n";
            err_msg.append(paramed_str);

            throw invalid_argument(err_msg);
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
                  str += '^';
                  i = ++j;
                  break;
               }
               
               // replace "^{" with "{" in output
               case '{': {
                  str.append(i, j-1);
                  std += '{';
                  i = ++j;
                  break;
               }
               
               // replace "^}" with "}" in output
               case '}': {
                  str.append(i, j-1);
                  std += '}';
                  i = ++j;
                  break;
               }

               default: {
                  string err_msg = "The tag ^";
                  err_msg += ch;
                  err_msg.append(" is invalid, and appears in the following tagged string:\n");
                  err_msg.append(paramed_str);

                  throw invalid_argument(err_msg);
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
                     string err_msg = "Attempted to push too many style tags onto the stack, in the following tagged string:\n";
                     err_msg.append(paramed_str);

                     throw invalid_argument(err_msg);
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
                  string err_msg = "Attempted to pop too many style tags from the stack, in the following tagged string:\n";
                  err_msg.append(paramed_str);

                  throw invalid_argument(err_msg);
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
