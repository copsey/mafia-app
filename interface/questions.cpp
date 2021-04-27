#include "command.hpp"
#include "console.hpp"
#include "questions.hpp"

void maf::Confirm_end_game::do_commands(const CmdSequence & commands) {
	if (commands_match(commands, {"yes"})) {
		console().end_game();
		console().dismiss_question();
	} else if (commands_match(commands, {"no"})) {
		console().dismiss_question();
	} else {
		Question::do_commands(commands);
	}
}
