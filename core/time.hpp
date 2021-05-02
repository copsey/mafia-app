#ifndef MAFIA_LOGIC_TIME
#define MAFIA_LOGIC_TIME

namespace maf {
	/// An integer representing the number of in-game days which have passed.
	using Date = unsigned;

	/// A period of in-game time.
	enum class Time { day, night };
}

#endif