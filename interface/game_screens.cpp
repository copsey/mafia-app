#include "../util/fstream.hpp"
#include "../util/misc.hpp"

#include "command.hpp"
#include "console.hpp"
#include "game_screens.hpp"
#include "names.hpp"

namespace maf {
	const core::Game & Game_screen::game() const {
		return console().game_log().game();
	}

	Game_log & Game_screen::game_log() {
		return console().game_log();
	}

	const Game_log & Game_screen::game_log() const {
		return console().game_log();
	}

	void Game_screen::do_commands(const CmdSequence & commands) {
		if (commands_match(commands, {"help"})) {
			console().show_help_screen<Game_help_screen>(*this);
		} else if (commands_match(commands, {"end"})) {
			console().show_question<Confirm_end_game>();
		} else {
			Screen::do_commands(commands);
		}
	}

	void Game_screen::summarise(string & output) const {
		auto path = application::root_dir();
		path /= "resources";
		path /= "txt";
		path /= "events";
		path /= this->id();
		path += ".txt";

		if (ifstream input{path}; input) {
			output += util::read_all(input);
		}
	}

	string Game_screen::escaped_name(const core::Player & player) const {
		return escaped(game_log().get_name(player));
	}

	string Game_screen::escaped_name(const core::Role & role) const {
		return escaped(full_name(role));
	}

	void Player_given_initial_role::do_commands(const CmdSequence & commands) {
		if (commands_match(commands, {"ok"})) {
			if (_is_private) game_log().advance();
			else _is_private = true;
		} else {
			Game_screen::do_commands(commands);
		}
	}

	void Player_given_initial_role::set_params(TextParams & params) const {
		params["player"] = escaped_name(*_player);
		params["private"] = _is_private;
		params["role"] = escaped_name(*_role);
		params["role.alias"] = escaped(_role->alias());
	}

	void Wildcards_resolved::do_commands(const CmdSequence & commands) {
		if (commands_match(commands, {"ok"})) {
			game_log().advance();
		} else {
			Game_screen::do_commands(commands);
		}
	}

	void maf::Wildcards_resolved::set_params(TextParams & params) const {
		std::map<not_null<const core::Role *>, int> cards;

		for (const core::Role & role : console().game_log().game().random_roles()) {
			if (cards.count(&role) > 0) {
				cards[&role] += 1;
			} else {
				cards[&role] = 1;
			}
		}

		params["cards"] = util::transform_into<vector<TextParams>>(cards, [&](auto& elem) -> TextParams {
			TextParams params{};
			params["role"] = escaped_name(*(elem.first));
			params["count"] = elem.second;
			return params;
		});
	}

	void maf::Time_changed::do_commands(const CmdSequence & commands) {
		if (commands_match(commands, {"ok"})) {
			game_log().advance();
		} else {
			Game_screen::do_commands(commands);
		}
	}

	void maf::Time_changed::set_params(TextParams & params) const {
		params["date"] = static_cast<int>(date);
		params["daytime"] = (time == core::Time::day);
		params["nighttime"] = (time == core::Time::night);
	}

	void maf::Obituary::do_commands(const CmdSequence & commands) {
		if (commands_match(commands, {"ok"})) {
			if (_deaths_index + 1 < _deaths.size()) {
				++_deaths_index;
			} else {
				game_log().advance();
			}
		} else {
			Game_screen::do_commands(commands);
		}
	}

	maf::TextParams maf::Obituary::_get_params(const core::Player & player) const {
		TextParams params;
		params["deceased"] = escaped_name(player);
		return params;
	}

	void maf::Obituary::set_params(TextParams& params) const {
		params["anyone_died"] = !_deaths.empty();
		params["num_deaths"] = static_cast<int>(_deaths.size());
		params["show_death"] = (_deaths_index >= 0);

		if (_deaths_index >= 0) {
			const core::Player & deceased = _deaths[_deaths_index];
			params["deceased"] = escaped_name(deceased);

			if (deceased.is_haunted()) {
				const core::Player & ghost = *deceased.haunter();
				params["deceased.is_haunted"] = true;
				params["ghost"] = escaped_name(ghost);
			} else {
				params["deceased.is_haunted"] = false;
			}
		}

		params["deaths"] = util::transformed_copy(_deaths, [&](const core::Player & player) {
			return this->_get_params(player);
		});
	}

	void maf::Town_meeting::do_commands(const CmdSequence & commands) {
		if (_lynch_can_occur) {
			_do_commands_before_lynch(commands);
		} else {
			_do_commands_after_lynch(commands);
		}
	}

	void maf::Town_meeting::_do_commands_before_lynch(const CmdSequence & commands) {
		if (commands_match(commands, {"", "vote", ""})) {
			auto& voter  = game_log().find_player(commands[0]);
			auto& target = game_log().find_player(commands[2]);
			game_log().cast_lynch_vote(voter.id(), target.id());
			game_log().advance();
		} else if (commands_match(commands, {"", "abstain"})) {
			auto& voter = game_log().find_player(commands[0]);
			game_log().clear_lynch_vote(voter.id());
			game_log().advance();
		} else if (commands_match(commands, {"lynch"})) {
			game_log().process_lynch_votes();
			game_log().advance();
		} else {
			_do_other_commands(commands);
		}
	}

	void maf::Town_meeting::_do_commands_after_lynch(const CmdSequence & commands) {
		if (commands_match(commands, {"night"})) {
			game_log().begin_night();
			game_log().advance();
		} else {
			_do_other_commands(commands);
		}
	}

	void maf::Town_meeting::_do_other_commands(const CmdSequence & commands) {
		if (commands_match(commands, {"kick", ""})) {
			auto& player = game_log().find_player(commands[1]);
			game_log().kick_player(player.id());
			game_log().advance();
		} else if (commands_match(commands, {"", "duel", ""})) {
			auto& caster = game_log().find_player(commands[0]);
			auto& target = game_log().find_player(commands[2]);
			game_log().stage_duel(caster.id(), target.id());
			game_log().advance();
		} else {
			Game_screen::do_commands(commands);
		}
	}

	maf::TextParams maf::Town_meeting::_get_params(const core::Player & player) const {
		TextParams params;

		params["player"] = escaped_name(player);

		if (_lynch_can_occur) {
			auto lynch_vote = player.lynch_vote();

			if (lynch_vote) {
				params["player.has_voted"] = true;
				params["player.vote"] = escaped_name(*lynch_vote);
			} else {
				params["player.has_voted"] = false;
			}
		}

		return params;
	}

	void maf::Town_meeting::set_params(TextParams & params) const {
		params["date"] = static_cast<int>(_date);
		params["lynch_can_occur"] = _lynch_can_occur;
		params["recent_vote"] = (_recent_vote_caster && _recent_vote_target);
		params["recent_abstain"] = (_recent_vote_caster && !_recent_vote_target);

		if (_lynch_can_occur) {
			if (_next_lynch_victim) {
				params["lynch_target.exists"] = true;
				params["lynch_target"] = escaped_name(*_next_lynch_victim);
			} else {
				params["lynch_target.exists"] = false;
			}
		}

		if (_recent_vote_caster) {
			if (_recent_vote_target) {
				params["recent_vote.caster"] = escaped_name(*_recent_vote_caster);
				params["recent_vote.target"] = escaped_name(*_recent_vote_target);
			} else {
				params["recent_abstain.caster"] = escaped_name(*_recent_vote_caster);
			}
		}

		params["townsfolk"] = util::transformed_copy(_players, [&](const core::Player & player) {
			return this->_get_params(player);
		});
	}

	void maf::Player_kicked::do_commands(const CmdSequence & commands) {
		if (commands_match(commands, {"ok"})) {
			game_log().advance();
		} else {
			Game_screen::do_commands(commands);
		}
	}

	void maf::Player_kicked::set_params(TextParams & params) const {
		params["player"] = escaped_name(_player);
		params["role"] = escaped_name(_player.role());
	}

	void maf::Lynch_result::do_commands(const CmdSequence & commands) {
		if (commands_match(commands, {"ok"})) {
			game_log().advance();
		} else {
			Game_screen::do_commands(commands);
		}
	}

	void maf::Lynch_result::set_params(TextParams & params) const {
		params["victim.exists"] = (victim != nullptr);

		if (victim) {
			params["victim"] = escaped_name(*victim);

			if (victim_role) {
				params["victim.role"] = escaped_name(*victim_role);
				params["victim.role.hidden"] = false;
				params["victim.role.is_troll"] = victim_role->is_troll();
			} else {
				params["victim.role.hidden"] = true;
			}
		}
	}

	void maf::Duel_result::do_commands(const CmdSequence & commands) {
		if (commands_match(commands, {"ok"})) {
			game_log().advance();
		} else {
			Game_screen::do_commands(commands);
		}
	}

	void maf::Duel_result::set_params(TextParams & params) const {
		params["caster"] = escaped_name(caster);
		params["caster.won_duel"] = (caster == target);
		params["target"] = escaped_name(target);
		params["winner"] = escaped_name(winner);
		params["loser"] = escaped_name(loser);
		params["winner.fled"] = !(winner.is_present());
	}

	void maf::Choose_fake_role::do_commands(const CmdSequence & commands) {
		if (_finished && commands_match(commands, {"ok"})) {
			game_log().advance();
		} else if (_fake_role && commands_match(commands, {"ok"})) {
			_finished = true;
		} else if (commands_match(commands, {"choose", ""})) {
			try {
				const core::Role & fake_role = game_log().look_up(commands[1]);
				game_log().choose_fake_role(_player->id(), fake_role.id());
				_fake_role = _player->fake_role();
			} catch (std::out_of_range) {
				throw core::Rulebook::Missing_role_alias{string{commands[1]}};
			}
		} else {
			Game_screen::do_commands(commands);
		}
	}

	void maf::Choose_fake_role::set_params(TextParams & params) const {
		params["finished"] = _finished;
		params["player"] = escaped_name(*_player);

		if (_fake_role) {
			params["fake_role"] = escaped_name(*_fake_role);
			params["fake_role.alias"] = escaped(_fake_role->alias());
			params["fake_role.chosen"] = true;
		} else {
			params["fake_role.chosen"] = false;
		}
	}

	void maf::Mafia_meeting::do_commands(const CmdSequence & commands) {
		if (_first_meeting) {
			_do_commands_for_first_meeting(commands);
		} else {
			_do_commands_for_regular_meeting(commands);
		}
	}

	void maf::Mafia_meeting::_do_commands_for_first_meeting(const CmdSequence & commands) {
		if (commands_match(commands, {"ok"})) {
			if (_finished) game_log().advance();
			else _finished = true;
		} else {
			Game_screen::do_commands(commands);
		}
	}

	void maf::Mafia_meeting::_do_commands_for_regular_meeting(const CmdSequence & commands) {
		if (_finished && commands_match(commands, {"ok"})) {
			game_log().advance();
		} else if (_mafiosi.size() == 1 && commands_match(commands, {"kill", ""})) {
			auto& caster = _mafiosi.front().get();
			auto& target = game_log().find_player(commands[1]);
			game_log().cast_mafia_kill(caster.id(), target.id());
			_finished = true;
		} else if (_mafiosi.size() > 1 && commands_match(commands, {"", "kill", ""})) {
			auto& caster = game_log().find_player(commands[0]);
			auto& target = game_log().find_player(commands[2]);
			game_log().cast_mafia_kill(caster.id(), target.id());
			_finished = true;
		} else if (commands_match(commands, {"skip"})) {
			// TODO: Show "confirm skip?" screen.
			game_log().skip_mafia_kill();
			_finished = true;
		} else {
			Game_screen::do_commands(commands);
		}
	}

	void maf::Mafia_meeting::set_params(TextParams& params) const {
		params["finished"] = _finished;
		params["first_meeting"] = _first_meeting;

		if (_mafiosi.size() == 1) {
			auto& player = _mafiosi.front().get();
			params["player"] = escaped_name(player);
			params["role"] = escaped_name(player.role());
			params["mafia.size"] = 1;
		} else {
			vector<TextParams> mafia;
			for (const core::Player & player: _mafiosi) {
				auto& subparams = mafia.emplace_back();
				subparams["player"] = escaped_name(player);
				subparams["role"] = escaped_name(player.role());
			}
			params["mafia.size"] = static_cast<int>(mafia.size());
			params["mafia"] = move(mafia);
		}
	}

	void maf::Kill_use::do_commands(const CmdSequence & commands) {
		if (_finished && commands_match(commands, {"ok"})) {
			game_log().advance();
		} else if (!_finished && commands_match(commands, {"kill", ""})) {
			auto& target = game_log().find_player(commands[1]);
			game_log().cast_kill(_caster.id(), target.id());
			_finished = true;
		} else if (!_finished && commands_match(commands, {"skip"})) {
			game_log().skip_kill(_caster.id());
			_finished = true;
		} else {
			Game_screen::do_commands(commands);
		}
	}

	void maf::Kill_use::set_params(TextParams& params) const {
		params["caster"] = escaped_name(_caster);
		params["finished"] = _finished;
	}

	void maf::Heal_use::do_commands(const CmdSequence & commands) {
		if (_finished && commands_match(commands, {"ok"})) {
			game_log().advance();
		} else if (!_finished && commands_match(commands, {"heal", ""})) {
			auto& target = game_log().find_player(commands[1]);
			game_log().cast_heal(_caster.id(), target.id());
			_finished = true;
		} else if (!_finished && commands_match(commands, {"skip"})) {
			game_log().skip_heal(_caster.id());
			_finished = true;
		} else {
			Game_screen::do_commands(commands);
		}
	}

	void maf::Heal_use::set_params(TextParams& params) const {
		params["caster"] = escaped_name(_caster);
		params["finished"] = _finished;
	}

	void maf::Investigate_use::do_commands(const CmdSequence & commands) {
		if (_finished && commands_match(commands, {"ok"})) {
			game_log().advance();
		} else if (!_finished && commands_match(commands, {"check", ""})) {
			auto& target = game_log().find_player(commands[1]);
			game_log().cast_investigate(_caster.id(), target.id());
			_finished = true;
		} else if (!_finished && commands_match(commands, {"skip"})) {
			game_log().skip_investigate(_caster.id());
			_finished = true;
		} else {
			Game_screen::do_commands(commands);
		}
	}

	void maf::Investigate_use::set_params(TextParams & params) const {
		params["caster"] = escaped_name(_caster);
		params["finished"] = _finished;
	}

	void maf::Peddle_use::do_commands(const CmdSequence & commands) {
		if (_finished && commands_match(commands, {"ok"})) {
			game_log().advance();
		} else if (!_finished && commands_match(commands, {"target", ""})) {
			auto& target = game_log().find_player(commands[1]);
			game_log().cast_peddle(_caster.id(), target.id());
			_finished = true;
		} else if (!_finished && commands_match(commands, {"skip"})) {
			game_log().skip_peddle(_caster.id());
			_finished = true;
		} else {
			Game_screen::do_commands(commands);
		}
	}

	void maf::Peddle_use::set_params(TextParams& params) const {
		params["caster"] = escaped_name(_caster);
		params["finished"] = _finished;
	}

	void maf::Boring_night::do_commands(const CmdSequence & commands) {
		if (commands_match(commands, {"ok"})) {
			game_log().advance();
		} else {
			Game_screen::do_commands(commands);
		}
	}

	void maf::Boring_night::set_params(TextParams& params) const {
		// Nothing to do
	}

	void maf::Investigation_result::do_commands(const CmdSequence & commands) {
		if (commands_match(commands, {"ok"})) {
			if (_finished) game_log().advance();
			else _finished = true;
		} else {
			Game_screen::do_commands(commands);
		}
	}

	void maf::Investigation_result::set_params(TextParams& params) const {
		params["caster"] = escaped_name(investigation.caster);
		params["finished"] = _finished;
		params["target"] = escaped_name(investigation.target);
		params["target.suspicious"] = investigation.result;
	}

	void maf::Game_ended::do_commands(const CmdSequence & commands) {
		if (commands_match(commands, {"end"})
			|| commands_match(commands, {"ok"}))
		{
			console().end_game();
		} else {
			Game_screen::do_commands(commands);
		}
	}

	void maf::Game_ended::set_params(TextParams& params) const {
		vector<TextParams> winners_params;
		vector<TextParams> losers_params;

		for (auto& player: game_log().players()) {
			TextParams subparams;
			subparams["player"] = escaped_name(player);
			subparams["role"] = escaped_name(player.role());

			if (player.has_won()) {
				winners_params.push_back(move(subparams));
			} else {
				losers_params.push_back(move(subparams));
			}
		}

		params["winners.size"] = static_cast<int>(winners_params.size());
		params["winners"] = move(winners_params);
		params["losers.size"] = static_cast<int>(losers_params.size());
		params["losers"] = move(losers_params);
	}
}
