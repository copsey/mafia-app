#ifndef MAFIA_CONSOLE_H
#define MAFIA_CONSOLE_H

#include <array>
#include <memory>
#include <string_view>
#include <utility>

#include "../util/algorithm.hpp"

#include "game_log.hpp"
#include "help_screens.hpp"
#include "questions.hpp"
#include "setup_screen.hpp"
#include "format.hpp"

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


	struct Console {
		// Creates a new console, displaying some initial output.
		Console();

		// Processes the given commands.
		// Returns true if the commands were accepted. In this case,
		// error_message() is empty.
		// Returns false if the commands are invalid for some reason. In this
		// case, output() remains unchanged and error_message() is non-empty.
		bool do_commands(const std::vector<std::string_view> & commands);
		// Process the given input string, by seperating it into commands
		// delimited by whitespace. For example, the input "add p Brutus" becomes
		// {"add", "p", "Brutus"}.
		// Returns true if the input was accepted. In this case, error_message()
		// is empty.
		// Returns false if the input is invalid for some reason. In this case,
		// output() remains unchanged and error_message() is non-empty.
		bool input(std::string_view input);

		// The most recent output. Never empty.
		const StyledText & output() const;
		// Read the tagged string `str`, updating the output to display
		// the styled text obtained.
		void read_output(std::string_view str, TextParams const& params);
		// Updates the output to display the appropriate screen.
		void refresh_output();

		// The most recent error message. Usually empty.
		// The only styles that will ever appear here are help, help_title and
		// command.
		const StyledText & error_message() const;
		// Reads the tagged string `str`, updating the error message to
		// display the styled text obtained.
		void read_error_message(std::string_view str, TextParams const& params = {});
		// Removes the current error message.
		void clear_error_message();

		// Gets the help screen currently being stored, or nullptr if none exists.
		const Help_Screen * help_screen() const;
		// Checks if a help screen is currently being stored.
		bool has_help_screen() const;
		// Stores hs, deleting any help screen that is already being stored.
		// Automatically assumes ownership of hs.
		void store_help_screen(Help_Screen *hs);
		// Deletes the current help screen, if one is being stored.
		void clear_help_screen();

		// Gets the question currently being stored, or nullptr if none exists.
		const Question * question() const;
		// Checks if a question is currently being stored.
		bool has_question() const;
		// Stores q, deleting any question that is already being stored.
		// Automatically assumes ownership of q.
		void store_question(Question *q);
		// Deletes the current question, if one is being stored.
		void clear_question();

		// The current game in progress.
		// Throws an exception if no game is in progress.
		const Game & game() const;
		// The game log managing the current game in progress.
		// Throws an exception if no game is in progress.
		const Game_log & game_log() const;
		// Checks if a game is currently in progress.
		bool has_game() const;

		// Instantly ends any game in progress, and saves a transcript of it.
		// (the game may not have actually reached an end state.)
		void end_game();

		// The rulebook currently being used: either the current game's rulebook
		// if a game is in progress, or the setup screen's rulebook.
		const Rulebook & active_rulebook() const;

	private:
		struct Game_parameters {
			std::vector<std::string> player_names;
			std::vector<Role::ID> role_ids;
			std::vector<Wildcard::ID> wildcard_ids;
			Rulebook rulebook;
		};

		static constexpr std::size_t num_presets{3};
		static const std::array<Game_parameters, num_presets> _presets;

		StyledText _output{};
		StyledText _error_message{};

		std::unique_ptr<Game_log> _game_log{};
		Setup_screen _setup_screen{};
		std::unique_ptr<Help_Screen> _help_screen{};
		std::unique_ptr<Question> _question{};

		void begin_game(const std::vector<std::string> &player_names,
		                const std::vector<Role::ID> &role_ids,
		                const std::vector<Wildcard::ID> &wildcard_ids,
		                const Rulebook &rulebook);
		void begin_pending_game();
		void begin_preset(int i);
	};
}

#endif
