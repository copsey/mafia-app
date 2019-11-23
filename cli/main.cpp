#include <iostream>
#include <string>

#include "../interface/console.hpp"

using std::string;


void print_output(const maf::Console & console) {
   for (auto & styled_string: console.output()) {
      switch(styled_string.style) {
         case maf::Styled_string::Style::title:
            // ignore titles
            break;
         case maf::Styled_string::Style::command:
            std::cout << '\'' << styled_string.string << '\'';
            break;
         default:
            std::cout << styled_string.string;
            break;
      }
   }

   std::cout << "\n\n";
}

void print_error_message(const maf::Console & console) {
   for (auto & styled_string: console.error_message()) {
      switch(styled_string.style) {
         case maf::Styled_string::Style::title:
            // ignore titles
            break;
         case maf::Styled_string::Style::command:
            std::cerr << '\'' << styled_string.string << '\'';
            break;
         default:
            std::cerr << styled_string.string;
            break;
      }
   }

   std::cerr << "\n\n";
}

int main() {
   maf::Console console{};
   print_output(console);

   for (bool quit = false; !quit; ) {
      std::cout << ">> ";

      string input;
      std::getline(std::cin, input);

      std::cout << '\n';

      if (input == "quit" || input == "exit") {
         quit = true;
      } else if (console.input(input)) {
         print_output(console);
      } else {
         print_error_message(console);
      }
   }
}
