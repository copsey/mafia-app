#include "../util/fstream.hpp"
#include "../util/misc.hpp"

#include "../core/core.hpp"

#include "command.hpp"
#include "console.hpp"
#include "screen.hpp"

namespace maf {
	fs::path Screen::txt_path() const {
		// FIXME: This is horrendously fragile.
		string_view root{"/Users/Jack/Documents/Developer/Projects/mafia"};

		fs::path path{root};
		path /= "resources";
		path /= this->txt_subdir();
		path /= this->id();
		path += ".txt";
		return path;
	}

	string Screen::load_txt() const {
		string contents;

		auto path = this->txt_path();

		if (ifstream input{path}; input) {
			contents = util::read_all(input);
		} else {
			contents += "=Error!=\n\nERROR: No text found for the `";
			contents += escaped(this->id());
			contents += "` screen.\n\nIt should be located at `";
			contents += escaped(path.native());
			contents += "`.\n\n$Enter `ok` to return to the previous screen.";
		}

		return contents;
	}

	void Screen::write(string & output) const {
		auto raw_txt = this->load_txt();

		TextParams params;
		this->set_params(params);

		try {
			output += preprocess_text(raw_txt, params);
		} catch (const preprocess_text_error & error) {
			output += "=Error!=\n\nERROR: ";
			output += escaped(error.message());
			output += " in the following string:\n\n`";
			output += escaped(error.input);
		}
	}

	void Screen::do_commands(const CmdSequence & commands) {
		if (commands.size() == 0) {
			string msg = "=Missing input!=\n\nEntering a blank input has no effect.\n(enter `help` if you're unsure what to do.)";
			auto params = TextParams{};
			throw Generic_error{move(msg), move(params)};
		} else if (commands_match(commands, {"help", "role", ""})) {
			try {
				const core::Role & role = console().active_rulebook().look_up(commands[2]);
				console().show_help_screen<Role_Info_Screen>(role);
			} catch (std::out_of_range) {
				throw core::Rulebook::Missing_role_alias{std::string(commands[2])};
			}
		} else if (commands_match(commands, {"list", "roles"})) {
			console().show_help_screen<List_Roles_Screen>();
		} else if (commands_match(commands, {"list", "roles", "village"})) {
			console().show_help_screen<List_Roles_Screen>(core::Alignment::village);
		} else if (commands_match(commands, {"list", "roles", "mafia"})) {
			console().show_help_screen<List_Roles_Screen>(core::Alignment::mafia);
		} else if (commands_match(commands, {"list", "roles", "freelance"})) {
			console().show_help_screen<List_Roles_Screen>(core::Alignment::freelance);
		} else if (commands_match(commands, {"info", ""})) {
			auto& player = console().game_log().find_player(commands[1]);
			console().show_help_screen<Player_Info_Screen>(player);
		} else {
			throw Bad_commands{};
		}
	}
}
