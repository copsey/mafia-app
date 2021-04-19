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
		virtual ~Help_Screen() = default;

		// A string representing the help screen.
		// Used when loading text from external files.
		virtual std::string_view id() const = 0;

		// Configure the text parameters for this help screen.
		// These are used to generate text output.
		//
		// By default, do nothing.
		virtual void set_params(TextParams & params) const { };

		// Write the help screen to `output`.
		// This text should then be preprocessed.
		void write(std::ostream & output) const;
	};


	struct Event_Help_Screen: Help_Screen {
		Event_Help_Screen(const Event &e)
		: event{e}
		{ }

		util::ref<const Event> event;

		std::string_view id() const override {
			return event->id();
		}
	};


	struct Role_Info_Screen: Help_Screen {
		Role_Info_Screen(const Role &role)
		: role{role}
		{ }

		util::ref<const Role> role;

		std::string_view id() const override {
			return alias(role->id());
		}

		void set_params(TextParams & params) const override;
	};


	struct List_Roles_Screen: Help_Screen {
		// An optional filter, specifying the alignment of roles that should be
		// displayed.
		enum class Filter_Alignment { all, village, mafia, freelance };

		// Create a help screen listing all of the roles present in `rulebook`.
		//
		// It is also possible to specify an optional alignment filter, in which case
		// only roles of that alignment will be listed.
		List_Roles_Screen(const Rulebook& rulebook, Filter_Alignment alignment = Filter_Alignment::all)
		: _rulebook{rulebook}, _filter_alignment{alignment}
		{ }

		std::string_view id() const override { return "list-roles"; }

		void set_params(TextParams & params) const override;

	private:
		util::ref<const Rulebook> _rulebook;
		Filter_Alignment _filter_alignment;
	};


	struct Setup_Help_Screen: Help_Screen {
		std::string_view id() const override { return "setup"; }
	};


	/// A screen presenting information on a given player.
	struct Player_Info_Screen: Help_Screen {
		/// Create an info screen for `player`, who should be a participant in the
		/// game managed by `game_log`.
		Player_Info_Screen(const Player & player, const Game_log & game_log)
		: _player_ref{player}, _game_log_ref{game_log}
		{ }

		std::string_view id() const override { return "player-info"; }

		void set_params(TextParams & params) const override;

	private:
		util::ref<const Player> _player_ref;
		util::ref<const Game_log> _game_log_ref;
	};
}

#endif
