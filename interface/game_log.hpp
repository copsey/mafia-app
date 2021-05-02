#ifndef MAFIA_GAME_LOG_H
#define MAFIA_GAME_LOG_H

#include <memory>
#include <string_view>

#include "../util/misc.hpp"
#include "../util/stdlib.hpp"
#include "../util/type_traits.hpp"

#include "../core/core.hpp"

namespace maf {
	class Console;
	class Game_screen;

	class Game_log {
	public:
		/// Exception signifying that an attempt was made to create a game with
		/// an unequal number of players and cards.
		struct Players_to_cards_mismatch {
			std::size_t num_players;
			std::size_t num_cards;
		};

		// Signifies that no player could be found with the given name.
		struct Player_not_found {
			string name;
		};

		// An exception signifying that an ability use has not been programmed in
		// yet.
		struct Unexpected_ability {
			core::Ability ability;
		};

		// Signifies that there are no more screens to advance to.
		struct Cannot_advance { };

		// Creates a new game log, managing a game with the given parameters.
		Game_log(Console & console,
				 const vector<string> &player_names,
		         const vector<core::Role::ID> &role_ids,
		         const vector<core::Wildcard::ID> &wildcard_ids,
		         const core::Rulebook &rulebook = {});

		// The game being managed.
		const core::Game & game() const { return _game; }
		// All of the players in the game.
		const vector<core::Player> & players() const { return game().players(); }

		// methods inherited from Game
		//
		bool contains(core::RoleRef r_ref) const { return _game.contains(r_ref); }
		core::Role const& look_up(core::RoleRef r_ref) const { return _game.look_up(r_ref); }
		//

		// The game screen that's currently active.
		const Game_screen & active_screen() const {
			return *_screen_stack[_screen_stack_idx];
		}

		// Tries to continue forward to the next screen.
		// Throws an exception if all of the screens in the game log have been
		// depleted.
		void advance();

		// Handles the given commands, by passing them to the active screen.
		// Throws an exception if the commands couldn't be handled.
		void do_commands(const vector<string_view> & commands);

		// Writes a transcript to `str`, containing a summary of every event
		// that has occurred so far, in chronological order.
		void write_transcript(string & str) const;

		// Finds the player with the given ID.
		// Throws an exception if no such player could be found.
		const core::Player & find_player(core::Player::ID id) const;

		// Finds the player whose name matches the given string, up to case.
		// Throws an exception if no such player could be found.
		const core::Player & find_player(string_view name) const;

		/// Get the name of the given player.
		string_view get_name(const core::Player & player) const;

		/// Get the name of the player with the given ID.
		string_view get_name(core::Player::ID id) const;

		void kick_player(core::Player::ID id);

		void cast_lynch_vote(core::Player::ID voter_id, core::Player::ID target_id);
		void clear_lynch_vote(core::Player::ID voter_id);
		void process_lynch_votes();

		void stage_duel(core::Player::ID caster_id, core::Player::ID target_id);

		void begin_night();

		void choose_fake_role(core::Player::ID player_id, core::Role::ID fake_role_id);

		void cast_mafia_kill(core::Player::ID caster_id, core::Player::ID target_id);
		void skip_mafia_kill();

		void cast_kill(core::Player::ID caster_id, core::Player::ID target_id);
		void skip_kill(core::Player::ID caster_id);

		void cast_heal(core::Player::ID caster_id, core::Player::ID target_id);
		void skip_heal(core::Player::ID caster_id);

		void cast_investigate(core::Player::ID caster_id, core::Player::ID target_id);
		void skip_investigate(core::Player::ID caster_id);

		void cast_peddle(core::Player::ID caster_id, core::Player::ID target_id);
		void skip_peddle(core::Player::ID caster_id);

	private:
		core::Game _game;

		vector<string> _player_names;

		vector<unique_ptr<Game_screen>> _screen_stack{};
		index _screen_stack_idx{0};

		not_null<Console *> _console;

		template <typename ScreenType, typename... Args>
		enable_if<is_base_of<Game_screen, ScreenType>>
		_append_screen(Args&&... args) {
			auto screen = make_unique<ScreenType>(*_console, args...);
			_screen_stack.push_back(move(screen));
		}

		// Adds the specified event to the end of the log.
		void log_player_given_role(core::Player const& player);
		void log_time_changed();
		void log_time_changed(core::Date date, core::Time time);
		void log_obituary(core::Date date);
		void log_town_meeting(const core::Player *recent_vote_caster = nullptr, const core::Player *recent_vote_target = nullptr);
		void log_lynch_result(const core::Player *victim);
		void log_duel_result(const core::Player &caster, const core::Player &target);
		void log_ability_use(core::Player const& player);
		void log_mafia_meeting(bool initial_meeting);
		void log_boring_night();
		void log_investigation_result(core::Investigation investigation);
		void log_game_ended();
		// Try to log that the night has ended, if this has actually happened.
		void try_to_log_night_ended();
	};
}

#endif
