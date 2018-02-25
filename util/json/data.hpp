#ifndef JSON_DATA
#define JSON_DATA

#include <istream>

namespace json {
	struct j_data {
		// Create some null data.
		constexpr j_data() = default;
		
		j_data(const j_data& other);
		j_data(j_data&& other);
		
		~j_data() { free_ptr(); }
		
		j_data& operator=(const j_data& other);
		j_data& operator=(j_data&& other);
		
	private:
		enum class datatype { arr, b, i, null, obj, str };
		
		void*    _ptr  {nullptr};
		datatype _type {datatype::null};
		
		void free_ptr();
		
		friend std::istream& read_data(std::istream&, j_data&);
		friend std::ostream& write_data(std::ostream&, const j_data&);
		friend std::ostream& pretty_print_data(std::ostream&, const j_data&, int);
	};
	
	std::istream& read_data(std::istream& in, j_data& data);
	
	std::ostream& write_data(std::ostream& out, const j_data& data);
	
	std::ostream& pretty_print_data(std::ostream& out, const j_data& data, int indent_level = 0);
}

#endif