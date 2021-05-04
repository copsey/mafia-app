#ifndef MAFIA_UTIL_FSTREAM_H
#define MAFIA_UTIL_FSTREAM_H

#include <fstream>
#include <iterator>

#include "string.hpp"

namespace maf {
    using std::ifstream;
    using std::ofstream;
    using std::fstream;
}

namespace maf::util {
    // Read the entire content of `input` into a string.
    //
    // # Preconditions
    // - `input` is in a valid state.
    inline string read_all(ifstream & input) {
        string content;
        std::istreambuf_iterator<char> input_iter{input}, eos{};
		content.append(input_iter, eos);
        return content;
    }
}

#endif
