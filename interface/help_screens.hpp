#ifndef MAFIA_HELP_SCREENS_H
#define MAFIA_HELP_SCREENS_H

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

		Event const& event;

		string_view id() const final
		{ return event.id(); }
	};


	struct Role_Info_Screen: Help_Screen {
		Role_Info_Screen(const Role &role):
		_role_id{role.id()} { }

		string_view id() const final
		{ return alias(_role_id); }

		string_view txt_subdir() const override
		{ return "txt/help/roles/"; }

		void set_params(TextParams & params) const override;

	private:
		Role::ID _role_id;
	};


	struct List_Roles_Screen: Help_Screen {
		// Create a help screen listing the roles present in `rulebook`.
		//
		// It's possible to provide an optional alignment, in which case only
		// roles of that alignment will be listed.
		List_Roles_Screen(Rulebook const& rulebook,
						  optional<Alignment> alignment = std::nullopt)
		: _rulebook{rulebook}, _filter_alignment{alignment} { }

		string_view id() const final
		{ return "list-roles"; }

		void set_params(TextParams & params) const override;

	private:
		Rulebook const& _rulebook;
		optional<Alignment> _filter_alignment;

		static bool _compare_by_name(Role const& role_1, Role const& role_2);
		static TextParams _get_params(Role const& role);

		vector_of_refs<const Role> _get_roles() const;
	};


	struct Setup_Help_Screen: Help_Screen {
		string_view id() const final
		{ return "setup"; }
	};


	struct Player_Info_Screen: Help_Screen {
		Player_Info_Screen(const Player & player, const Game_log & game_log)
		: _player{player}, _game_log{game_log} { }

		string_view id() const final
		{ return "player-info"; }

		void set_params(TextParams & params) const override;

	private:
		const Player & _player;
		const Game_log & _game_log;
	};
}

#endif
