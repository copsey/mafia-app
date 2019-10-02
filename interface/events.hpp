#ifndef MAFIA_EVENT_H
#define MAFIA_EVENT_H

#include <ostream>
#include <string>

#include "../riketi/ref.hpp"

#include "styled_string.hpp"
#include "../logic/logic.hpp"

namespace maf {
	struct Game_log;

	struct Event {
		// Signifies that an event failed to process a set of commands.
		struct Bad_commands { };

		/// Create an event spawned from the given game log.
		///
		/// A reference to the game log passed in is stored in the event, in order
		/// to allow commands to be handled as a generic screen.
		Event(Game_log & game_log)
		: _game_log_ref{game_log}
		{ }

		virtual ~Event() = default;

		/// Get this event's stored game log.
		Game_log & game_log() const 
		{
			return *_game_log_ref;
		}

		// Handles the given commands, acting on this event's game log as required.
		// Throws an exception if the commands couldn't be handled.
		virtual void do_commands(const std::vector<std::string_view> & commands) = 0;

		// Writes a tagged string detailing the event to os.
		virtual void write_full(std::ostream & os, TextParams& params) const = 0;
		// Writes a summary of the event to os.
		// (this is a "normal" string, and not a tagged string.)
		// By default, nothing is written at all.
		virtual void write_summary(std::ostream & os) const;
		// Writes a tagged string to os containing help with the event.
		// By default, complains that no help has been written for the event.
		virtual void write_help(std::ostream & os, TextParams& params) const;

	private:
		rkt::ref<Game_log> _game_log_ref;
	};


	struct Player_given_initial_role: Event {
		Player_given_initial_role(Game_log & game_log,
										  const Player & player,
										  const Role & role,
										  const Wildcard * wildcard)
			: Event{game_log}, _p{&player}, _r{&role}, _w{wildcard}
		{ }

		void do_commands(const std::vector<std::string_view> & commands) override;

		void write_full(std::ostream & os, TextParams& params) const override;
		void write_summary(std::ostream & os) const override;

	private:
		const Player *_p;
		const Role *_r;
		const Wildcard *_w;
		bool _is_private{false};
	};


	struct Time_changed: Event {
		Time_changed(Game_log & game_log, Date d, Time t)
			: Event{game_log}, date{d}, time{t}
		{ }

		Date date;
		Time time;

		void do_commands(const std::vector<std::string_view> & commands) override;

		void write_full(std::ostream & os, TextParams& params) const override;
		void write_summary(std::ostream & os) const override;
	};


	struct Obituary: Event {
		Obituary(Game_log & game_log, std::vector<rkt::ref<const Player>> deaths)
			: Event{game_log}, _deaths{deaths}
		{ }

		void do_commands(const std::vector<std::string_view> & commands) override;

		void write_full(std::ostream& os, TextParams& params) const override;
		void write_summary(std::ostream& os) const override;

	private:
		std::vector<rkt::ref<const Player>> _deaths;
		std::ptrdiff_t _deaths_index{-1};
	};


	struct Town_meeting: Event {
		Town_meeting(Game_log & game_log,
		             std::vector<rkt::ref<const Player>> players,
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

		void do_commands(const std::vector<std::string_view>& commands) override;

		void write_full(std::ostream &os, TextParams& params) const override;
		void write_summary(std::ostream &os) const override;

	private:
		std::vector<rkt::ref<const Player>> _players;
		Date _date;
		bool _lynch_can_occur;
		const Player *_next_lynch_victim;
		const Player *_recent_vote_caster;
		const Player *_recent_vote_target;
	};


	struct Player_kicked: Event {
		Player_kicked(Game_log & game_log, const Player & player, const Role & player_role)
			: Event{game_log}, player{player}, player_role{player_role}
		{ }

		void do_commands(const std::vector<std::string_view> & commands) override;

		void write_full(std::ostream &os, TextParams& params) const override;
		void write_summary(std::ostream &os) const override;

		rkt::ref<const Player> player;
		rkt::ref<const Role> player_role;
	};


	struct Lynch_result: Event {
		Lynch_result(Game_log & game_log, const Player * victim, const Role * victim_role)
			: Event{game_log}, victim{victim}, victim_role{victim_role}
		{ }

		// The player who was lynched, or nullptr if nobody was lynched.
		const Player *victim;
		// The role of the player who was lynched, or nullptr if nobody was
		// lynched / the role could not be determined.
		const Role *victim_role;

		void do_commands(const std::vector<std::string_view> & commands) override;

		void write_full(std::ostream &os, TextParams& params) const override;
		void write_summary(std::ostream &os) const override;
	};


	struct Duel_result: Event {
		Duel_result(Game_log & game_log, const Player & caster, const Player & target)
			: Event{game_log}, caster{caster}, target{target}
		{ }

		void do_commands(const std::vector<std::string_view> & commands) override;

		void write_full(std::ostream &os, TextParams& params) const override;
		void write_summary(std::ostream &os) const override;

		rkt::ref<const Player> caster;
		rkt::ref<const Player> target;
	};


	struct Choose_fake_role: Event {
		Choose_fake_role(Game_log & game_log, const Player & player)
			: Event{game_log}, _player{&player}
		{ }

		void do_commands(const std::vector<std::string_view> & commands) override;

		void write_full(std::ostream &os, TextParams& params) const override;
		void write_summary(std::ostream &os) const override;

	private:
		const Player *_player;
		const Role *_fake_role{nullptr};
		bool _go_to_sleep{false};
	};


	struct Mafia_meeting: Event {
		Mafia_meeting(Game_log & game_log,
		              std::vector<rkt::ref<const Player>> mafiosi,
		              bool is_initial_meeting)
			: Event{game_log}, _mafiosi{mafiosi}, _initial{is_initial_meeting}
		{ }

		void do_commands(const std::vector<std::string_view> & commands) override;

		void write_full(std::ostream &os, TextParams& params) const override;

	private:
		std::vector<rkt::ref<const Player>> _mafiosi;
		bool _initial;
		bool _go_to_sleep{false};
	};


	struct Kill_use: Event {
		Kill_use(Game_log & game_log, const Player & caster)
			: Event{game_log}, _caster{&caster}
		{ }

		void do_commands(const std::vector<std::string_view>& commands) override;

		void write_full(std::ostream &os, TextParams& params) const override;

	private:
		const Player *_caster;
		bool _go_to_sleep{false};
	};


	struct Heal_use: Event {
		Heal_use(Game_log & game_log, const Player & caster)
			: Event{game_log}, _caster{&caster}
		{ }

		void do_commands(const std::vector<std::string_view> & commands) override;

		void write_full(std::ostream &os, TextParams& params) const override;

	private:
		const Player *_caster;
		bool _go_to_sleep{false};
	};


	struct Investigate_use: Event {
		Investigate_use(Game_log & game_log, const Player & caster)
			: Event{game_log}, _caster{&caster}
		{ }

		void do_commands(const std::vector<std::string_view> & commands) override;

		void write_full(std::ostream &os, TextParams& params) const override;

	private:
		const Player *_caster;
		bool _go_to_sleep{false};
	};


	struct Peddle_use: Event {
		Peddle_use(Game_log & game_log, const Player & caster)
			: Event{game_log}, _caster{&caster}
		{ }

		void do_commands(const std::vector<std::string_view> & commands) override;

		void write_full(std::ostream &os, TextParams& params) const override;

	private:
		const Player *_caster;
		bool _go_to_sleep{false};
	};


	struct Boring_night: Event {
		Boring_night(Game_log & game_log)
			: Event{game_log}
		{ }

		void do_commands(const std::vector<std::string_view> & commands) override;

		void write_full(std::ostream &os, TextParams& params) const override;
	};


	struct Investigation_result: Event {
		Investigation_result(Game_log & game_log, Investigation investigation)
			: Event{game_log}, investigation{investigation}
		{ }

		Investigation investigation;

		void do_commands(const std::vector<std::string_view> & commands) override;

		void write_full(std::ostream &os, TextParams& params) const override;
		void write_summary(std::ostream &os) const override;

	private:
		bool _go_to_sleep{false};
	};


	struct Game_ended: Event {
		Game_ended(Game_log & game_log)
			: Event{game_log}
		{ }

		void do_commands(const std::vector<std::string_view> & commands) override;

		void write_full(std::ostream &os, TextParams& params) const override;
		void write_summary(std::ostream &os) const override;
	};
}

#endif
