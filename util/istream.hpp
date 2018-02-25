#ifndef UTIL_ISTREAM
#define UTIL_ISTREAM

#include <istream>

namespace util {
	template <typename K, typename InObj = K>
	// Read an object from in and check if it is equal to k.
	//
	// Input is performed using operator>> for InObj.
	// The object inputted is discarded on return.
	//
	// \return in
	// \post Set failbit on in if the object inputted is not equal to k.
	std::istream& consume(std::istream& in, const K& k) {
		InObj x;
		if ((in >> x) && (x != k)) in.setstate(std::ios::failbit);
		return in;
	}
}

#endif