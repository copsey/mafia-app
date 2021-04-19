#include "command.hpp"
#include "console.hpp"
#include "questions.hpp"

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
