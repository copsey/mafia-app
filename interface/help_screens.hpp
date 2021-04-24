#ifndef MAFIA_HELP_SCREENS_H
#define MAFIA_HELP_SCREENS_H

#include <functional>
#include <optional>
#include <ostream>

#include "../util/stdlib.hpp"
#include "../logic/logic.hpp"
#include "format.hpp"

#include "events.hpp"
#include "game_log.hpp"
#include "screen.hpp"

namespace maf {
	struct Help_Screen: Screen {
		string_view txt_subdir() const override
		{ return "txt/help/"; }
	};


	struct Event_Help_Screen: Help_Screen {
		Event_Help_Screen(const Event &e): event{e} { }

		util::ref<const Event> event;

		string_view id() const final
		{ return event->id(); }
	};


	struct Role_Info_Screen: Help_Screen {
		Role_Info_Screen(const Role &role): role{role} { }

		util::ref<const Role> role;

		string_view id() const final
		{ return alias(role->id()); }

		string_view txt_subdir() const override
		{ return "txt/help/roles/"; }

		void set_params(TextParams & params) const override;
	};


	struct List_Roles_Screen: Help_Screen {
		// Create a help screen listing the roles present in `rulebook`.
		//
		// It's possible to provide an optional alignment, in which case only
		// roles of that alignment will be listed.
		List_Roles_Screen(const Rulebook& rulebook, optional<Alignment> alignment = std::nullopt)
		: _rulebook{rulebook}, _filter_alignment{alignment} { }

		string_view id() const final
		{ return "list-roles"; }

		void set_params(TextParams & params) const override;

	private:
		util::ref<const Rulebook> _rulebook;
		optional<Alignment> _filter_alignment;

		static bool _compare_by_name(Role const& role_1, Role const& role_2);
		static TextParams _get_params(Role const& role);
	};


	struct Setup_Help_Screen: Help_Screen {
		string_view id() const final
		{ return "setup"; }
	};


	struct Player_Info_Screen: Help_Screen {
		Player_Info_Screen(const Player & player, const Game_log & game_log)
		: _player_ref{player}, _game_log_ref{game_log}
		{ }

		string_view id() const final
		{ return "player-info"; }

		void set_params(TextParams & params) const override;

	private:
		util::ref<const Player> _player_ref;
		util::ref<const Game_log> _game_log_ref;
	};
}

#endif
