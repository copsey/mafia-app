/// Contains functions, classes, etc. from the C++ standard
/// library that are used across the mafia codebase.
/// 
/// Namespaces are stripped for readability, so take care
/// only to #include this header in implementation files

#ifndef MAFIA_STDLIB
#define MAFIA_STDLIB

#include <istream>
#include <map>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

// general types
using size = std::size_t;
using std::istream;
using std::map;
using std::ostream;
using std::pair;
using std::string;
using std::string_view;
using std::vector;

// exception types
using std::invalid_argument;
using std::out_of_range;

#endif
