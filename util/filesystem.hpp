#ifndef MAFIA_UTIL_FILESYSTEM
#define MAFIA_UTIL_FILESYSTEM

#include <filesystem>

namespace maf {
	namespace fs = std::filesystem;

	namespace application {
		/// Path containing the application and all its resources
		/// (e.g. '.txt' files).
		inline auto root_dir() -> fs::path {
			// TODO: Set this at run-time instead of compile-time.
			return APPLICATION_ROOT_DIR;
		}
	}
}

#endif
