#ifndef MAFIA_SCREEN
#define MAFIA_SCREEN

#include <string>
#include <string_view>

#include "format.hpp"

namespace maf {
	// A single screen from the console. Intended to be used as a base class,
	// providing a common interface for all screens.
	struct Screen {
		virtual ~Screen() = default;

		// A string to identify the screen by. Used for example when loading
		// resources from the file system.
		virtual std::string_view id() const = 0;

		// A string indicating which subdirectory of "resources/" contains the
		// ".txt" file for this screen. Defaults to "txt/".
		virtual std::string_view txt_subdir() const { return "txt/"; }

		// A string consisting of the complete path of the ".txt" file for this
		// screen.
		std::string txt_path() const;

		// Open the file at `this->txt_paths()` and return its contents as a
		// string.
		std::string load_txt() const;

		// Fill `params` with this screen's text parameters. Does nothing by
		// default.
		virtual void set_params(TextParams & params) const { };

		// Apply `preprocess_text` to the contents of the ".txt" file for this
		// screen, and write the result to `output`.
		void write(std::string & output) const;
	};
}

#endif
