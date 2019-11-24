#include "../common/stdlib.h"
#include "command.hpp"
#include "console.hpp"
#include "questions.hpp"


bool maf::Confirm_end_game::do_commands(const vector<string_view> & commands) {
	if (commands_match(commands, {"yes"})) {
		_console_ref->end_game();
		return true;
	}
	else if (commands_match(commands, {"no"})) {
		return true;
	}
	else {
		throw Bad_commands();
	}
}

void maf::Confirm_end_game::write(ostream &os) const {
	os << "^TEnd Game?^/You are about to end the current game.\nAre you sure that you want to do this? (^cyes^/ or ^cno^/)";
}

// bool maf::Confirm_mafia_kill_skip::do_commands(const vector<string_view> &commands, Console &console) {
// 	if (commands_match(commands, {"yes"})) {
// 		console.g
// 		return true;
// 	}
// 	else if (commands_match(commands, {"no"})) {
// 		return true;
// 	}
// 	else {
// 		throw Bad_commands();
// 	}
// }

// void maf::Confirm_mafia_kill_skip::write(ostream &os) const {
// 	os << "^TSkip?^/You are about to skip the mafia's nightly kill.\nAre you sure that you want to do this? (^cyes^/ or ^cno^/)";
// }
