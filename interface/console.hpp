#ifndef MAFIA_INTERFACE_CONSOLE_H
#define MAFIA_INTERFACE_CONSOLE_H

#include "../util/memory.hpp"
#include "../util/misc.hpp"

#include "command.hpp"
#include "format.hpp"
#include "game_log.hpp"
#include "help_screens.hpp"
#include "questions.hpp"
#include "setup_screen.hpp"

namespace maf {
	// Signifies that no preset is defined with index i.
	struct Missing_preset {
		int index;
	};

	// Signifies that there is no game in progress at the moment.
	struct No_game_in_progress { };

	// Signifies that a new game cannot begin right now.
	struct Begin_game_failed {
		enum class Reason {
			game_already_in_progress
		};

		Reason reason;
	};

	struct Generic_error {
		string msg;
		TextParams params;
	};


	struct Console {
		// Creates a new console, displaying some initial output.
		Console();

		// Processes the given commands.
		// Returns true if the commands were accepted. In this case,
		// error_message() is empty.
		// Returns false if the commands are invalid for some reason. In this
		// case, output() remains unchanged and error_message() is non-empty.
		bool do_commands(const CmdSequence & commands);
		// Process the given input string, by seperating it into commands
		// delimited by whitespace. For example, the input "add p Brutus" becomes
		// {"add", "p", "Brutus"}.
		// Returns true if the input was accepted. In this case, error_message()
		// is empty.
		// Returns false if the input is invalid for some reason. In this case,
		// output() remains unchanged and error_message() is non-empty.
		bool input(string_view input);

		// The most recent output. Never empty.
		const StyledText & output() const;
		// Format the given contents, updating the output to display the styled
		// text obtained.
		void read_output(string_view contents);
		// Updates the output to display the appropriate screen.
		void refresh_output();

		// The most recent error message. Usually empty.
		// The only styles that will ever appear here are help, help_title and
		// command.
		const StyledText & error_message() const;
		// Reads the tagged string `str`, updating the error message to
		// display the styled text obtained.
		void read_error_message(string_view str, TextParams const& params = {});
		// Removes the current error message.
		void clear_error_message();

		// Get the screen currently being displayed.
		Screen & active_screen();
		const Screen & active_screen() const;

		// Get the help screen currently being stored, or `nullptr` if none
		// exists.
		const Help_Screen * help_screen() const { return _help_screen.get(); }
		// Check if a help screen is currently being stored.
		bool has_help_screen() const { return _help_screen != nullptr; }
		// Create a new help screen, replacing any that is already being
		// stored.
		template <typename ScreenType, typename... Args>
		void show_help_screen(Args&&... args) {
			_help_screen = make_unique<ScreenType>(*this, args...);
		}
		// Delete the help screen, if one is being stored. Otherwise has no
		// effect.
		void dismiss_help_screen() { _help_screen.reset(); }

		// Gets the question currently being stored, or nullptr if none exists.
		const Question * question() const { return _question.get(); }
		// Checks if a question is currently being stored.
		bool has_question() const { return _question != nullptr; }
		// Create a new question, replacing any that is already being stored.
		template <typename ScreenType, typename... Args>
		void show_question(Args&&... args) {
			_question = make_unique<ScreenType>(*this, args...);
		}
		// Delete the current question, if one is being stored. Otherwise has
		// no effect.
		void dismiss_question() { _question.reset(); }

		// The game log managing the current game in progress.
		// Throws an exception if no game is in progress.
		Game_log & game_log() {
			if (!has_game()) throw No_game_in_progress{};
			return *_game_log;
		}
		// The game log managing the current game in progress.
		// Throws an exception if no game is in progress.
		const Game_log & game_log() const {
			if (!has_game()) throw No_game_in_progress{};
			return *_game_log;
		}
		// Checks if a game is currently in progress.
		bool has_game() const { return _game_log != nullptr; }

		void store_game(unique_ptr<Game_log> game_log) {
			_game_log = move(game_log);
		}

		// Instantly ends any game in progress, and saves a transcript of it.
		// (the game may not have actually reached an end state.)
		void end_game();

		// The rulebook currently being used: either the current game's rulebook
		// if a game is in progress, or the setup screen's rulebook.
		const core::Rulebook & active_rulebook() const;

	private:
		StyledText _output{};
		StyledText _error_message{};

		Setup_screen _setup_screen;
		unique_ptr<Game_log> _game_log{};
		unique_ptr<Help_Screen> _help_screen{};
		unique_ptr<Question> _question{};
	};
}

#endif
