#include <utility>

#include "../ios/istream.hpp"

#include "array.hpp"
#include "bool.hpp"
#include "data.hpp"
#include "int.hpp"
#include "null.hpp"
#include "object.hpp"
#include "string.hpp"

using util::istream_backup;

json::j_data::j_data(const j_data& other):
	_type {other._type}
{
	switch (_type) {
		case datatype::arr:
			_ptr = new j_array {*static_cast<j_array*>(other._ptr)};
			break;
		case datatype::b:
			_ptr = new j_bool {*static_cast<j_bool*>(other._ptr)};
			break;
		case datatype::i:
			_ptr = new j_int {*static_cast<j_int*>(other._ptr)};
			break;
		case datatype::null:
			_ptr = new j_null {*static_cast<j_null*>(other._ptr)};
			break;
		case datatype::obj:
			_ptr = new j_object {*static_cast<j_object*>(other._ptr)};
			break;
		case datatype::str:
			_ptr = new j_string {*static_cast<j_string*>(other._ptr)};
			break;
	}
}

json::j_data::j_data(j_data&& other):
	_type {other._type},
	_ptr  {other._ptr}
{
	other._type = datatype::null;
	other._ptr  = new j_null {};
}

json::j_data& json::j_data::operator= (const j_data& other) {
	_type = other._type;
	
	switch (_type) {
		case datatype::arr:
			_ptr = new j_array {*static_cast<j_array*>(other._ptr)};
			break;
		case datatype::b:
			_ptr = new j_bool {*static_cast<j_bool*>(other._ptr)};
			break;
		case datatype::i:
			_ptr = new j_int {*static_cast<j_int*>(other._ptr)};
			break;
		case datatype::null:
			_ptr = new j_null {*static_cast<j_null*>(other._ptr)};
			break;
		case datatype::obj:
			_ptr = new j_object {*static_cast<j_object*>(other._ptr)};
			break;
		case datatype::str:
			_ptr = new j_string {*static_cast<j_string*>(other._ptr)};
			break;
	}
	
	return *this;
}

json::j_data& json::j_data::operator= (j_data&& other) {
	free_ptr();
	
	_type = other._type;
	_ptr  = other._ptr;
	
	other._type = datatype::null;
	other._ptr  = new j_null {};
	
	return *this;
}

void json::j_data::free_ptr() {
	switch (_type) {
		case datatype::arr:
			delete static_cast<j_array*>(_ptr);
			break;
		case datatype::b:
			delete static_cast<j_bool*>(_ptr);
			break;
		case datatype::i:
			delete static_cast<j_int*>(_ptr);
			break;
		case datatype::null:
			delete static_cast<j_null*>(_ptr);
			break;
		case datatype::obj:
			delete static_cast<j_object*>(_ptr);
			break;
		case datatype::str:
			delete static_cast<j_string*>(_ptr);
			break;
	}
}

std::istream& json::read_data(std::istream& in, j_data& data) {
	// try to read a j_array
	{
		istream_backup backup{in};

		j_array* arr_ptr = new j_array {};
		if (read_array(in, *arr_ptr)) {
			data.free_ptr();
			data._ptr  = arr_ptr;
			data._type = j_data::datatype::arr;
			return in;
		}
		delete arr_ptr;
	}

	// try to read a j_bool
	{
		istream_backup backup{in};

		j_bool* b_ptr = new j_bool {};
		if (read_bool(in, *b_ptr)) {
			data.free_ptr();
			data._ptr  = b_ptr;
			data._type = j_data::datatype::b;
			return in;
		}
		delete b_ptr;
	}

	// try to read a j_int
	{
		istream_backup backup{in};

		j_int* i_ptr = new j_int {};
		if (read_int(in, *i_ptr)) {
			data.free_ptr();
			data._ptr  = i_ptr;
			data._type = j_data::datatype::i;
			return in;
		}
		delete i_ptr;
	}
	
	// try to read a j_null
	{
		istream_backup backup{in};

		j_null null;
		if (in >> null) {
			data.free_ptr();
			data._ptr  = nullptr;
			data._type = j_data::datatype::null;
			return in;
		}
	}

	// try to read a j_object
	{
		istream_backup backup{in};
	
		j_object* obj_ptr = new j_object {};
		if (read_object(in, *obj_ptr)) {
			data.free_ptr();
			data._ptr  = obj_ptr;
			data._type = j_data::datatype::obj;
			return in;
		}
		delete obj_ptr;
	}

	// try to read a j_string
	{
		istream_backup backup{in};

		j_string* str_ptr = new j_string {};
		if (read_string(in, *str_ptr)) {
			data.free_ptr();
			data._ptr  = str_ptr;
			data._type = j_data::datatype::str;
			return in;
		}
		delete str_ptr;
	}
	
	// failed to read any value, so input failed
	in.setstate(std::ios::failbit);
	return in;
}

std::ostream& json::write_data(std::ostream& out, const j_data& data) {
	switch (data._type) {
		case j_data::datatype::arr:
			write_array(out, *static_cast<j_array*>(data._ptr));
			break;
		case j_data::datatype::b:
			write_bool(out, *static_cast<j_bool*>(data._ptr));
			break;
		case j_data::datatype::i:
			write_int(out, *static_cast<j_int*>(data._ptr));
			break;
		case j_data::datatype::null:
			out << *static_cast<j_null*>(data._ptr);
			break;
		case j_data::datatype::obj:
			write_object(out, *static_cast<j_object*>(data._ptr));
			break;
		case j_data::datatype::str:
			write_string(out, *static_cast<j_string*>(data._ptr));
			break;
	}
	
	return out;
}

std::ostream& json::pretty_print_data(
	std::ostream& out,
	const j_data& data,
	util::repeat_t<const char*, std::string> indent)
{
	switch (data._type) {
		case j_data::datatype::arr:
			pretty_print_array(out, *static_cast<j_array*>(data._ptr), indent);
			break;
		case j_data::datatype::b:
			write_bool(out, *static_cast<j_bool*>(data._ptr));
			break;
		case j_data::datatype::i:
			write_int(out, *static_cast<j_int*>(data._ptr));
			break;
		case j_data::datatype::null:
			out << *static_cast<j_null*>(data._ptr);
			break;
		case j_data::datatype::obj:
			pretty_print_object(out, *static_cast<j_object*>(data._ptr), indent);
			break;
		case j_data::datatype::str:
			write_string(out, *static_cast<j_string*>(data._ptr));
			break;
	}
	
	return out;
}

auto json::operator<< (pretty_print_t<std::ostream> & out, const j_data & data)
	-> json::pretty_print_t<std::ostream> &
{
	switch (data._type) {
		case j_data::datatype::arr:   return out << *static_cast<j_array*>(data._ptr);
		case j_data::datatype::b:     return out << *static_cast<j_bool*>(data._ptr);
		case j_data::datatype::i:     return out << *static_cast<j_int*>(data._ptr);
		case j_data::datatype::null:  return out << *static_cast<j_null*>(data._ptr);
		case j_data::datatype::obj:   return out << *static_cast<j_object*>(data._ptr);
		case j_data::datatype::str:   return out << *static_cast<j_string*>(data._ptr);
	}
}