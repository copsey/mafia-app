#ifndef MAFIA_HELP_SCREENS_H
#define MAFIA_HELP_SCREENS_H

#include <functional>
#include <ostream>

#include "../logic/logic.hpp"
#include "format.hpp"

#include "events.hpp"
#include "game_log.hpp"

namespace maf {
	struct Help_Screen {
		// Write a tagged string detailing the help screen to os.
		virtual void write(std::ostream &os, TextParams& params) const = 0;
	};


	struct Event_Help_Screen: Help_Screen {
		Event_Help_Screen(const Event &e)
			: event{e}
		{ }

		rkt::ref<const Event> event;

		void write(std::ostream &os, TextParams& params) const override;
	};


	struct Role_Info_Screen: Help_Screen {
		Role_Info_Screen(const Role &role)
			: role{role}
		{ }

		rkt::ref<const Role> role;

		void write(std::ostream &os, TextParams& params) const override;
	};


	struct List_Roles_Screen: Help_Screen {
		// An optional filter, specifying the alignment of roles that should be displayed.
		enum class Filter_Alignment { all, village, mafia, freelance };

		// Create a help screen listing all of the roles present in `rulebook`.
		//
		// It is also possible to specify an optional alignment filter, in which case
		// only roles of that alignment will be listed.
		List_Roles_Screen(const Rulebook& rulebook, Filter_Alignment alignment = Filter_Alignment::all)
			: _rulebook{rulebook}, _filter_alignment{alignment}
		{ }

		void write(std::ostream &os, TextParams& params) const override;

	private:
		rkt::ref<const Rulebook> _rulebook;
		Filter_Alignment _filter_alignment;
	};


	struct Setup_Help_Screen: Help_Screen {
		void write(std::ostream &os, TextParams& params) const override;
	};


	/// A screen presenting information on a given player.
	struct Player_Info_Screen: Help_Screen {
		/// Create an info screen for `player`, who should be a participant in the
		/// game managed by `game_log`.
		Player_Info_Screen(const Player & player, const Game_log & game_log)
			: _player_ref{player}, _game_log_ref{game_log}
		{ }

		void write(std::ostream& os, TextParams& params) const override;

	private:
		rkt::ref<const Player> _player_ref;
		rkt::ref<const Game_log> _game_log_ref;
	};
}

#endif
