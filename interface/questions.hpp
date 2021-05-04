#ifndef MAFIA_INTERFACE_QUESTIONS
#define MAFIA_INTERFACE_QUESTIONS

#include "screen.hpp"

namespace maf {
	// TODO: Place all Questions into separate 'question' child namespace of 'maf'.


	struct Question: Screen {
		using Screen::Screen;

		string_view txt_subdir() const override { return "txt/questions/"; }
	};


	struct Confirm_end_game: Question {
		using Question::Question;

		string_view id() const override { return "end-game"; }

		void do_commands(const CmdSequence & commands) override;
	};


	// TODO: Add screen for confirming if the daily lynch should be skipped.

	// TODO: Add screen for confirming if the mafia kill should be skipped.
}

#endif
