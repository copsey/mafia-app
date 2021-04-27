#ifndef MAFIA_GAME_LOG_H
#define MAFIA_GAME_LOG_H

#include <memory>
#include <string_view>

#include "../util/stdlib.hpp"
#include "../util/type_traits.hpp"

#include "../logic/logic.hpp"

namespace maf {
	struct Console;
	struct Game_screen;

	struct Game_log {
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
			Ability ability;
		};

		// Signifies that there are no more screens to advance to.
		struct Cannot_advance { };

		// Creates a new game log, managing a game with the given parameters.
		Game_log(Console & console,
				 const vector<string> &player_names,
		         const vector<Role::ID> &role_ids,
		         const vector<Wildcard::ID> &wildcard_ids,
		         const Rulebook &rulebook = Rulebook{});

		// The game being managed.
		Game const& game;
		// All of the players in the game.
		vector<Player> const& players;

		// methods inherited from Game
		//
		bool contains(RoleRef r_ref) const { return _game.contains(r_ref); }
		Role const& look_up(RoleRef r_ref) const { return _game.look_up(r_ref); }
		//

		// The game screen that's currently active.
		const Game_screen & active_screen() const {
			return *_screen_stack[_screen_stack_index];
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
		const Player & find_player(Player::ID id) const;

		// Finds the player whose name matches the given string, up to case.
		// Throws an exception if no such player could be found.
		const Player & find_player(string_view name) const;

		/// Get the name of the given player.
		string_view get_name(const Player & player) const;

		/// Get the name of the player with the given ID.
		string_view get_name(Player::ID id) const;




		void kick_player(Player::ID id);

		void cast_lynch_vote(Player::ID voter_id, Player::ID target_id);
		void clear_lynch_vote(Player::ID voter_id);
		void process_lynch_votes();

		void stage_duel(Player::ID caster_id, Player::ID target_id);

		void begin_night();

		void choose_fake_role(Player::ID player_id, Role::ID fake_role_id);

		void cast_mafia_kill(Player::ID caster_id, Player::ID target_id);
		void skip_mafia_kill();

		void cast_kill(Player::ID caster_id, Player::ID target_id);
		void skip_kill(Player::ID caster_id);

		void cast_heal(Player::ID caster_id, Player::ID target_id);
		void skip_heal(Player::ID caster_id);

		void cast_investigate(Player::ID caster_id, Player::ID target_id);
		void skip_investigate(Player::ID caster_id);

		void cast_peddle(Player::ID caster_id, Player::ID target_id);
		void skip_peddle(Player::ID caster_id);




	private:
		Game _game;

		vector<string> _player_names;

		vector<unique_ptr<Game_screen>> _screen_stack{};
		decltype(_screen_stack)::size_type _screen_stack_index{0};

		Console & _console;

		template <typename ScreenType, typename... Args>
		auto _append_screen(Args&&... args)
		-> enable_if<is_base_of<Game_screen, ScreenType>> {
			auto screen = make_unique<ScreenType>(_console, args...);
			_screen_stack.push_back(move(screen));
		}

		// Adds the specified event to the end of the log.
		void log_player_given_role(Player const& player);
		void log_time_changed();
		void log_time_changed(Date date, Time time);
		void log_obituary(Date date);
		void log_town_meeting(const Player *recent_vote_caster = nullptr, const Player *recent_vote_target = nullptr);
		void log_lynch_result(const Player *victim);
		void log_duel_result(const Player &caster, const Player &target);
		void log_ability_use(Player const& player);
		void log_mafia_meeting(bool initial_meeting);
		void log_boring_night();
		void log_investigation_result(Investigation investigation);
		void log_game_ended();
		// Try to log that the night has ended, if this has actually happened.
		void try_to_log_night_ended();
	};
}

#endif
