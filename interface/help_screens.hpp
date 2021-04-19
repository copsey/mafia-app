#ifndef MAFIA_HELP_SCREENS_H
#define MAFIA_HELP_SCREENS_H

#include <functional>
#include <optional>
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

		// A string indicating where in the file system the screen's contents
		// can be found. Defaults to "txt/help/".
		virtual std::string_view txt_loc() const { return "txt/help/"; }

		// Fill `params` with the text parameters for this help screen.
		// These are used to generate text output.
		//
		// By default, does nothing.
		virtual void set_params(TextParams & params) const { };

		// Write the help screen to `output`.
		// This text should then be preprocessed.
		void write(std::ostream & output) const;
	};


	struct Event_Help_Screen: Help_Screen {
		Event_Help_Screen(const Event &e): event{e} { }

		util::ref<const Event> event;

		std::string_view id() const override {
			return event->id();
		}
	};


	struct Role_Info_Screen: Help_Screen {
		Role_Info_Screen(const Role &role): role{role} { }

		util::ref<const Role> role;

		std::string_view id() const override {
			return alias(role->id());
		}

		// Override the text location to "txt/help/roles/".
		std::string_view txt_loc() const override { return "txt/help/roles/"; }

		void set_params(TextParams & params) const override;
	};


	struct List_Roles_Screen: Help_Screen {
		// Create a help screen listing the roles present in `rulebook`.
		//
		// It's possible to provide an optional alignment, in which case only
		// roles of that alignment will be listed.
		List_Roles_Screen(const Rulebook& rulebook, std::optional<Alignment> alignment = std::nullopt)
		: _rulebook{rulebook}, _filter_alignment{alignment}
		{ }

		std::string_view id() const override { return "list-roles"; }

		void set_params(TextParams & params) const override;

	private:
		util::ref<const Rulebook> _rulebook;
		std::optional<Alignment> _filter_alignment;
	};


	struct Setup_Help_Screen: Help_Screen {
		std::string_view id() const override { return "setup"; }
	};


	struct Player_Info_Screen: Help_Screen {
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
