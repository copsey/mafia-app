#include <fstream>
#include <iterator>

#include "../core/core.hpp"

#include "command.hpp"
#include "console.hpp"
#include "screen.hpp"

maf::string maf::Screen::txt_path() const {
	// FIXME: This is horrendously fragile.
	string fname = "/Users/Jack/Documents/Developer/Projects/mafia/";

	fname += "resources/";
	fname += this->txt_subdir();
	fname += this->id();
	fname += ".txt";
	return fname;
}

maf::string maf::Screen::load_txt() const {
	string contents;

	auto path = this->txt_path();
	std::ifstream input{path};

	if (input) {
		std::istreambuf_iterator<char> input_iter{input}, eos{};
		contents.append(input_iter, eos);
	} else {
		contents += "=Error!=\n\nERROR: No text found for the @";
		contents += escaped(this->id());
		contents += "@ screen.\n\nIt should be located at @";
		contents += escaped(path);
		contents += "@.\n\n%Enter @ok@ to return to the previous screen.";
	}

	return contents;
}

void maf::Screen::write(string & output) const {
	auto raw_txt = this->load_txt();

	TextParams params;
	this->set_params(params);

	try {
		output += preprocess_text(raw_txt, params);
	} catch (const preprocess_text_error & error) {
		output += "=Error!=\n\nERROR: ";
		output += escaped(error.message());
		output += " in the following string:\n\n@";
		output += escaped(error.input);
	}
}

void maf::Screen::do_commands(const CmdSequence & commands) {
	if (commands.size() == 0) {
		string msg = "=Missing input!=\n\nEntering a blank input has no effect.\n(enter @help@ if you're unsure what to do.)";
		auto params = TextParams{};
		throw Generic_error{move(msg), move(params)};
	} else if (commands_match(commands, {"help", "r", ""})) {
		try {
			const Role & role = console().active_rulebook().look_up(commands[2]);
			console().show_help_screen<Role_Info_Screen>(role);
		} catch (std::out_of_range) {
			throw Rulebook::Missing_role_alias{std::string(commands[2])};
		}
	} else if (commands_match(commands, {"list", "r"})) {
		console().show_help_screen<List_Roles_Screen>();
	} else if (commands_match(commands, {"list", "r", "v"})) {
		console().show_help_screen<List_Roles_Screen>(Alignment::village);
	} else if (commands_match(commands, {"list", "r", "m"})) {
		console().show_help_screen<List_Roles_Screen>(Alignment::mafia);
	} else if (commands_match(commands, {"list", "r", "f"})) {
		console().show_help_screen<List_Roles_Screen>(Alignment::freelance);
	} else if (commands_match(commands, {"info", ""})) {
		const Player & player = console().game_log().find_player(commands[1]);
		console().show_help_screen<Player_Info_Screen>(player);
	} else {
		throw Bad_commands{};
	}
}
