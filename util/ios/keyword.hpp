#ifndef UTIL_IOS_KEYWORD
#define UTIL_IOS_KEYWORD

#include <locale>
#include <istream>
#include <ostream>
#include <string>

#include "istream.hpp"

namespace util {
	template <typename charT>
	// A string of alphanumeric characters.
	// 
	// Useful mainly for operator>>, which ensures that there is not an
	// alphanumeric character immediately following the keyword.
	struct keyword_t {
		// The type of characters in the string.
		using char_type = charT;
		
		// Make a keyword from the string of alphanumeric characters at z.
		// 
		// \pre z points to a C-style string.
		constexpr keyword_t(const char_type* z):
			z {z}
		{ }
		
		// Get the string of characters for this keyword.
		constexpr const char_type* val() const {
			return z;
		}
		
	private:
		const charT* z;
	};

	template <typename charT>
	constexpr keyword_t<charT> keyword(const charT* z) {
		return keyword_t<charT> {z};
	}
	
	template <typename charT>
	// Read the keyword from in.
	// 
	// This operation checks that there are no alphanumeric characters
	// immediately following the keyword. These are identified using std::isalnum
	// from the stream's locale.
	// 
	// \pre All of the characters in kw.val() are alphanumeric.
	// 
	// \post Set failbit on in if the keyword could not be read, or if there is
	//       an alphanumeric character immediately following the keyword.
	auto operator>> (std::basic_istream<charT>& in, const keyword_t<charT>& kw)
		-> std::basic_istream<charT>&
	{
		using sentry = typename std::basic_istream<charT>::sentry;
		
		sentry sen {in};
		if (!sen) return in;
		
		auto loc = in.getloc();

		if (consume(in, kw.val())) {
			auto c = in.peek();
			if (c != std::char_traits<charT>::eof() && std::isalnum(static_cast<charT>(c), loc)) {
				in.setstate(std::ios::failbit);
			}
		}

		return in;
	}
	
	template <typename charT>
	// Write the keyword to out.
	auto operator<< (std::basic_ostream<charT>& out, const keyword_t<charT>& kw)
		-> std::basic_ostream<charT>&
	{
		return out << kw.val();
	}
}

#endif