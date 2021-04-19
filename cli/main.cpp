#include <iostream>
#include <string>

#include "../interface/console.hpp"

using style_option = maf::StyledString::attributes_t::style_option;
using weight_option = maf::StyledString::attributes_t::weight_option;
using typeface_option = maf::StyledString::attributes_t::typeface_option;
using semantics_option = maf::StyledString::attributes_t::semantics_option;

void print(maf::StyledText const& text, std::ostream & out) {
   out << "\n";

   for (auto&& [string, attributes]: text) {
      if (attributes.semantics == semantics_option::title) {
         out << " " << string << "\n";
         out << std::string(string.size() + 2, '=');
      } else if (attributes.typeface == typeface_option::monospace) {
         out << '\'' << string << '\'';
      } else {
         out << string;
      }
   }

   out << "\n\n";
}

void print_output(const maf::Console & console) {
   print(console.output(), std::cout);
}

void print_error_message(const maf::Console & console) {
   print(console.error_message(), std::cerr);
}

int main() {
   maf::Console console{};
   print_output(console);

   for (bool quit = false; !quit; ) {
      std::cout << ">> ";

      std::string input;
      std::getline(std::cin, input);

      if (input == "quit" || input == "exit") {
         quit = true;
      } else if (console.input(input)) {
         print_output(console);
      } else {
         print_error_message(console);
      }
   }
}
