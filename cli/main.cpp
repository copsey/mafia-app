#include "../util/iostream.hpp"
#include "../util/string.hpp"

#include "../interface/console.hpp"

namespace maf {
	void print(const StyledText & text, ostream & out) {
		// using style_option     = StyledString::attributes_t::style_option;
		// using weight_option    = StyledString::attributes_t::weight_option;
		using typeface_option  = StyledString::attributes_t::typeface_option;
		using semantics_option = StyledString::attributes_t::semantics_option;

		out << "\n";

		for (auto&& [str, attributes]: text) {
			if (attributes.semantics == semantics_option::title) {
				out << " " << str << " \n";
				out << string(str.size() + 2, '=');
			} else if (attributes.typeface == typeface_option::monospace) {
				out << '\'' << str << '\'';
			} else {
				out << str;
			}
		}

		out << "\n\n";
	}

	void print_output(const Console & console) {
		print(console.output(), std::cout);
	}

	void print_error_message(const Console & console) {
		print(console.error_message(), std::cerr);
	}
}

int main() {
	using namespace maf;

	std::ios_base::sync_with_stdio(false);

	Console console{};
	print_output(console);

	for (bool quit = false; !quit; ) {
		std::cout << ">> ";

		string input;
		if (!getline(std::cin, input)) break;

		if (input == "quit" || input == "exit") {
			quit = true;
		} else if (console.input(input)) {
			print_output(console);
		} else {
			print_error_message(console);
		}
	}
}
