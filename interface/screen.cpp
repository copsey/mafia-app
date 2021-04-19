#include <fstream>
#include <iterator>

#include "screen.hpp"

std::string maf::Screen::txt_path() const {
	// FIXME: This is horrendously fragile.
	std::string fname = "/Users/Jack/Documents/Developer/Projects/mafia/";

	fname += "resources/";
	fname += this->txt_subdir();
	fname += this->id();
	fname += ".txt";
	return fname;
}

std::string maf::Screen::load_txt() const {
	std::string contents;

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

void maf::Screen::write(std::string & output) const {
	auto raw_txt = this->load_txt();

	TextParams params;
	this->set_params(params);

	try {
		output += preprocess_text(raw_txt, params);
	} catch (preprocess_text_error const& error) {
		output += "=Error!=\n\nERROR: ";
		error.write(output);
		output += " in the following string:\n\n@";
		output += escaped(error.input);
	}
}
