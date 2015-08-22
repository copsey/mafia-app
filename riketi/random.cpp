#include "random.hpp"

std::default_random_engine rkt::random_engine{std::random_device{}()};
