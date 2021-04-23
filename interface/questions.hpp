#ifndef MAFIA_INTERFACE_QUESTIONS
#define MAFIA_INTERFACE_QUESTIONS

#include "../util/ref.hpp"
#include "../util/stdlib.hpp"

#include "screen.hpp"

namespace maf {
	struct Console;
	
	// TODO: Place all Questions into separate 'question' child namespace of 'maf'.

	 
	struct Question: Screen {
		// Signifies that a question failed to process a set of commands.
		struct Bad_commands { };

		string_view txt_subdir() const override
		{ return "txt/questions/"; }

		// Handles the given commands, taking action as appropriate.
		//
		// Returns true if the question has been fully answered, and false otherwise.
		// If false, then the tagged string outputted by write may have changed.
		//
		// Throws an exception if the commands couldn't be handled.
		virtual bool do_commands(const vector<string_view> & commands) = 0;
	};


	struct Confirm_end_game: Question {
		Confirm_end_game(Console & console): _console_ref(console) { }

		string_view id() const override
		{ return "end-game"; }

		bool do_commands(const vector<string_view> & commands) override;

	private:
		util::ref<Console> _console_ref;
	};


	// TODO: Add screen for confirming if the daily lynch should be skipped.

	// TODO: Add screen for confirming if the mafia kill should be skipped.
}

#endif
