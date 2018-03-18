#ifndef UTIL_IOS_DISCARD
#define UTIL_IOS_DISCARD

#include <istream>
#include <ostream>
#include <type_traits>

namespace util {
	template <typename ObjT, typename InT = typename std::remove_const<ObjT>::type>
	// A simple wrapper type, used for reading and discarding a value
	// with operator<<.
	struct discard_t {
		// The type of object stored by this.
		using value_type = ObjT;
		// The type used for input operations.
		using in_type    = InT;
		
		// Wrap the default value_type in this.
		constexpr discard_t() = default;

		// Copy val into this.
		constexpr discard_t(const value_type& val):
			v {val}
		{ }
		
		// Wrap val in this.
		constexpr discard_t(value_type&& val):
			v {val}
		{ }
		
		// Get the value stored by this.
		constexpr const value_type& val() const {
			return v;
		}
		
	private:
		ObjT v {};
	};

	template <typename ObjT, typename InT = typename std::remove_const<ObjT>::type>
	constexpr discard_t<ObjT, InT> discard(const ObjT& val) {
		return discard_t<ObjT, InT> {val};
	}

	template <typename ObjT, typename InT = typename std::remove_const<ObjT>::type>
	constexpr discard_t<ObjT, InT> discard(ObjT&& val) {
		return discard_t<ObjT, InT> {std::move(val)};
	}

	template <typename charT>
	// Specialisation of discard_t for C-style strings.
	constexpr auto discard(const charT* z)
		-> discard_t<const charT*, std::basic_string<charT>>
	{
		return discard_t<const charT*, std::basic_string<charT>> {z};
	}
	
	template <typename charT, typename ObjT, typename InT>
	// Read a value from in and check if it's equal to x.val().
	// 
	// Input is performed using operator>> for InT.
	// The value inputted is discarded on return.
	// 
	// \post Set failbit on in if the value inputted is not equal to x.val().
	auto operator>> (std::basic_istream<charT>& in, const discard_t<ObjT, InT>& x)
		-> std::basic_istream<charT>&
	{
		InT y;
		if ((in >> y) && (y != x.val())) in.setstate(std::ios::failbit);
		return in;
	}
	
	template <typename charT, typename ObjT, typename InT>
	// Write the value wrapped by x to out.
	auto operator<< (std::basic_ostream<charT>& out, const discard_t<ObjT, InT>& x)
		-> std::basic_ostream<charT>&
	{
		return out << x.val();
	}
}

#endif