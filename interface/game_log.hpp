#ifndef MAFIA_GAME_LOG_H
#define MAFIA_GAME_LOG_H

#include <memory>
#include <string_view>

#include "../util/stdlib.hpp"
#include "events.hpp"

namespace maf {
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

		// Signifies that there are no more events to advance to.
		struct Cannot_advance { };

		// Creates a new game log, managing a game with the given parameters.
		Game_log(const vector<string> &player_names,
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

		// The current event in the log.
		// There will always be at least one event.
		const Event & current_event() const {
			return *_log[_log_index];
		}

		// The total number of events that have occurred in the game.
		std::size_t num_events() const {
			return _log.size();
		}

		// Tries to continue forward to the next state in the events.
		// Throws an exception if all of the events in the game log have been
		// depleted.
		void advance();

		// Handles the given commands, by passing them to the current event.
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

		vector<unique_ptr<Event>> _log{};
		decltype(_log)::size_type _log_index{0};

		template <typename ScreenType, typename... Args>
		auto emplace_screen(Args&&... args)
		-> std::enable_if_t<std::is_base_of_v<Event, ScreenType>> {
			auto event = make_unique<ScreenType>(*this, args...);
			_log.push_back(move(event));
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
