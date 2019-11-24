#include "command.hpp"
#include "../common/stdlib.h"

vector<string_view> maf::parse_input(string_view str) {
	vector<string_view> v = {};

	{
		      auto i   = str.data();
		      auto j   = i;
		const auto end = str.data() + str.size();

		for ( ; j != end; ) {
			if (*j == ' ' || *j == '\t') {
				if (i != j) v.emplace_back(i, j - i);
				i = ++j;
			} else {
				++j;
			}
		}

		if (i != j) v.emplace_back(i, j - i);
	}

	return v;
}
