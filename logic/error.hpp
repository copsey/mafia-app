#ifndef MAFIA_LOGIC_ERROR
#define MAFIA_LOGIC_ERROR

namespace maf {
	namespace error {
		struct action_invalid_for_player {};
		struct bad_timing {};
		struct game_has_ended {};
		struct identical_players {};
		struct inactive_player {};
		struct inactive_caster: inactive_player {};
		struct inactive_target: inactive_player {};
		struct inactive_voter: inactive_player {};
		struct missing_player {};
		struct missing_caster: missing_player {};
		struct missing_target: missing_player {};
		struct missing_voter: missing_player {};
		struct pending_lynch {};
		struct repeat_action {};
		struct unavailable_ability {};
		struct wrong_alignment {};
	}
}

#endif
