#ifndef MAFIA_HELP_SCREENS_H
#define MAFIA_HELP_SCREENS_H

#include "../util/optional.hpp"
#include "../util/vector.hpp"

#include "../core/core.hpp"

#include "format.hpp"
#include "game_log.hpp"
#include "game_screens.hpp"
#include "screen.hpp"

namespace maf {
	struct Help_Screen: Screen {
		Help_Screen(Console & console): Screen{console} { }

		fs::path txt_subdir() const override { return "txt/help"; }

		void do_commands(const CmdSequence & commands) override;
	};


	struct Game_help_screen: Help_Screen {
		Game_help_screen(Console & console, const Game_screen & screen):
		Help_Screen{console}, game_screen{screen} { }

		const Game_screen & game_screen;

		string_view id() const final { return game_screen.id(); }
	};


	struct Role_Info_Screen: Help_Screen {
		Role_Info_Screen(Console & console, const core::Role & role):
		Help_Screen{console}, _role_id{role.id()} { }

		string_view id() const final { return core::alias(_role_id); }

		fs::path txt_subdir() const override { return "txt/help/roles"; }

		void set_params(TextParams & params) const override;

	private:
		core::Role::ID _role_id;
	};


	struct List_Roles_Screen: Help_Screen {
		// Create a help screen listing the roles present in `rulebook`.
		//
		// It's possible to provide an optional alignment, in which case only
		// roles of that alignment will be listed.
		List_Roles_Screen(Console & console,
						  optional<core::Alignment> alignment = nullopt)
		: Help_Screen{console}, _filter_alignment{alignment} { }

		string_view id() const final { return "list-roles"; }

		void set_params(TextParams & params) const override;

	private:
		optional<core::Alignment> _filter_alignment;

		static bool _compare_by_name(const core::Role & role_1, const core::Role & role_2);
		static TextParams _get_params(const core::Role & role);

		vector_of_refs<const core::Role> _get_roles() const;
	};


	struct Setup_Help_Screen: Help_Screen {
		using Help_Screen::Help_Screen;

		string_view id() const final { return "setup"; }
	};


	struct Player_Info_Screen: Help_Screen {
		Player_Info_Screen(Console & console, const core::Player & player)
		: Help_Screen{console}, _player{player} { }

		string_view id() const final { return "player-info"; }

		void set_params(TextParams & params) const override;

	private:
		const core::Player & _player;
	};
}

#endif
