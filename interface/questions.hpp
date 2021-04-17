#ifndef MAFIA_INTERFACE_QUESTIONS
#define MAFIA_INTERFACE_QUESTIONS

#include "../riketi/ref.hpp"

#include <ostream>

namespace maf {
	struct Console;

	
	// FIXME: Place all Questions into separate 'question' child namespace
	// of 'maf'.
	 
	struct Question {
		// Signifies that a question failed to process a set of commands.
		struct Bad_commands { };

		virtual ~Question() = default;

		// A string representing the screen.
		// Used when loading text from external files.
		virtual std::string_view id() const = 0;

		// Handles the given commands, taking action as appropriate.
		//
		// Returns true if the question has been fully answered, and false otherwise.
		// If false, then the tagged string outputted by write may have changed.
		//
		// Throws an exception if the commands couldn't be handled.
		virtual bool do_commands(const std::vector<std::string_view> & commands) = 0;

		// Write the screen to `output`.
		// This text should then be preprocessed.
		void write(std::ostream & output) const;

		// Configure the text parameters for this screen.
		// These are used to generate text output.
		//
		// By default, do nothing.
		virtual void set_params(TextParams & params) const { };
	};

	struct Confirm_end_game: Question {
		Confirm_end_game(Console & console)
		: _console_ref(console)
		{ }

		std::string_view id() const override { return "end-game"; }

		bool do_commands(const std::vector<std::string_view> & commands) override;

	private:
		rkt::ref<Console> _console_ref;
	};

	// TODO: Add screen for confirming if the daily lynch should be skipped.

	// TODO: Add screen for confirming if the mafia kill should be skipped.
}

#endif
