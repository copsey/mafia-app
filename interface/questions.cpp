#include "command.hpp"
#include "console.hpp"
#include "questions.hpp"

bool maf::Confirm_end_game::do_commands(vector<string_view> const& commands) {
	if (commands_match(commands, {"yes"})) {
		_console.end_game();
		return true;
	} else if (commands_match(commands, {"no"})) {
		return true;
	} else {
		throw Bad_commands();
	}
}
