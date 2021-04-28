#include <fstream>

#include "command.hpp"
#include "console.hpp"
#include "game_screens.hpp"
#include "names.hpp"

const maf::Game & maf::Game_screen::game() const {
	return console().game_log().game();
}

maf::Game_log & maf::Game_screen::game_log() {
	return console().game_log();
}

const maf::Game_log & maf::Game_screen::game_log() const {
	return console().game_log();
}

void maf::Game_screen::do_commands(const CmdSequence & commands) {
	if (commands_match(commands, {"help"})) {
		console().show_help_screen<Game_help_screen>(*this);
	} else if (commands_match(commands, {"end"})) {
		console().show_question<Confirm_end_game>();
	} else {
		Screen::do_commands(commands);
	}
}

void maf::Game_screen::summarise(string & output) const {
	// FIXME: This is horrendously fragile.
	string fname = "/Users/Jack/Documents/Developer/Projects/mafia/resources/";

	fname += "txt/events/";
	fname += this->id();
	fname += ".txt";

	auto input = std::ifstream{fname};

	if (input) {
		std::istreambuf_iterator<char> input_iter{input}, eos{};
		output.append(input_iter, eos);
	}
}

maf::string maf::Game_screen::escaped_name(Player const& player) const {
	return escaped(game_log().get_name(player));
}

maf::string maf::Game_screen::escaped_name(Role const& role) const {
	return escaped(full_name(role));
}

void maf::Player_given_initial_role::do_commands(const CmdSequence & commands) {
	if (commands_match(commands, {"ok"})) {
		if (_is_private) game_log().advance();
		else _is_private = true;
	} else {
		Game_screen::do_commands(commands);
	}
}

void maf::Player_given_initial_role::set_params(TextParams & params) const {
	params["from_wildcard"] = (_wildcard != nullptr);
	params["player"] = escaped_name(*_player);
	params["private"] = _is_private;
	params["role"] = escaped_name(*_role);
	params["role.alias"] = escaped(_role->alias());

	if (_wildcard != nullptr) {
		params["wildcard.alias"] = escaped(_wildcard->alias());
	}
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
	params["daytime"] = (time == Time::day);
	params["nighttime"] = (time == Time::night);
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

maf::TextParams maf::Obituary::_get_params(const Player & player) const {
	TextParams params;
	params["deceased"] = escaped_name(player);
	return params;
}

void maf::Obituary::set_params(TextParams& params) const {
	params["anyone_died"] = !_deaths.empty();
	params["num_deaths"] = static_cast<int>(_deaths.size());
	params["show_death"] = (_deaths_index >= 0);

	if (_deaths_index >= 0) {
		const Player& deceased = _deaths[_deaths_index];
		params["deceased"] = escaped_name(deceased);
		params["deceased.is_haunted"] = deceased.is_haunted();

		if (deceased.is_haunted()) {
			const Player& ghost = *deceased.haunter();
			params["ghost"] = escaped_name(ghost);
		}
	}

	params["deaths"] = util::transformed_copy(_deaths, [&](const Player & player) {
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
		const Player & voter  = game_log().find_player(commands[0]);
		const Player & target = game_log().find_player(commands[2]);
		game_log().cast_lynch_vote(voter.id(), target.id());
		game_log().advance();
	} else if (commands_match(commands, {"", "abstain"})) {
		const Player & voter = game_log().find_player(commands[0]);
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
		const Player & player = game_log().find_player(commands[1]);
		game_log().kick_player(player.id());
		game_log().advance();
	} else if (commands_match(commands, {"", "duel", ""})) {
		const Player & caster = game_log().find_player(commands[0]);
		const Player & target = game_log().find_player(commands[2]);
		game_log().stage_duel(caster.id(), target.id());
		game_log().advance();
	} else {
		Game_screen::do_commands(commands);
	}
}

maf::TextParams maf::Town_meeting::_get_params(Player const& player) const {
	TextParams params;

	params["player"] = escaped_name(player);

	if (_lynch_can_occur) {
		auto lynch_vote = player.lynch_vote();

		params["player.has_voted"] = (lynch_vote != nullptr);

		if (lynch_vote) {
			params["player.vote"] = escaped_name(*lynch_vote);
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
		params["lynch_target.exists"] = (_next_lynch_victim != nullptr);
		if (_next_lynch_victim) {
			params["lynch_target"] = escaped_name(*_next_lynch_victim);
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

	params["townsfolk"] = util::transformed_copy(_players, [&](Player const& player) {
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
		params["victim.role.hidden"] = (victim_role == nullptr);

		if (victim_role) {
			params["victim.role"] = escaped_name(*victim_role);
			params["victim.role.is_troll"] = victim_role->is_troll();
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
			const Role & fake_role = game_log().look_up(commands[1]);
			game_log().choose_fake_role(_player->id(), fake_role.id());
			_fake_role = _player->fake_role();
		} catch (std::out_of_range) {
			throw Rulebook::Missing_role_alias{string{commands[1]}};
		}
	} else {
		Game_screen::do_commands(commands);
	}
}

void maf::Choose_fake_role::set_params(TextParams & params) const {
	params["finished"] = _finished;
	params["player"] = escaped_name(*_player);
	params["fake_role.chosen"] = (_fake_role != nullptr);

	if (_fake_role) {
		params["fake_role"] = escaped_name(*_fake_role);
		params["fake_role.alias"] = escaped(_fake_role->alias());
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
		const Player & caster = _mafiosi.front();
		const Player & target = game_log().find_player(commands[1]);
		game_log().cast_mafia_kill(caster.id(), target.id());
		_finished = true;
	} else if (_mafiosi.size() > 1 && commands_match(commands, {"", "kill", ""})) {
		const Player & caster = game_log().find_player(commands[0]);
		const Player & target = game_log().find_player(commands[2]);
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
		const Player & player = _mafiosi.front();
		params["player"] = escaped_name(player);
		params["role"] = escaped_name(player.role());
		params["mafia.size"] = 1;
	} else {
		vector<TextParams> mafia;
		for (const Player & player: _mafiosi) {
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
	} else if (!_finished && commands_match(commands, {"check", ""})) {
		const Player & target = game_log().find_player(commands[1]);
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
		const Player & target = game_log().find_player(commands[1]);
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
		const Player & target = game_log().find_player(commands[1]);
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
		const Player & target = game_log().find_player(commands[1]);
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
