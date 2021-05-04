#ifndef MAFIA_SETUP_SCREEN_H
#define MAFIA_SETUP_SCREEN_H

#include <map>
#include <set>

#include "../util/memory.hpp"
#include "../util/string.hpp"
#include "../util/vector.hpp"

#include "../core/core.hpp"

#include "game_log.hpp"
#include "names.hpp"
#include "screen.hpp"

namespace maf {
	struct Setup_screen: Screen {
		// Signifies that the given name is invalid for a player.
		struct Bad_player_name {
			string name;
		};

		// Signifies that a player with the given name already exists.
		struct Player_already_exists {
			string name;
		};

		// Signifies that no player with the given name exists.
		struct Player_missing {
			string name;
		};

		// Signifies that no copies of the given rolecard have been chosen.
		struct Rolecard_unselected {
			Rolecard_unselected(const core::Role & r): role{r} { }

			const core::Role & role;
		};

		// Signifies that no copies of the given wildcard have been chosen.
		struct Wildcard_unselected {
			Wildcard_unselected(const core::Wildcard & w): wildcard{w} { }

			const core::Wildcard & wildcard;
		};

		// Signifies that a set of commands couldn't be interpreted.
		struct Bad_commands { };

		using Screen::Screen;

		string_view id() const final { return "setup"; }

		// The rulebook to be used in the pending game.
		const core::Rulebook & rulebook() const;
		// The names of the currently selected players.
		vector<string> player_names() const;
		// The IDs of the currently selected rolecards.
		vector<core::Role::ID> rolecard_ids() const;
		// The IDs of the currently selected wildcards.
		vector<core::Wildcard::ID> wildcard_ids() const;

		// Checks if a player with the given name already exists.
		bool has_player(string_view name) const;
		// Checks if at least one rolecard with the given alias has been chosen.
		bool has_rolecard(string_view alias) const;
		// Checks if at least one wildcard with the given alias has been chosen.
		bool has_wildcard(string_view alias) const;

		// The total number of players that have been chosen.
		std::size_t num_players() const;
		// The total number of rolecards that have been chosen.
		std::size_t num_rolecards() const;
		// The total number of wildcards that have been chosen.
		std::size_t num_wildcards() const;
		// The total number of cards that have been chosen.
		std::size_t num_cards() const;

		// Adds a new player with the given name.
		void add_player(string_view name);
		// Adds a single copy of the rolecard with the given alias.
		void add_rolecard(string_view alias);
		// Adds a single copy of the wildcard with the given alias.
		void add_wildcard(string_view alias);

		// Removes the player with the given name.
		void remove_player(string_view name);
		// Removes a single copy of the rolecard with the given alias.
		void remove_rolecard(string_view alias);
		// Removes a single copy of the wildcard with the given alias.
		void remove_wildcard(string_view alias);

		// Removes all of the players that have been chosen.
		void clear_all_players();
		// Removes all copies of the rolecard with the given alias.
		void clear_rolecards(string_view alias);
		// Removes all of the rolecards that have been chosen.
		void clear_all_rolecards();
		// Removes all copies of the wildcard with the given alias.
		void clear_wildcards(string_view alias);
		// Removes all of the wildcards that have been chosen.
		void clear_all_wildcards();
		// Removes all of the cards that have been chosen.
		void clear_all_cards();
		// Removes all of the players and cards that have been chosen.
		void clear_all();

		unique_ptr<Game_log> begin_pending_game();
		unique_ptr<Game_log> begin_preset(int i);

		// Handles the given commands, making alterations to the setup screen as
		// appropriate.
		// Throws an exception if the commands couldn't be interpreted.
		void do_commands(const CmdSequence & commands) override;

		void set_params(TextParams & params) const override;

	private:
		core::Rulebook _rulebook{};
		std::set<string> _player_names{};
		std::map<core::Role::ID, std::size_t, Role_ID_full_name_compare> _role_ids{};
		std::map<core::Wildcard::ID, std::size_t> _wildcard_ids{};
	};
}

#endif
