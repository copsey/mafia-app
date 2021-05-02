#include <sstream>

#include "../util/algorithm.hpp"
#include "../util/string.hpp"

#include "game_log.hpp"
#include "game_screens.hpp"

namespace maf {
	Game_log::Game_log (
		Console & console,
		const vector<string> & player_names,
		const vector<core::Role::ID> & role_ids,
		const vector<core::Wildcard::ID> & wildcard_ids,
		const core::Rulebook & rulebook)
	:
		_console{&console},
		_game{role_ids, wildcard_ids, rulebook},
		_player_names{player_names}
	{
		if (player_names.size() != role_ids.size() + wildcard_ids.size()) {
			throw Players_to_cards_mismatch{player_names.size(), role_ids.size() + wildcard_ids.size()};
		}

		if (!game().random_roles().empty()) {
			_append_screen<Wildcards_resolved>();
		}

		for (const core::Player & player: players()) {
			log_player_given_role(player);
		}

		if (_game.game_has_ended()) {
			log_game_ended();
			return;
		}

		_game.begin_night();
		log_time_changed(core::Date{0}, core::Time::night);

		auto prev_size = _screen_stack.size();

		if (_game.num_players_left(core::Alignment::mafia) > 0) {
			log_mafia_meeting(true);
		}

		for (const core::Player & player: players()) {
			if (player.is_present() && player.role().is_role_faker() && !player.has_fake_role()) {
				_append_screen<Choose_fake_role>(player);
			}
		}

		if (_screen_stack.size() == prev_size) {
			log_boring_night();
		} else {
			util::shuffle(_screen_stack.begin() + prev_size, _screen_stack.end());
		}

		try_to_log_night_ended();
	}

	void Game_log::advance() {
		if (_screen_stack_idx + 1 < _screen_stack.size()) {
			++_screen_stack_idx;
		} else {
			throw Cannot_advance{};
		}
	}

	void Game_log::do_commands(const vector<string_view> & commands) {
		_screen_stack[_screen_stack_idx]->do_commands(commands);
	}

	void Game_log::write_transcript(string & output) const {
		for (auto& event: _screen_stack) {
			string raw_text;
			event->summarise(raw_text);

			if (!raw_text.empty()) {
				TextParams params;
				event->set_params(params);

				try {
					auto summary = preprocess_text(raw_text, params);

					if (!summary.empty()) {
						output += summary;
						char last_char = summary.back();
						if (last_char != '\n') output += '\n';
					}
				} catch (preprocess_text_error & error) {
					output += "ERROR: Unable to summarise \"";
					output += escaped(event->id());
					output += "\".\n";
				}
			}
		}
	}

	const core::Player & Game_log::find_player(core::Player::ID id) const {
		for (const core::Player & player: players()) {
			if (id == player.id()) return player;
		}

		/* FIXME: throw exception in Game_log namespace, or remove this function. */
		throw core::Game::Player_not_found{id};
	}

	const core::Player & Game_log::find_player(string_view name) const {
		auto names_match = [&](const core::Player & player) -> bool {
			return util::equal_up_to_case(name, this->get_name(player));
		};

		auto iter = util::find_if(players(), names_match);

		if (iter == players().end())
			throw Player_not_found{string{name}};

		return *iter;
	}

	string_view Game_log::get_name(const core::Player & player) const {
		return get_name(player.id());
	}

	string_view Game_log::get_name(core::Player::ID id) const {
		return _player_names[id];
	}

	void Game_log::kick_player(core::Player::ID id) {
		_game.kick_player(id);
		const core::Player & player = find_player(id);
		_append_screen<Player_kicked>(player);

		if (_game.game_has_ended()) {
			log_game_ended();
		} else {
			log_town_meeting();
		}
	}

	void Game_log::cast_lynch_vote(core::Player::ID voter_id, core::Player::ID target_id) {
		const core::Player & voter = find_player(voter_id);
		const core::Player & target = find_player(target_id);

		_game.cast_lynch_vote(voter.id(), target.id());
		log_town_meeting(&voter, &target);
	}

	void Game_log::clear_lynch_vote(core::Player::ID voter_id) {
		const core::Player & voter = find_player(voter_id);

		_game.clear_lynch_vote(voter.id());
		log_town_meeting(&voter);
	}

	void Game_log::process_lynch_votes() {
		const core::Player * victim = _game.process_lynch_votes();
		log_lynch_result(victim);

		if (_game.game_has_ended()) {
			log_game_ended();
		} else {
			log_town_meeting();
		}
	}

	void Game_log::stage_duel(core::Player::ID caster_id, core::Player::ID target_id) {
		const core::Player & caster = find_player(caster_id);
		const core::Player & target = find_player(target_id);

		_game.stage_duel(caster.id(), target.id());
		log_duel_result(caster, target);

		if (_game.game_has_ended()) {
			log_game_ended();
		} else {
			log_town_meeting();
		}
	}

	void maf::Game_log::begin_night() {
		_game.begin_night();
		log_time_changed();

		auto screens_before_night = _screen_stack.size();

		if (_game.mafia_can_use_kill()) log_mafia_meeting(false);

		/* FIXME: minimise number of events when a player has multiple things to do this night. */
		for (const core::Player & player : players()) {
			log_ability_use(player);
		}

		if (_screen_stack.size() == screens_before_night) {
			log_boring_night();
		} else {
			util::shuffle(_screen_stack.begin() + screens_before_night, _screen_stack.end());
		}

		try_to_log_night_ended();
	}

	void Game_log::choose_fake_role(core::Player::ID player_id, core::Role::ID fake_role_id) {
		_game.choose_fake_role(player_id, fake_role_id);
		try_to_log_night_ended();
	}

	void Game_log::cast_mafia_kill(core::Player::ID caster_id, core::Player::ID target_id) {
		_game.cast_mafia_kill(caster_id, target_id);
		try_to_log_night_ended();
	}

	void Game_log::skip_mafia_kill() {
		_game.skip_mafia_kill();
		try_to_log_night_ended();
	}

	void Game_log::cast_kill(core::Player::ID caster_id, core::Player::ID target_id) {
		_game.cast_kill(caster_id, target_id);
		try_to_log_night_ended();
	}

	void Game_log::skip_kill(core::Player::ID caster_id) {
		_game.skip_kill(caster_id);
		try_to_log_night_ended();
	}

	void Game_log::cast_heal(core::Player::ID caster_id, core::Player::ID target_id) {
		_game.cast_heal(caster_id, target_id);
		try_to_log_night_ended();
	}

	void Game_log::skip_heal(core::Player::ID caster_id) {
		_game.skip_heal(caster_id);
		try_to_log_night_ended();
	}

	void Game_log::cast_investigate(core::Player::ID caster_id, core::Player::ID target_id) {
		_game.cast_investigate(caster_id, target_id);
		try_to_log_night_ended();
	}

	void Game_log::skip_investigate(core::Player::ID caster_id) {
		_game.skip_investigate(caster_id);
		try_to_log_night_ended();
	}

	void Game_log::cast_peddle(core::Player::ID caster_id, core::Player::ID target_id) {
		_game.cast_peddle(caster_id, target_id);
		try_to_log_night_ended();
	}

	void Game_log::skip_peddle(core::Player::ID caster_id) {
		_game.skip_peddle(caster_id);
		try_to_log_night_ended();
	}

	void Game_log::log_player_given_role(const core::Player & player) {
		_append_screen<Player_given_initial_role>(player, player.role());
	}

	void Game_log::log_time_changed() {
		log_time_changed(_game.date(), _game.time());
	}

	void Game_log::log_time_changed(core::Date date, core::Time time) {
		_append_screen<Time_changed>(date, time);
	}

	void Game_log::log_obituary(core::Date date) {
		auto died_this_night = [&](const core::Player & player) {
			return (player.is_dead()
					&& player.time_of_death() == core::Time::night
					&& player.date_of_death() == date);
		};

		auto deaths = util::filtered_crefs(players(), died_this_night);
		_append_screen<Obituary>(move(deaths));
	}

	void Game_log::log_town_meeting(const core::Player *recent_vote_caster, const core::Player *recent_vote_target) {
		auto is_present = [](const core::Player & player) {
			return player.is_present();
		};

		auto townsfolk = util::filtered_crefs(players(), is_present);
		_append_screen<Town_meeting>(move(townsfolk), _game.date(), _game.lynch_can_occur(), _game.next_lynch_victim(), recent_vote_caster, recent_vote_target);
	}

	void Game_log::log_lynch_result(const core::Player *victim) {
		const core::Role * role = victim ? &victim->role() : nullptr;
		_append_screen<Lynch_result>(victim, role);
	}

	void Game_log::log_duel_result(const core::Player &caster, const core::Player &target) {
		auto& winner = caster.is_alive() ? caster : target;
		auto& loser  = caster.is_alive() ? target : caster;

		_append_screen<Duel_result>(caster, target, winner, loser);
	}

	void Game_log::log_ability_use(const core::Player & player) {
		for (core::Ability ability: player.compulsory_abilities()) {
			switch (ability.id) {
			using ID = core::Ability::ID;
			case ID::kill:
				_append_screen<Kill_use>(player);
				break;
			case ID::heal:
				_append_screen<Heal_use>(player);
				break;
			case ID::investigate:
				_append_screen<Investigate_use>(player);
				break;
			case ID::peddle:
				_append_screen<Peddle_use>(player);
				break;
			default:
				throw Unexpected_ability{ability};
			}
		}
	}

	void Game_log::log_mafia_meeting(bool initial_meeting) {
		_append_screen<Mafia_meeting>(_game.remaining_players(core::Alignment::mafia), initial_meeting);
	}

	void Game_log::log_boring_night() {
		_append_screen<Boring_night>();
	}

	void Game_log::log_investigation_result(core::Investigation investigation) {
		_append_screen<Investigation_result>(investigation);
	}

	void Game_log::log_game_ended() {
		_append_screen<Game_ended>();
	}

	void Game_log::try_to_log_night_ended() {
		if (_game.time() == core::Time::day) {
			auto date = _game.date();

			for (auto & inv: _game.investigations()) {
				if (inv.date + 1 == date) { // only show results from previous night
					log_investigation_result(inv);
				}
			}

			log_time_changed();

			if (_game.date() > 1) {
				log_obituary(_game.date() - 1);
			}

			if (_game.game_has_ended()) {
				log_game_ended();
			} else {
				log_town_meeting();
			}
		}
	}
}