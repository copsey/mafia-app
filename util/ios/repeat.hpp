#ifndef UTIL_IOS_REPEAT
#define UTIL_IOS_REPEAT

#include <cstddef>
#include <istream>
#include <ostream>
#include <string>
#include <type_traits>

namespace util {
	template <typename ObjT, typename InT = typename std::remove_const<ObjT>::type>
	// A simple wrapper type, used for repeating an I/O operation multiple times.
	struct repeat_t {
		// The type of value stored by this.
		using value_type = ObjT;
		// The type used for input operations.
		using in_type    = InT;
		// The type used for counting.
		using size_type  = std::size_t;
		
		// Wrap the default value_type in this, using a count of 0.
		// (So no values will be read during I/O.)
		constexpr repeat_t() = default;
		
		// Wrap v in this, using a count of n.
		constexpr repeat_t(const value_type& v, size_type n = 0):
			v {v},
			n {n}
		{ }
		
		// The value to use during I/O operations.
		constexpr const value_type& val() const {
			return v;
		}
		
		// The number of times to repeat the I/O operation.
		constexpr size_type count() const {
			return n;
		}
		
		// Make a wrapper for repeating I/O operations on this->val() a total of
		// this->count() + i times.
		constexpr repeat_t operator+ (size_type i) const {
			return repeat_t {v, n + i};
		}
		
	private:
		value_type v {};
		size_type  n {0};
	};
	
	template <typename ObjT, typename InT = typename std::remove_const<ObjT>::type>
	constexpr repeat_t<ObjT, InT> repeat(const ObjT& x, std::size_t n = 0) {
		return repeat_t<ObjT, InT>(x, n);
	}
	
	template <typename charT>
	// Specialisation of repeat_t for C-style strings.
	constexpr auto repeat(const charT* z, std::size_t n = 0)
		-> repeat_t<const charT*, std::basic_string<charT>>
	{
		return repeat_t<const charT*, std::basic_string<charT>>(z, n);
	}
	
	template <typename charT, typename ObjT, typename InT>
	// Read values of type InT from in a total of x.count() times, and check if
	// all are equal to x.val().
	// 
	// Input is performed using operator>> for InT.
	// The values inputted are discarded on return.
	// 
	// \post Set failbit on in if not all of the values inputted are equal
	//       to x.val().
	auto operator>> (std::basic_istream<charT>& in, const repeat_t<ObjT, InT>& x)
		-> std::basic_istream<charT>&
	{	
		InT y;
		
		for (std::size_t i = 0; i < x.count(); ++i) {
			if (!((in >> y) && y == x.val())) {
				in.setstate(std::ios::failbit);
				return in;
			}
		}
		
		return in;
	}
	
	template <typename charT, typename ObjT, typename InT>
	// Write x.val() to out a total of x.count() times.
	auto operator<< (std::basic_ostream<charT>& out, const repeat_t<ObjT, InT>& x)
		-> std::basic_ostream<charT>&
	{	
		for (std::size_t i = 0; i < x.count(); ++i) out << x.val();
		return out;
	}
}

#endif