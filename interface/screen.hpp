#ifndef MAFIA_SCREEN
#define MAFIA_SCREEN

#include <string>
#include <string_view>

#include "../util/stdlib.hpp"
#include "format.hpp"

namespace maf {
	// A single screen from the console. Intended to be used as a base class,
	// providing a common interface for all screens.
	struct Screen {
		virtual ~Screen() = default;

		// A string to identify the screen by. Used for example when loading
		// resources from the file system.
		virtual string_view id() const = 0;

		// A string indicating which subdirectory of "resources/" contains the
		// ".txt" file for this screen. Defaults to "txt/".
		virtual string_view txt_subdir() const { return "txt/"; }
		// A string with the complete path of the ".txt" file for this screen.
		string txt_path() const;
		// Open the ".txt" file for this screen and read its contents into a
		// string.
		string load_txt() const;

		// Fill `params` with this screen's text parameters. Does nothing by
		// default.
		virtual void set_params(TextParams & params) const { };

		// Apply `preprocess_text` to the contents of the ".txt" file for this
		// screen, and write the result to `output`.
		void write(string & output) const;
	};
}

#endif
