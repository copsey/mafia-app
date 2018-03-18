#ifndef UTIL_ISTREAM
#define UTIL_ISTREAM

#include <istream>

namespace util {
	template <typename charT>
	// Attempt to read a C-style string from in, discarding all characters.
	// 
	// \return in
	// \post Set failbit on in if the character sequence inputted is not equal to z.
	std::basic_istream<charT>& consume(std::basic_istream<charT>& in, const charT* z) {
		for (const charT* p = z; (*p) != charT('\0'); ++p) {
			charT c1 = *p;
			charT c2;
			
			if (!(in.get(c2) && c1 == c2)) {
				in.setstate(std::ios::failbit);
				return in;
			}
		}
		
		return in;
	}
	
	template <typename InFunc, typename... Args>
	// Evaluate f on the parameter pack (in, args...).
	// If (!in) after evaluation, reset in to its original state when this
	// function was called.
	// 
	// \return true if the input operation succeeded; false otherwise.
	// \note f(in, args...) will be evaluated even if (!in) when this function
	//       was called.
	// 
	// \see basic_istream_backup for a resource-based approach.
	bool try_input(std::istream& in, InFunc f, Args... args) {
		auto state = in.rdstate();
		auto pos   = in.tellg();
		
		f(in, args...);
		
		if (!in) {
			in.seekg(pos);
			in.clear(state);
			return false;
		}
		
		return true;
	}
	
	template <typename charT>
	// A resource preserving the state of a basic_istream in case of failure
	// during I/O operations.
	// 
	// Stores a pointer to a basic_istream on construction. If failbit or
	// badbit have been set when this resouce is destroyed, restore the stream's
	// iostate and position to their original values.
	// 
	// \see try_input for a function-based approach.
	struct basic_istream_backup {
		// The type of stream being managed.
		using stream_type = std::basic_istream<charT>;
		// The type of characters in the stream.
		using char_type   = charT;
		
		basic_istream_backup(std::basic_istream<charT>& in):
			in_p  {&in},
			state {in.rdstate()},
			pos   {in.tellg()}
		{ }
		
		~basic_istream_backup() {
			if (in_p->fail()) {
				in_p->clear(state);
				in_p->seekg(pos);
			}
		}
		
		// no copy or move constructors
		basic_istream_backup(const basic_istream_backup&) = delete;
		basic_istream_backup(basic_istream_backup&&) = delete;
	
	private:
		stream_type *                  in_p;
		typename stream_type::iostate  state;
		typename stream_type::pos_type pos;
	};
	
	// specialisations for istream and wistream
	using istream_backup  = basic_istream_backup<char>;
	using wistream_backup = basic_istream_backup<wchar_t>;
}

#endif