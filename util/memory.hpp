#ifndef MAFIA_UTIL_MEMORY_H
#define MAFIA_UTIL_MEMORY_H

#include <memory>

#include <gsl/pointers>

namespace maf {
	using std::make_unique;
	using std::unique_ptr;

	using gsl::not_null;
}

#endif
