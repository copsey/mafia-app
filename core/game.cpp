#include <iterator>

#include "../util/algorithm.hpp"
#include "../util/misc.hpp"
#include "../util/random.hpp"

#include "game.hpp"

namespace maf::core {
	using WC = Win_condition;

	Game::Game(span<const Role::ID> role_ids,
		span<const Wildcard::ID> wildcard_ids,
		const Rulebook & rulebook)
	: _rulebook{rulebook} {
		auto append_to_random_roles = std::back_inserter(_random_roles);

		util::transform(wildcard_ids, append_to_random_roles, [&](Wildcard::ID id) -> const Role & {
			Wildcard & wildcard = _rulebook.get_wildcard(id);
			const Role & role = wildcard.pick_role(_rulebook);
			return role;
		});

		vector_of_refs<const Role> cards{};
		auto append_to_cards = std::back_inserter(cards);

		util::transform(role_ids, append_to_cards, [&](Role::ID id) -> const Role & {
			const Role & role = _rulebook.look_up(id);
			return role;
		});
		util::copy(_random_roles, append_to_cards);
		util::shuffle(cards);

		for (index i = 0; i < cards.size(); ++i) {
			const Role & role = cards[i];
			auto player = Player{i, role};
			_players.push_back(move(player));
		}

		try_to_end();
	}

	bool Game::contains(RoleRef r_ref) const {
		return rulebook().contains(r_ref);
	}

	const Role & Game::look_up(RoleRef r_ref) const {
		return rulebook().look_up(r_ref);
	}

	const vector_of_refs<const Role> & Game::random_roles() const {
		return _random_roles;
	}

	vector_of_refs<const Player> Game::remaining_players() const {
		return util::filtered_crefs(_players, [&](const Player & player) {
			return player.is_present();
		});
	}

	vector_of_refs<const Player> Game::remaining_players(Alignment alignment) const {
		return util::filtered_crefs(_players, [&](const Player & player) {
			return player.is_present() && player.alignment() == alignment;
		});
	}

	std::size_t Game::num_players_left() const {
		auto is_present = [](auto& player) { return player.is_present(); };
		return util::count_if(_players, is_present);
	}

	std::size_t Game::num_players_left(Alignment alignment) const {
		auto is_member_of_alignment = [alignment](auto& player) {
			return player.is_present() && player.alignment() == alignment;
		};

		return util::count_if(_players, is_member_of_alignment);
	}

	void Game::kick_player(Player::ID id) {
		using Reason = Kick_failed::Reason;

		Player& player = find_player(id);

		if (game_has_ended())
			throw Kick_failed{player, Reason::game_ended};
		if (!is_day())
			throw Kick_failed{player, Reason::bad_timing};
		if (player.has_been_kicked())
			throw Kick_failed{player, Reason::already_kicked};

		player.kick();
		try_to_end();
	}

	const Player* Game::next_lynch_victim() const {
		std::map<not_null<const Player *>, std::size_t> votes_per_player{};
		std::size_t total_votes = 0;

		for (const Player& voter : _players) {
			if (voter.is_present() && voter.has_lynch_vote()) {
				++votes_per_player[voter.lynch_vote()];
				++total_votes;
			}
		}

		auto less_votes = [](auto&& p1, auto&& p2) {
			return p1.second < p2.second;
		};

		auto it = util::max_element(votes_per_player, less_votes);

		if (it != votes_per_player.end() && (2 * it->second > total_votes)) {
			return it->first;
		} else {
			return nullptr;
		}
	}

	bool Game::lynch_can_occur() const {
		return _lynch_can_occur;
	}

	void Game::cast_lynch_vote(Player::ID voter_id, Player::ID target_id) {
		using Reason = Lynch_vote_failed::Reason;

		Player& voter = find_player(voter_id);
		Player& target = find_player(target_id);

		if (game_has_ended())
			throw Lynch_vote_failed{voter, &target, Reason::game_ended};
		if (!lynch_can_occur())
			throw Lynch_vote_failed{voter, &target, Reason::bad_timing};
		if (!voter.is_present())
			throw Lynch_vote_failed{voter, &target, Reason::voter_is_not_present};
		if (!target.is_present())
			throw Lynch_vote_failed{voter, &target, Reason::target_is_not_present};
		if (voter == target)
			throw Lynch_vote_failed{voter, &target, Reason::voter_is_target};

		voter.cast_lynch_vote(target);
	}

	void Game::clear_lynch_vote(Player::ID voter_id) {
		using Reason = Lynch_vote_failed::Reason;

		Player& voter = find_player(voter_id);

		if (game_has_ended())
			throw Lynch_vote_failed{voter, nullptr, Reason::game_ended};
		if (!lynch_can_occur())
			throw Lynch_vote_failed{voter, nullptr, Reason::bad_timing};
		if (!voter.is_present())
			throw Lynch_vote_failed{voter, nullptr, Reason::voter_is_not_present};

		voter.clear_lynch_vote();
	}

	const Player * Game::process_lynch_votes() {
		using Reason = Lynch_failed::Reason;

		if (game_has_ended())
			throw Lynch_failed{Reason::game_ended};
		if (!lynch_can_occur())
			throw Lynch_failed{Reason::bad_timing};

		auto victim = const_cast<Player*>(next_lynch_victim());
		if (victim) {
			victim->kill(_date, _time);
			if (victim->is_troll()) _pending_haunters.push_back(victim);
		}

		_lynch_can_occur = false;

		try_to_end();

		return victim;
	}

	void Game::stage_duel(Player::ID caster_id, Player::ID target_id) {
		using Reason = Duel_failed::Reason;

		Player& caster = find_player(caster_id);
		Player& target = find_player(target_id);

		if (game_has_ended())
			throw Duel_failed(caster, target, Reason::game_ended);
		if (!is_day())
			throw Duel_failed(caster, target, Reason::bad_timing);
		if (!caster.is_present())
			throw Duel_failed(caster, target, Reason::caster_is_not_present);
		if (!target.is_present())
			throw Duel_failed(caster, target, Reason::target_is_not_present);
		if (caster == target)
			throw Duel_failed(caster, target, Reason::caster_is_target);
		if (!caster.role().has_ability() || caster.role().ability().id != Ability::ID::duel)
			throw Duel_failed(caster, target, Reason::caster_has_no_duel);

		double sum = caster.duel_strength() + target.duel_strength();
		if (sum <= 0.0)
			throw Duel_failed(caster, target, Reason::bad_probability);

		double p = caster.duel_strength() / sum;
		std::bernoulli_distribution bd{p};

		Player *winner, *loser;

		if (bd(util::random_engine)) {
			winner = &caster;
			loser = &target;
		} else {
			winner = &target;
			loser = &caster;
		}

		winner->win_duel();
		if (winner->win_condition() == WC::win_duel) {
			winner->leave();
		}
		loser->kill(_date, _time);

		try_to_end();
	}

	void Game::begin_night() {
		using Reason = Begin_night_failed::Reason;

		if (game_has_ended())
			throw Begin_night_failed{Reason::game_ended};
		if (is_night())
			throw Begin_night_failed{Reason::already_night};
		if (lynch_can_occur())
			throw Begin_night_failed{Reason::lynch_can_occur};

		_time = Time::night;

		if (_date > 0) {
			_mafia_can_use_kill = (num_players_left(Alignment::mafia) > 0);

			for (Player& player: _players) {
				if (player.is_present() && player.role().has_ability()) {
					Ability ability = player.role().ability();

					switch (ability.id) {
					using ID = Ability::ID;
					case ID::kill:
					case ID::heal:
					case ID::investigate:
					case ID::peddle:
						player.add_compulsory_ability(ability);
						break;
					case ID::duel:
						break;
					}
				}
			}
		}

		try_to_end_night();
	}

	void Game::choose_fake_role(Player::ID player_id, Role::ID fake_role_id) {
		using Reason = Choose_fake_role_failed::Reason;

		Player& player = find_player(player_id);
		const Role& fake_role = _rulebook.look_up(fake_role_id);

		if (game_has_ended())
			throw Choose_fake_role_failed{player, fake_role, Reason::game_ended};
		if (!is_night())
			throw Choose_fake_role_failed{player, fake_role, Reason::bad_timing};
		if (!player.is_role_faker())
			throw Choose_fake_role_failed{player, fake_role, Reason::player_is_not_faker};
		if (player.has_fake_role())
			throw Choose_fake_role_failed{player, fake_role, Reason::already_chosen};

		player.give_fake_role(fake_role);

		try_to_end_night();
	}

	void Game::cast_mafia_kill(Player::ID caster_id, Player::ID target_id) {
		using Reason = Mafia_kill_failed::Reason;

		Player& caster = find_player(caster_id);
		Player& target = find_player(target_id);

		if (game_has_ended())
			throw Mafia_kill_failed{caster, target, Reason::game_ended};
		if (!is_night())
			throw Mafia_kill_failed{caster, target, Reason::bad_timing};
		if (!mafia_can_use_kill())
			throw Mafia_kill_failed{caster, target, Reason::already_used};
		if (!caster.is_present())
			throw Mafia_kill_failed{caster, target, Reason::caster_is_not_present};
		if (caster.alignment() != Alignment::mafia)
			throw Mafia_kill_failed{caster, target, Reason::caster_is_not_in_mafia};
		if (!target.is_present())
			throw Mafia_kill_failed{caster, target, Reason::target_is_not_present};
		if (caster == target)
			throw Mafia_kill_failed{caster, target, Reason::caster_is_target};

		_mafia_can_use_kill = false;
		_mafia_kill_caster = &caster;
		_mafia_kill_target = &target;

		try_to_end_night();
	}

	void Game::skip_mafia_kill() {
		if (game_has_ended())
			throw Skip_failed{};
		if (!is_night())
			throw Skip_failed{};
		if (!mafia_can_use_kill())
			throw Skip_failed{};

		_mafia_can_use_kill = false;

		try_to_end_night();
	}

	void Game::cast_kill(Player::ID caster_id, Player::ID target_id) {
		using Reason = Kill_failed::Reason;

		auto is_kill = [](const Ability& abl) {
			return abl.id == Ability::ID::kill;
		};

		Player& caster = find_player(caster_id);
		Player& target = find_player(target_id);

		if (game_has_ended())
			throw Kill_failed{caster, target, Reason::game_ended};
		if (util::none_of(caster.compulsory_abilities(), is_kill))
			throw Kill_failed{caster, target, Reason::caster_cannot_kill};
		if (!target.is_present())
			throw Kill_failed{caster, target, Reason::target_is_not_present};
		if (caster == target)
			throw Kill_failed{caster, target, Reason::caster_is_target};

		_pending_kills.emplace_back(&caster, &target);
		caster.remove_compulsory_ability(Ability{Ability::ID::kill});

		try_to_end_night();
	}

	void Game::skip_kill(Player::ID caster_id) {
		auto is_kill = [](const Ability & abl) {
			return abl.id == Ability::ID::kill;
		};

		Player& caster = find_player(caster_id);

		if (game_has_ended())
			throw Skip_failed{};
		if (util::none_of(caster.compulsory_abilities(), is_kill))
			throw Skip_failed{};

		caster.remove_compulsory_ability(Ability{Ability::ID::kill});

		try_to_end_night();
	}

	void Game::cast_heal(Player::ID caster_id, Player::ID target_id) {
		using Reason = Heal_failed::Reason;

		auto is_heal = [](const Ability & abl) {
			return abl.id == Ability::ID::heal;
		};

		Player& caster = find_player(caster_id);
		Player& target = find_player(target_id);

		if (game_has_ended())
			throw Heal_failed{caster, target, Reason::game_ended};
		if (util::none_of(caster.compulsory_abilities(), is_heal))
			throw Heal_failed{caster, target, Reason::caster_cannot_heal};
		if (!target.is_present())
			throw Heal_failed{caster, target, Reason::target_is_not_present};
		if (caster == target)
			throw Heal_failed{caster, target, Reason::caster_is_target};

		_pending_heals.emplace_back(&caster, &target);
		caster.remove_compulsory_ability(Ability{Ability::ID::heal});

		try_to_end_night();
	}

	void Game::skip_heal(Player::ID caster_id) {
		auto is_heal = [](const Ability & abl) {
			return abl.id == Ability::ID::heal;
		};

		Player& caster = find_player(caster_id);

		if (game_has_ended())
			throw Skip_failed{};
		if (util::none_of(caster.compulsory_abilities(), is_heal))
			throw Skip_failed{};

		caster.remove_compulsory_ability(Ability{Ability::ID::heal});

		try_to_end_night();
	}

	void Game::cast_investigate(Player::ID caster_id, Player::ID target_id) {
		using Reason = Investigate_failed::Reason;

		auto is_investigate = [](const Ability & abl) {
			return abl.id == Ability::ID::investigate;
		};

		Player& caster = find_player(caster_id);
		Player& target = find_player(target_id);

		if (game_has_ended())
			throw Investigate_failed{caster, target, Reason::game_ended};
		if (util::none_of(caster.compulsory_abilities(), is_investigate))
			throw Investigate_failed{caster, target, Reason::caster_cannot_investigate};
		if (!target.is_present())
			throw Investigate_failed{caster, target, Reason::target_is_not_present};
		if (caster == target)
			throw Investigate_failed{caster, target, Reason::caster_is_target};

		_pending_investigations.emplace_back(&caster, &target);
		caster.remove_compulsory_ability(Ability{Ability::ID::investigate});

		try_to_end_night();
	}

	void Game::skip_investigate(Player::ID caster_id) {
		auto is_investigate = [](const Ability & abl) {
			return abl.id == Ability::ID::investigate;
		};

		Player& caster = find_player(caster_id);

		if (game_has_ended())
			throw Skip_failed{};
		if (util::none_of(caster.compulsory_abilities(), is_investigate))
			throw Skip_failed{};

		caster.remove_compulsory_ability(Ability{Ability::ID::investigate});

		try_to_end_night();
	}

	void Game::cast_peddle(Player::ID caster_id, Player::ID target_id) {
		using Reason = Peddle_failed::Reason;

		auto is_peddle = [](const Ability & abl) {
			return abl.id == Ability::ID::peddle;
		};

		Player& caster = find_player(caster_id);
		Player& target = find_player(target_id);

		if (game_has_ended())
			throw Peddle_failed{caster, target, Reason::game_ended};
		if (util::none_of(caster.compulsory_abilities(), is_peddle))
			throw Peddle_failed{caster, target, Reason::caster_cannot_peddle};
		if (!target.is_present())
			throw Peddle_failed{caster, target, Reason::target_is_not_present};

		_pending_peddles.emplace_back(&caster, &target);
		caster.remove_compulsory_ability(Ability{Ability::ID::peddle});

		try_to_end_night();
	}

	void Game::skip_peddle(Player::ID caster_id) {
		auto is_peddle = [](const Ability & abl) {
			return abl.id == Ability::ID::peddle;
		};

		Player& caster = find_player(caster_id);

		if (game_has_ended())
			throw Skip_failed{};

		if (util::none_of(caster.compulsory_abilities(), is_peddle))
			throw Skip_failed{};

		caster.remove_compulsory_ability(Ability{Ability::ID::peddle});

		try_to_end_night();
	}

	Player & Game::find_player(Player::ID id) {
		if (id < _players.size()) {
			return _players[id];
		} else {
			throw Player_not_found{id};
		}
	}

	bool Game::try_to_end_night() {
		if (!is_night()) {
			return false;
		}

		auto faker_needs_role = [](const Player & pl) {
			return pl.is_present()
				&& pl.role().is_role_faker()
				&& !pl.has_fake_role();
		};

		if (util::any_of(_players, faker_needs_role)) {
			return false;
		}

		auto has_compulsory_ability = [](const Player& pl) {
			return pl.compulsory_abilities().size() > 0;
		};

		if (mafia_can_use_kill()) return false;
		if (util::any_of(players(), has_compulsory_ability)) return false;

		for (const auto &pair: _pending_heals) {
			Player &target = *pair.second;
			target.heal();
		}

		for (const auto &pair: _pending_peddles) {
			Player& target = *pair.second;
			target.give_drugs();
		}

		if (_mafia_kill_caster) {
			if (!_mafia_kill_target->is_healed()) {
				_mafia_kill_target->kill(_date, _time);
			}
		}

		// FIXME: make kill strengths work correctly.
		for (const auto &pair: _pending_kills) {
			Player &target = *pair.second;
			if (!target.is_healed()) {
				target.kill(_date, _time);
			}
		}

		for (auto& pair: _pending_investigations) {
			Player& caster = *pair.first;
			Player& target = *pair.second;

			if (caster.is_present()) {
				_investigations.emplace_back(caster, target, _date, target.is_suspicious());
			}
		}

		for (Player* haunter: _pending_haunters) {
			vector<Player*> possible_victims{};
			for (Player& player: _players) {
				if (player.is_present() && player.lynch_vote() == haunter)   {
					possible_victims.push_back(&player);
				}
			}

			if (possible_victims.size() > 0) {
				Player& victim = **util::pick(possible_victims);
				victim.kill(_date, _time);
				victim.haunt(*haunter);
			}
		}

		++_date;
		_time = Time::day;

		if (!try_to_end()) {
			_lynch_can_occur = true;

			_mafia_kill_caster = nullptr;
			_mafia_kill_target = nullptr;

			_pending_kills.clear();
			_pending_heals.clear();
			_pending_investigations.clear();
			_pending_peddles.clear();

			_pending_haunters.clear();

			for (Player& player: _players) player.refresh();
		}

		return true;
	}

	bool Game::try_to_end() {
		if (_has_ended) return true;

		std::size_t num_players_left = 0;
		std::size_t num_village_left = 0;
		std::size_t num_mafia_left = 0;

		bool check_for_village_eliminated = false;
		bool check_for_mafia_eliminated = false;
		bool check_for_last_survivor = false;

		for (const Player& player : _players) {
			if (player.is_present()) {
				++num_players_left;
				switch (player.alignment()) {
				case Alignment::village:
					++num_village_left;
					break;
				case Alignment::mafia:
					++num_mafia_left;
					break;
				default:
					break;
				}

				switch (player.peace_condition()) {
				case Peace_condition::always_peaceful:
					break;
				case Peace_condition::village_eliminated:
					check_for_village_eliminated = true;
					break;
				case Peace_condition::mafia_eliminated:
					check_for_mafia_eliminated = true;
					break;
				case Peace_condition::last_survivor:
					check_for_last_survivor = true;
					break;
				}
			}
		}

		if ((check_for_village_eliminated && num_village_left > 0)
		     || (check_for_mafia_eliminated && num_mafia_left > 0)
		     || (check_for_last_survivor && num_players_left > 1)) {
			return false;
		}

		for (Player& player: _players) {
			bool has_won = false;

			if (!player.has_been_kicked()) {
				switch (player.win_condition()) {
				case Win_condition::survive:
					has_won = player.is_alive();
					break;
				case Win_condition::village_remains:
					has_won = (num_village_left > 0);
					break;
				case Win_condition::mafia_remains:
					has_won = (num_mafia_left > 0);
					break;
				case Win_condition::be_lynched:
					has_won = player.has_been_lynched();
					break;
				case Win_condition::win_duel:
					has_won = player.has_won_duel();
					break;
				}
			}

			if (has_won) player.win(); else player.lose();
		}

		_has_ended = true;
		return true;
	}
}
