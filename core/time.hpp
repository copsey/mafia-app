#ifndef MAFIA_CORE_TIME_H
#define MAFIA_CORE_TIME_H

namespace maf::core {
	/// An integer representing the number of in-game days which have passed.
	using Date = unsigned;

	/// A period of in-game time.
	enum class Time { day, night };
}

#endif