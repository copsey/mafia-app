#ifndef MAFIA_EVENT_H
#define MAFIA_EVENT_H

#include "../util/ref.hpp"
#include "../util/stdlib.hpp"

#include "../logic/logic.hpp"

#include "format.hpp"
#include "screen.hpp"

namespace maf {
	struct Game_log;

	struct Event: Screen {
		// Signifies that an event failed to process a set of commands.
		struct Bad_commands { };

		/// Create an event spawned from the given game log.
		///
		/// A reference to the game log passed in is stored in the event, in order
		/// to allow commands to be handled as a generic screen.
		Event(Game_log & game_log)
		: _game_log_ref{game_log} { }

		/// Get this event's stored game log.
		Game_log & game_log() const { return *_game_log_ref; }

		string_view txt_subdir() const override { return "txt/events/"; }

		// Handles the given commands, acting on this event's game log as
		// required.
		//
		// Throws an exception if the commands couldn't be handled.
		virtual void do_commands(const vector<string_view> & commands) = 0;

		// Write a summary of the event to `output`. This should be followed
		// by calling `preprocess_text` with the screen's parameters.
		void summarise(string & output) const;

		string escaped_name(Player const& player) const;

		string escaped_name(Role const& role) const;

	private:
		util::ref<Game_log> _game_log_ref;
	};


	struct Player_given_initial_role: Event {
		Player_given_initial_role(Game_log & game_log,
								  const Player & player,
								  const Role & role,
								  const Wildcard * wildcard)
		: Event{game_log}, _p{&player}, _r{&role}, _w{wildcard} { }

		string_view id() const final { return "player-given-role"; }

		void do_commands(const vector<string_view> & commands) override;
		void set_params(TextParams & params) const override;

	private:
		const Player *_p;
		const Role *_r;
		const Wildcard *_w;
		bool _is_private{false};
	};


	struct Time_changed: Event {
		Time_changed(Game_log & game_log, Date d, Time t)
		: Event{game_log}, date{d}, time{t} { }

		string_view id() const final { return "time-changed"; }

		Date date;
		Time time;

		void do_commands(const vector<string_view> & commands) override;
		void set_params(TextParams & params) const override;
	};


	struct Obituary: Event {
		Obituary(Game_log & game_log, vector<util::ref<const Player>> deaths)
		: Event{game_log}, _deaths{deaths} { }

		string_view id() const final { return "obituary"; }

		void do_commands(const vector<string_view> & commands) override;
		void set_params(TextParams & params) const override;

	private:
		vector<util::ref<const Player>> _deaths;
		std::ptrdiff_t _deaths_index{-1};
	};


	struct Town_meeting: Event {
		Town_meeting(Game_log & game_log,
		             vector<util::ref<const Player>> players,
		             Date date,
		             bool lynch_can_occur,
		             const Player * next_lynch_victim,
		             const Player * recent_lynch_vote_caster,
		             const Player * recent_lynch_vote_target)
		:
			Event{game_log},
			_players{players},
			_date{date},
			_lynch_can_occur{lynch_can_occur},
			_next_lynch_victim{next_lynch_victim},
			_recent_vote_caster{recent_lynch_vote_caster},
			_recent_vote_target{recent_lynch_vote_target}
		{ }

		string_view id() const final { return "town-meeting"; }

		void do_commands(const vector<string_view>& commands) override;
		void set_params(TextParams & params) const override;

	private:
		vector<util::ref<const Player>> _players;
		Date _date;
		bool _lynch_can_occur;
		const Player *_next_lynch_victim;
		const Player *_recent_vote_caster;
		const Player *_recent_vote_target;
	};


	struct Player_kicked: Event {
		Player_kicked(Game_log & game_log, const Player & player, const Role & player_role)
		: Event{game_log}, player{player}, player_role{player_role} { }

		string_view id() const final { return "player-kicked"; }

		util::ref<const Player> player;
		util::ref<const Role> player_role;

		void do_commands(const vector<string_view> & commands) override;
		void set_params(TextParams & params) const override;
	};


	struct Lynch_result: Event {
		Lynch_result(Game_log & game_log, const Player * victim, const Role * victim_role)
		: Event{game_log}, victim{victim}, victim_role{victim_role} { }

		string_view id() const final { return "lynch-result"; }

		// The player who was lynched, or `nullptr` if nobody was lynched.
		const Player *victim = nullptr;
		// The role of the player who was lynched, or `nullptr` if nobody was
		// lynched / the role could not be determined.
		const Role *victim_role = nullptr;

		void do_commands(const vector<string_view> & commands) override;
		void set_params(TextParams & params) const override;
	};


	struct Duel_result: Event {
		Duel_result(Game_log & game_log, const Player & caster, const Player & target, const Player & winner, const Player & loser)
		: Event{game_log}, caster{caster}, target{target}, winner{winner}, loser{loser}
		{ }

		string_view id() const final { return "duel-result"; }

		util::ref<const Player> caster;
		util::ref<const Player> target;
		util::ref<const Player> winner;
		util::ref<const Player> loser;

		void do_commands(const vector<string_view> & commands) override;
		void set_params(TextParams & params) const override;
	};


	struct Choose_fake_role: Event {
		Choose_fake_role(Game_log & game_log, const Player & player)
			: Event{game_log}, _player{&player}
		{ }

		string_view id() const final { return "choose-fake-role"; }

		void do_commands(const vector<string_view> & commands) override;
		void set_params(TextParams & params) const override;

	private:
		const Player *_player;
		const Role *_fake_role{nullptr};
		bool _go_to_sleep{false};
	};


	struct Mafia_meeting: Event {
		Mafia_meeting(Game_log & game_log,
		              vector<util::ref<const Player>> mafiosi,
		              bool is_initial_meeting)
			: Event{game_log}, _mafiosi{mafiosi}, _initial{is_initial_meeting}
		{ }

		string_view id() const final { return "mafia-meeting"; }

		void do_commands(const vector<string_view> & commands) override;
		void set_params(TextParams & params) const override;

	private:
		vector<util::ref<const Player>> _mafiosi;
		bool _initial;
		bool _go_to_sleep{false};
	};


	struct Kill_use: Event {
		Kill_use(Game_log & game_log, const Player & caster)
			: Event{game_log}, _caster{&caster}
		{ }

		string_view id() const final { return "use-kill"; }

		void do_commands(const vector<string_view>& commands) override;
		void set_params(TextParams & params) const override;

	private:
		const Player *_caster;
		bool _go_to_sleep{false};
	};


	struct Heal_use: Event {
		Heal_use(Game_log & game_log, const Player & caster)
			: Event{game_log}, _caster{&caster}
		{ }

		string_view id() const final { return "use-heal"; }

		void do_commands(const vector<string_view> & commands) override;
		void set_params(TextParams & params) const override;

	private:
		const Player *_caster;
		bool _go_to_sleep{false};
	};


	struct Investigate_use: Event {
		Investigate_use(Game_log & game_log, const Player & caster)
			: Event{game_log}, _caster{&caster}
		{ }

		string_view id() const final { return "use-investigate"; }

		void do_commands(const vector<string_view> & commands) override;
		void set_params(TextParams & params) const override;

	private:
		const Player *_caster;
		bool _go_to_sleep{false};
	};


	struct Peddle_use: Event {
		Peddle_use(Game_log & game_log, const Player & caster)
			: Event{game_log}, _caster{&caster}
		{ }

		string_view id() const final { return "use-peddle"; }

		void do_commands(const vector<string_view> & commands) override;
		void set_params(TextParams & params) const override;

	private:
		const Player *_caster;
		bool _go_to_sleep{false};
	};


	struct Boring_night: Event {
		Boring_night(Game_log & game_log)
			: Event{game_log}
		{ }

		string_view id() const final { return "boring-night"; }

		void do_commands(const vector<string_view> & commands) override;
		void set_params(TextParams & params) const override;
	};


	struct Investigation_result: Event {
		Investigation_result(Game_log & game_log, Investigation investigation)
		: Event{game_log}, investigation{investigation} { }

		string_view id() const final { return "investigation-result"; }

		Investigation investigation;

		void do_commands(const vector<string_view> & commands) override;
		void set_params(TextParams & params) const override;

	private:
		bool _go_to_sleep{false};
	};


	struct Game_ended: Event {
		Game_ended(Game_log & game_log)
		: Event{game_log} { }

		string_view id() const final
		{ return "game-ended"; }

		void do_commands(const vector<string_view> & commands) override;
		void set_params(TextParams & params) const override;
	};
}

#endif
