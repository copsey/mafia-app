#include <fstream>
#include <string>

#include "command.hpp"
#include "console.hpp"
#include "questions.hpp"

void maf::Question::write(std::ostream & output) const {
	// FIXME: This is horrendously fragile.
	std::string fname = "/Users/Jack/Documents/Developer/Projects/mafia/resources/txt/questions/";
	fname += this->id();
	fname += ".txt";

	auto input = std::ifstream{fname};
	if (input) {
		output << input.rdbuf();
	} else {
		output << "=Missing Screen=\n\nERROR: No text found for the \"";
		output << this->id();
		output << "\" question screen.\n\n%Enter @ok@ to return to the previous screen.";
	}
}

bool maf::Confirm_end_game::do_commands(std::vector<std::string_view> const& commands) {
	if (commands_match(commands, {"yes"})) {
		_console_ref->end_game();
		return true;
	} else if (commands_match(commands, {"no"})) {
		return true;
	} else {
		throw Bad_commands();
	}
}
