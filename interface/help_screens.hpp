#ifndef MAFIA_HELP_SCREENS_H
#define MAFIA_HELP_SCREENS_H

#include <optional>
#include <ostream>

#include "../util/stdlib.hpp"
#include "../logic/logic.hpp"
#include "format.hpp"

#include "game_log.hpp"
#include "game_screens.hpp"
#include "screen.hpp"

namespace maf {
	struct Help_Screen: Screen {
		Help_Screen(Console & console): Screen{console} { }

		string_view txt_subdir() const override { return "txt/help/"; }

		void do_commands(const CmdSequence & commands) override;
	};


	struct Game_help_screen: Help_Screen {
		Game_help_screen(Console & console, const Game_screen & screen):
		Help_Screen{console}, game_screen{screen} { }

		const Game_screen & game_screen;

		string_view id() const final { return game_screen.id(); }
	};


	struct Role_Info_Screen: Help_Screen {
		Role_Info_Screen(Console & console, const Role &role):
		Help_Screen{console}, _role_id{role.id()} { }

		string_view id() const final { return alias(_role_id); }

		string_view txt_subdir() const override { return "txt/help/roles/"; }

		void set_params(TextParams & params) const override;

	private:
		Role::ID _role_id;
	};


	struct List_Roles_Screen: Help_Screen {
		// Create a help screen listing the roles present in `rulebook`.
		//
		// It's possible to provide an optional alignment, in which case only
		// roles of that alignment will be listed.
		List_Roles_Screen(Console & console,
						  optional<Alignment> alignment = std::nullopt)
		: Help_Screen{console}, _filter_alignment{alignment} { }

		string_view id() const final { return "list-roles"; }

		void set_params(TextParams & params) const override;

	private:
		optional<Alignment> _filter_alignment;

		static bool _compare_by_name(Role const& role_1, Role const& role_2);
		static TextParams _get_params(Role const& role);

		vector_of_refs<const Role> _get_roles() const;
	};


	struct Setup_Help_Screen: Help_Screen {
		using Help_Screen::Help_Screen;

		string_view id() const final { return "setup"; }
	};


	struct Player_Info_Screen: Help_Screen {
		Player_Info_Screen(Console & console, const Player & player)
		: Help_Screen{console}, _player{player} { }

		string_view id() const final { return "player-info"; }

		void set_params(TextParams & params) const override;

	private:
		const Player & _player;
	};
}

#endif
