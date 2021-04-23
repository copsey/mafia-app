#include <fstream>
#include <iterator>

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
	} catch (preprocess_text_error const& error) {
		string msg;
		error.write(msg);

		output += "=Error!=\n\nERROR: ";
		output += escaped(msg);
		output += " in the following string:\n\n@";
		output += escaped(error.input);
	}
}
