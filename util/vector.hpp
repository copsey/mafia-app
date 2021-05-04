#ifndef MAFIA_UTIL_VECTOR_H
#define MAFIA_UTIL_VECTOR_H

#include <functional>
#include <vector>

namespace maf {
	using std::vector;

	template <typename T>
	using vector_of_refs = vector<std::reference_wrapper<T>>;
}

#endif
