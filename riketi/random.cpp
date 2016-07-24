#include "random.hpp"

std::mt19937 rkt::random_engine{std::random_device{}()};
