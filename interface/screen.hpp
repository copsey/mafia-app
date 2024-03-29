#ifndef MAFIA_INTERFACE_SCREEN_H
#define MAFIA_INTERFACE_SCREEN_H

#include "../util/filesystem.hpp"
#include "../util/memory.hpp"
#include "../util/string.hpp"

#include "command.hpp"
#include "format.hpp"

namespace maf {
	struct Console;

	// A single screen from the console. Intended to be used as a base class,
	// providing a common interface for all screens.
	struct Screen {
		// Create a screen belonging to `console`.
		//
		// Note that only a pointer to `console` is stored. As such, the
		// lifetime of this screen must not outlive the console.
		Screen(Console & console) : _console{&console} {}

		virtual ~Screen() = default;

		// The console owning this screen.
		Console & console() const { return *_console; }

		// A string to identify the screen by. Used for example when loading
		// resources from the file system.
		virtual string_view id() const = 0;

		// A string indicating which subdirectory of "resources" contains the
		// ".txt" file for this screen. Defaults to "txt".
		virtual fs::path txt_subdir() const { return "txt"; }
		// The complete path of the ".txt" file for this screen.
		fs::path txt_path() const;
		// Open the ".txt" file for this screen and read its contents into a
		// string.
		string load_txt() const;

		// Fill `params` with this screen's text parameters. Does nothing by
		// default.
		virtual void set_params(TextParams & params) const {};

		// Apply `preprocess_text` to the contents of the ".txt" file for this
		// screen, and write the result to `output`.
		void write(string & output) const;

		// Attempt to apply the given commands to the console. Each screen
		// will handle commands differently.
		//
		// By default, try to show a help screen based on the given commands
		// and, if that fails, throw an exception.
		//
		// # Exceptions
		// Throws `Bad_commands` if the commands couldn't be handled.
		virtual void do_commands(const CmdSequence & commands);

		// An error thrown when a screen fails to handle a set of commands.
		struct Bad_commands {};

	private:
		not_null<Console *> _console;
	};
}

#endif
