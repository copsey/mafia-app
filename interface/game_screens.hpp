#ifndef MAFIA_GAME_SCREENS
#define MAFIA_GAME_SCREENS

#include "../util/stdlib.hpp"
#include "../util/string.hpp"

#include "../core/core.hpp"

#include "command.hpp"
#include "format.hpp"
#include "game_log.hpp"
#include "screen.hpp"

namespace maf {
	struct Console;
	struct Game_log;

	struct Game_screen: Screen {
		using Screen::Screen;

		const Game & game() const;
		Game_log & game_log();
		const Game_log & game_log() const;

		string_view txt_subdir() const override { return "txt/game_screens/"; }

		void do_commands(const CmdSequence & commands) override;

		// Write a summary of the event to `output`. This should be followed
		// by calling `preprocess_text` with the screen's parameters.
		void summarise(string & output) const;

		string escaped_name(Player const& player) const;
		string escaped_name(Role const& role) const;
	};


	struct Player_given_initial_role: Game_screen {
		Player_given_initial_role(Console & console,
								  const Player & player,
								  const Role & role)
		: Game_screen{console}, _player{&player}, _role{&role} { }

		string_view id() const final { return "player-given-role"; }

		void do_commands(const CmdSequence & commands) override;
		void set_params(TextParams & params) const override;

	private:
		not_null<const Player *> _player;
		not_null<const Role *> _role;
		bool _is_private{false};
	};


	class Wildcards_resolved : public Game_screen {
	public:
		using Game_screen::Game_screen;

		string_view id() const final { return "wildcards-resolved"; }

		void do_commands(const CmdSequence & commands) override;
		void set_params(TextParams & params) const override;
	};


	struct Time_changed: Game_screen {
		Time_changed(Console & console, Date d, Time t)
		: Game_screen{console}, date{d}, time{t} { }

		string_view id() const final { return "time-changed"; }

		Date date;
		Time time;

		void do_commands(const CmdSequence & commands) override;
		void set_params(TextParams & params) const override;
	};


	struct Obituary: Game_screen {
		Obituary(Console & console, vector_of_refs<const Player> deaths)
		: Game_screen{console}, _deaths{deaths} { }

		string_view id() const final { return "obituary"; }

		void do_commands(const CmdSequence & commands) override;
		void set_params(TextParams & params) const override;

	private:
		vector_of_refs<const Player> _deaths;
		std::ptrdiff_t _deaths_index{-1};

		TextParams _get_params(const Player & player) const;
	};


	struct Town_meeting: Game_screen {
		Town_meeting(Console & console,
		             vector_of_refs<const Player> players,
		             Date date,
		             bool lynch_can_occur,
		             const Player * next_lynch_victim,
		             const Player * recent_lynch_vote_caster,
		             const Player * recent_lynch_vote_target)
		:
			Game_screen{console},
			_players{players},
			_date{date},
			_lynch_can_occur{lynch_can_occur},
			_next_lynch_victim{next_lynch_victim},
			_recent_vote_caster{recent_lynch_vote_caster},
			_recent_vote_target{recent_lynch_vote_target}
		{ }

		string_view id() const final { return "town-meeting"; }

		void do_commands(const CmdSequence & commands) override;
		void set_params(TextParams & params) const override;

	private:
		vector_of_refs<const Player> _players;
		Date _date;
		bool _lynch_can_occur;
		const Player *_next_lynch_victim;
		const Player *_recent_vote_caster;
		const Player *_recent_vote_target;

		TextParams _get_params(const Player & player) const;

		void _do_commands_before_lynch(const CmdSequence & commands);
		void _do_commands_after_lynch(const CmdSequence & commands);
		void _do_other_commands(const CmdSequence & commands);
	};


	struct Player_kicked: Game_screen {
		Player_kicked(Console & console, const Player & player)
		: Game_screen{console}, _player{player} { }

		string_view id() const final { return "player-kicked"; }

		void do_commands(const CmdSequence & commands) override;
		void set_params(TextParams & params) const override;

	private:
		const Player & _player;
	};


	struct Lynch_result: Game_screen {
		Lynch_result(Console & console, const Player * victim, const Role * victim_role)
		: Game_screen{console}, victim{victim}, victim_role{victim_role} { }

		string_view id() const final { return "lynch-result"; }

		// The player who was lynched, or `nullptr` if nobody was lynched.
		const Player *victim = nullptr;
		// The role of the player who was lynched, or `nullptr` if nobody was
		// lynched / the role could not be determined.
		const Role *victim_role = nullptr;

		void do_commands(const CmdSequence & commands) override;
		void set_params(TextParams & params) const override;
	};


	struct Duel_result: Game_screen {
		Duel_result(Console & console, const Player & caster, const Player & target, const Player & winner, const Player & loser)
		: Game_screen{console}, caster{caster}, target{target}, winner{winner}, loser{loser} { }

		string_view id() const final { return "duel-result"; }

		const Player & caster;
		const Player & target;
		const Player & winner;
		const Player & loser;

		void do_commands(const CmdSequence & commands) override;
		void set_params(TextParams & params) const override;
	};


	struct Choose_fake_role: Game_screen {
		Choose_fake_role(Console & console, const Player & player)
		: Game_screen{console}, _player{&player} { }

		string_view id() const final { return "choose-fake-role"; }

		void do_commands(const CmdSequence & commands) override;
		void set_params(TextParams & params) const override;

	private:
		not_null<const Player *> _player;
		const Role * _fake_role{nullptr};
		bool _finished{false};
	};


	struct Mafia_meeting: Game_screen {
		Mafia_meeting(Console & console,
					  vector_of_refs<const Player> mafiosi,
		              bool is_first_meeting)
		: Game_screen{console}, _mafiosi{mafiosi}, _first_meeting{is_first_meeting}
		{ }

		string_view id() const final { return "mafia-meeting"; }

		void do_commands(const CmdSequence & commands) override;
		void set_params(TextParams & params) const override;

	private:
		vector_of_refs<const Player> _mafiosi;
		bool _first_meeting;
		bool _finished{false};

		void _do_commands_for_first_meeting(const CmdSequence & commands);
		void _do_commands_for_regular_meeting(const CmdSequence & commands);
	};


	struct Kill_use: Game_screen {
		Kill_use(Console & console, const Player & caster)
		: Game_screen{console}, _caster{caster} { }

		string_view id() const final { return "use-kill"; }

		void do_commands(const CmdSequence & commands) override;
		void set_params(TextParams & params) const override;

	private:
		const Player & _caster;
		bool _finished{false};
	};


	struct Heal_use: Game_screen {
		Heal_use(Console & console, const Player & caster)
		: Game_screen{console}, _caster{caster} { }

		string_view id() const final { return "use-heal"; }

		void do_commands(const CmdSequence & commands) override;
		void set_params(TextParams & params) const override;

	private:
		const Player & _caster;
		bool _finished{false};
	};


	struct Investigate_use: Game_screen {
		Investigate_use(Console & console, const Player & caster)
		: Game_screen{console}, _caster{caster} { }

		string_view id() const final { return "use-investigate"; }

		void do_commands(const CmdSequence & commands) override;
		void set_params(TextParams & params) const override;

	private:
		const Player & _caster;
		bool _finished{false};
	};


	struct Peddle_use: Game_screen {
		Peddle_use(Console & console, const Player & caster)
		: Game_screen{console}, _caster{caster} { }

		string_view id() const final { return "use-peddle"; }

		void do_commands(const CmdSequence & commands) override;
		void set_params(TextParams & params) const override;

	private:
		const Player & _caster;
		bool _finished{false};
	};


	struct Boring_night: Game_screen {
		using Game_screen::Game_screen;

		string_view id() const final { return "boring-night"; }

		void do_commands(const CmdSequence & commands) override;
		void set_params(TextParams & params) const override;
	};


	struct Investigation_result: Game_screen {
		Investigation_result(Console & console, Investigation investigation)
		: Game_screen{console}, investigation{investigation} { }

		string_view id() const final { return "investigation-result"; }

		Investigation investigation;

		void do_commands(const CmdSequence & commands) override;
		void set_params(TextParams & params) const override;

	private:
		bool _finished{false};
	};


	struct Game_ended: Game_screen {
		using Game_screen::Game_screen;

		string_view id() const final { return "game-ended"; }

		void do_commands(const CmdSequence & commands) override;
		void set_params(TextParams & params) const override;
	};
}

#endif
