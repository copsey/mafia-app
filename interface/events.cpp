#include <fstream>

#include "command.hpp"
#include "console.hpp"
#include "events.hpp"
#include "names.hpp"
#include "../common/stdlib.h"

void maf::Event::summarise(ostream &os) const {
	// By default, write nothing.
}

void maf::Event::write(std::ostream & output) const {
	// FIXME: This is horrendously fragile.
	std::string fname = "/Users/Jack/Documents/Developer/Projects/mafia/resources/txt/events/";
	fname += this->id();
	fname += ".txt";

	auto input = std::ifstream{fname};
	if (input) {
		output << input.rdbuf();
	} else {
		output << "=Error=\n\nERROR: No text found for the \"";
		output << this->id();
		output << "\" event.";
	}
}

std::string maf::Event::escaped_name(Player const& player) const {
	return escaped(game_log().get_name(player));
}

std::string maf::Event::escaped_name(Role const& role) const {
	return escaped(full_name(role));
}

void maf::Player_given_initial_role::do_commands(const vector<string_view> & commands) {
	if (commands_match(commands, {"ok"})) {
		if (_is_private) {
			game_log().advance();
		} else {
			_is_private = true;
		}
	}
	else {
		throw Bad_commands{};
	}
}

void maf::Player_given_initial_role::set_params(TextParams& params) const {
	params["from_wildcard"] = (_w != nullptr);
	params["player"] = escaped(game_log().get_name(*_p));
	params["private"] = _is_private;
	params["role"] = escaped(full_name(*_r));
	params["role.alias"] = escaped(_r->alias());

	if (_w != nullptr) {
		params["wildcard.alias"] = escaped(_w->alias());
	}
}

void maf::Player_given_initial_role::summarise(ostream &os) const {
	os << game_log().get_name(*_p) << " played as the " << full_name(*_r) << ".";
}

void maf::Time_changed::do_commands(const vector<string_view> & commands) {
	if (commands_match(commands, {"ok"})) {
		game_log().advance();
	}
	else {
		throw Bad_commands{};
	}
}

void maf::Time_changed::set_params(TextParams& params) const {
	params["date"] = std::to_string(date);
	params["daytime"] = (time == Time::day);
	params["nighttime"] = (time == Time::night);
}

void maf::Time_changed::summarise(ostream &os) const {
	switch (time) {
		case Time::day:
			os << "Day " << date << " began.";
			break;
		case Time::night:
			os << "Night " << date << " began.";
			break;
	}
}

void maf::Obituary::do_commands(const vector<string_view> & commands) {
	if (commands_match(commands, {"ok"})) {
		if (_deaths_index + 1 < _deaths.size()) {
			++_deaths_index;
		} else {
			game_log().advance();
		}
	} else {
		throw Bad_commands{};
	}
}

void maf::Obituary::set_params(TextParams& params) const {
	params["anyone_died"] = !_deaths.empty();
	params["num_deaths"] = std::to_string(_deaths.size());
	params["show_death"] = (_deaths_index >= 0);
	
	if (_deaths_index >= 0) {
		const Player& deceased = *_deaths[_deaths_index];
		params["deceased"] = escaped(game_log().get_name(deceased));
		params["deceased.is_haunted"] = deceased.is_haunted();
		
		if (deceased.is_haunted()) {
			const Player& ghost = *deceased.haunter();
			params["ghost"] = escaped(game_log().get_name(ghost));
		}
	}
}

void maf::Obituary::summarise(ostream &os) const {
	bool write_nl = false;

	for (auto& p_ref: _deaths) {
		if (write_nl) os << '\n';
		os << game_log().get_name(*p_ref) << " died during the night.";
		write_nl = true;
	}
}

void maf::Town_meeting::do_commands(const vector<string_view> & commands) {
	auto & glog = game_log();


	if (_lynch_can_occur) {
		if (commands_match(commands, {"kick", ""})) {
			auto & pl_name = commands[1];
			auto & pl = glog.find_player(pl_name);

			glog.kick_player(pl.id());
			glog.advance();
		}
		else if (commands_match(commands, {"", "vote", ""})) {
			auto & voter = glog.find_player(commands[0]);
			auto & target = glog.find_player(commands[2]);

			glog.cast_lynch_vote(voter.id(), target.id());
			glog.advance();
		}
		else if (commands_match(commands, {"", "abstain"})) {
			auto & voter = glog.find_player(commands[0]);

			glog.clear_lynch_vote(voter.id());
			glog.advance();
		}
		else if (commands_match(commands, {"lynch"})) {
			glog.process_lynch_votes();
			glog.advance();
		}
		else if (commands_match(commands, {"", "duel", ""})) {
			auto & caster = glog.find_player(commands[0]);
			auto & target = glog.find_player(commands[2]);

			glog.stage_duel(caster.id(), target.id());
			glog.advance();
		}
		else {
			throw Bad_commands{};
		}
	} else {
		if (commands_match(commands, {"night"})) {
			glog.begin_night();
			glog.advance();
		}
		else if (commands_match(commands, {"kick", ""})) {
			const Player & pl = glog.find_player(commands[1]);

			glog.kick_player(pl.id());
			glog.advance();
		}
		else if (commands_match(commands, {"", "duel", ""})) {
			const Player & caster = glog.find_player(commands[0]);
			const Player & target = glog.find_player(commands[2]);

			glog.stage_duel(caster.id(), target.id());
			glog.advance();
		}
		else {
			throw Bad_commands{};
		}
	}
}

void maf::Town_meeting::set_params(TextParams & params) const {
	params["date"] = std::to_string(_date);
	params["lynch_can_occur"] = _lynch_can_occur;

	if (_lynch_can_occur) {
		params["lynch_target.exists"] = (_next_lynch_victim != nullptr);
		if (_next_lynch_victim) {
			params["lynch_target"] = escaped(game_log().get_name(*_next_lynch_victim));
		}
	}

	auto townsfolk = std::vector<TextParams>();

	for (auto p_ref: _players) {
		auto& subparams = townsfolk.emplace_back();

		subparams["player"] = escaped_name(*p_ref);
		if (_lynch_can_occur) {
			subparams["player.has_voted"] = (p_ref->lynch_vote() != nullptr);
			if (p_ref->lynch_vote() != nullptr) {
				subparams["player.vote"] = escaped_name(*(p_ref->lynch_vote()));
			}
		}
	}

	params["townsfolk"] = std::move(townsfolk);
}

void maf::Town_meeting::summarise(ostream &os) const {
	if (_recent_vote_caster) {
		if (_recent_vote_target) {
			os << game_log().get_name(*_recent_vote_caster) << " voted to lynch " << game_log().get_name(*_recent_vote_target) << ".";
		} else {
			os << game_log().get_name(*_recent_vote_caster) << " chose not to vote.";
		}
	}
}

void maf::Player_kicked::do_commands(const vector<string_view> &commands) {
	if (commands_match(commands, {"ok"})) {
		game_log().advance();
	}
	else {
		throw Bad_commands{};
	}
}

void maf::Player_kicked::set_params(TextParams & params) const {
	params["player"] = escaped(game_log().get_name(*player));
	params["role"] = escaped(full_name(player->role()));
}

void maf::Player_kicked::summarise(ostream &os) const {
	os << game_log().get_name(*player) << " was kicked.";
}

void maf::Lynch_result::do_commands(const vector<string_view> &commands) {
	if (commands_match(commands, {"ok"})) {
		game_log().advance();
	}
	else {
		throw Bad_commands{};
	}
}

void maf::Lynch_result::set_params(TextParams & params) const {
	params["victim.exists"] = (victim != nullptr);
	
	if (victim) {
		params["victim"] = escaped(game_log().get_name(*victim));
		params["victim.role.hidden"] = (victim_role == nullptr);
		
		if (victim_role) {
			params["victim.role"] = escaped(full_name(victim_role->id()));
			params["victim.role.is_troll"] = victim_role->is_troll();
		}
	}
}

void maf::Lynch_result::summarise(ostream &os) const {
	if (victim) {
		os << game_log().get_name(*victim) << " was lynched.";
	}
	else {
		os << "Nobody was lynched.";
	}
}

void maf::Duel_result::do_commands(const vector<string_view> & commands) {
	if (commands_match(commands, {"ok"})) {
		game_log().advance();
	}
	else {
		throw Bad_commands{};
	}
}

void maf::Duel_result::set_params(TextParams & params) const {
	params["caster"] = escaped(game_log().get_name(*caster));
	params["target"] = escaped(game_log().get_name(*target));
	params["winner"] = escaped(game_log().get_name(*winner));
	params["loser"] = escaped(game_log().get_name(*loser));
	params["winner.fled"] = !winner->is_present();
}

void maf::Duel_result::summarise(ostream &os) const {
	auto & glog = game_log();
	
	auto caster_name = glog.get_name(*caster);
	auto target_name = glog.get_name(*target);

	if (winner == caster) {
		os << caster_name << " won a duel against " << target_name << ".";
	} else {
		os << caster_name << " lost a duel against " << target_name << ".";
	}
}

void maf::Choose_fake_role::do_commands(const vector<string_view> & commands) {
	auto & glog = game_log();

	if (_go_to_sleep) {
		if (commands_match(commands, {"ok"})) {
			glog.advance();
		} else {
			throw Bad_commands{};
		}
	} else if (_fake_role) {
		if (commands_match(commands, {"ok"})) {
			_go_to_sleep = true;
		} else {
			throw Bad_commands{};
		}
	} else {
		if (commands_match(commands, {"choose", ""})) {
			RoleRef fake_role_ref = commands[1];

			try {
				auto & fake_role = glog.look_up(fake_role_ref);
				glog.choose_fake_role(_player->id(), fake_role.id());
				_fake_role = _player->fake_role();
			} catch (std::out_of_range) {
				throw Rulebook::Missing_role_alias{string(commands[2])};
			}
		} else {
			throw Bad_commands{};
		}
	}
}

void maf::Choose_fake_role::set_params(TextParams & params) const {
	params["finished"] = _go_to_sleep;
	params["player"] = escaped(game_log().get_name(*_player));
	params["fake_role.chosen"] = (_fake_role != nullptr);

	if (_fake_role) {
		params["fake_role"] = escaped_name(*_fake_role);
		params["fake_role.alias"] = escaped(_fake_role->alias());
	}
}

void maf::Choose_fake_role::summarise(ostream &os) const {
	if (_fake_role) {
		os << game_log().get_name(*_player) << " was given the " << full_name(*_fake_role) << " as a fake role.";
	}
}

void maf::Mafia_meeting::do_commands(const vector<string_view> & commands) {
	auto & glog = game_log();

	if (_go_to_sleep) {
		if (commands_match(commands, {"ok"})) {
			glog.advance();
		} else {
			throw Bad_commands{};
		}
	} else if (_initial) {
		if (commands_match(commands, {"ok"})) {
			_go_to_sleep = true;
		} else {
			throw Bad_commands{};
		}
	} else {
		if (commands_match(commands, {"kill", ""}) && _mafiosi.size() == 1) {
			auto & caster = *(_mafiosi.front());
			auto & target = glog.find_player(commands[1]);

			glog.cast_mafia_kill(caster.id(), target.id());
			_go_to_sleep = true;
		} else if (commands_match(commands, {"", "kill", ""})) {
			auto & caster = glog.find_player(commands[0]);
			auto & target = glog.find_player(commands[2]);

			glog.cast_mafia_kill(caster.id(), target.id());
			_go_to_sleep = true;
		} else if (commands_match(commands, {"skip"})) {
			/* FIXME: show "confirm skip?" screen. */
			glog.skip_mafia_kill();
			_go_to_sleep = true;
		} else {
			throw Bad_commands{};
		}
	}
}

void maf::Mafia_meeting::set_params(TextParams& params) const {
	params["finished"] = _go_to_sleep;
	params["first_meeting"] = _initial;
	params["single_member"] = (_mafiosi.size() == 1);

	if (_mafiosi.size() == 1) {
		auto& player = *_mafiosi.front();
		params["player"] = escaped_name(player);
		params["role"] = escaped_name(player.role());
	} else {
		std::vector<TextParams> mafia;
		for (auto p_ref: _mafiosi) {
			auto& player = *p_ref;
			auto& subparams = mafia.emplace_back();
			subparams["player"] = escaped_name(player);
			subparams["role"] = escaped_name(player.role());
		}
		params["mafia"] = std::move(mafia);
	}
}

void maf::Kill_use::do_commands(const vector<string_view> &commands) {
	auto & glog = game_log();

	if (_go_to_sleep) {
		if (commands_match(commands, {"ok"})) {
			glog.advance();
		} else {
			throw Bad_commands{};
		}
	} else {
		if (commands_match(commands, {"kill", ""})) {
			auto & target_name = commands[1];
			auto & target = glog.find_player(target_name);

			glog.cast_kill(_caster->id(), target.id());
			_go_to_sleep = true;
		} else if (commands_match(commands, {"skip"})) {
			glog.skip_kill(_caster->id());
			_go_to_sleep = true;
		} else {
			throw Bad_commands{};
		}
	}
}

void maf::Kill_use::set_params(TextParams& params) const {
	params["caster"] = escaped_name(*_caster);
	params["finished"] = _go_to_sleep;
}

void maf::Heal_use::do_commands(const vector<string_view> & commands) {
	auto & glog = game_log();

	if (_go_to_sleep) {
		if (commands_match(commands, {"ok"})) {
			glog.advance();
		} else {
			throw Bad_commands{};
		}
	} else {
		if (commands_match(commands, {"heal", ""})) {
			auto & target_name = commands[1];
			auto & target = glog.find_player(target_name);

			glog.cast_heal(_caster->id(), target.id());
			_go_to_sleep = true;
		} else if (commands_match(commands, {"skip"})) {
			glog.skip_heal(_caster->id());
			_go_to_sleep = true;
		} else {
			throw Bad_commands{};
		}
	}
}

void maf::Heal_use::set_params(TextParams& params) const {
	params["caster"] = escaped_name(*_caster);
	params["finished"] = _go_to_sleep;
}

void maf::Investigate_use::do_commands(const vector<string_view> & commands) {
	auto & glog = game_log();

	if (_go_to_sleep) {
		if (commands_match(commands, {"ok"})) {
			glog.advance();
		} else {
			throw Bad_commands{};
		}
	} else {
		if (commands_match(commands, {"check", ""})) {
			auto & target_name = commands[1];
			auto & target = glog.find_player(target_name);

			glog.cast_investigate(_caster->id(), target.id());
			_go_to_sleep = true;
		} else if (commands_match(commands, {"skip"})) {
			glog.skip_investigate(_caster->id());
			_go_to_sleep = true;
		} else {
			throw Bad_commands{};
		}
	}
}

void maf::Investigate_use::set_params(TextParams& params) const {
	params["caster"] = escaped_name(*_caster);
	params["finished"] = _go_to_sleep;
}

void maf::Peddle_use::do_commands(const vector<string_view> & commands) {
	auto & glog = game_log();

	if (_go_to_sleep) {
		if (commands_match(commands, {"ok"})) {
			glog.advance();
		} else {
			throw Bad_commands{};
		}
	} else {
		if (commands_match(commands, {"target", ""})) {
			auto & target_name = commands[1];
			auto & target = glog.find_player(target_name);

			glog.cast_peddle(_caster->id(), target.id());
			_go_to_sleep = true;
		} else if (commands_match(commands, {"skip"})) {
			glog.skip_peddle(_caster->id());
			_go_to_sleep = true;
		} else {
			throw Bad_commands{};
		}
	}
}

void maf::Peddle_use::set_params(TextParams& params) const {
	params["caster"] = escaped_name(*_caster);
	params["finished"] = _go_to_sleep;
}

void maf::Boring_night::do_commands(const vector<string_view> & commands) {
	if (commands_match(commands, {"ok"})) {
		game_log().advance();
	}
	else {
		throw Bad_commands{};
	}
}

void maf::Boring_night::set_params(TextParams& params) const
{ }

void maf::Investigation_result::do_commands(const vector<string_view> & commands) {
	auto & glog = game_log();

	if (_go_to_sleep) {
		if (commands_match(commands, {"ok"})) {
			glog.advance();
		}
		else {
			throw Bad_commands{};
		}
	} else {
		if (commands_match(commands, {"ok"})) {
			_go_to_sleep = true;
		}
		else {
			throw Bad_commands{};
		}
	}
}

void maf::Investigation_result::set_params(TextParams& params) const {
	params["caster"] = escaped(game_log().get_name(investigation.caster()));
	params["finished"] = _go_to_sleep;
	params["target"] = escaped(game_log().get_name(investigation.target()));
	params["target.suspicious"] = investigation.result();
}

void maf::Investigation_result::summarise(ostream &os) const {
	os << game_log().get_name(investigation.caster())
	<< " decided that "
	<< game_log().get_name(investigation.target())
	<< " was "
	<< (investigation.result() ? "suspicious" : "innocent")
	<< ".";
}

void maf::Game_ended::do_commands(const vector<string_view> & commands) {
	throw Bad_commands{};
}

void maf::Game_ended::set_params(TextParams& params) const {
	std::vector<TextParams> winners;
	std::vector<TextParams> losers;

	for (auto& player: game_log().game().players()) {
		TextParams subparams;
		subparams["player"] = escaped_name(player);
		subparams["role"] = escaped_name(player.role());

		if (player.has_won()) {
			winners.push_back(std::move(subparams));
		} else {
			losers.push_back(std::move(subparams));
		}
	}

	params["any_winners"] = !winners.empty();
	params["any_losers"] = !losers.empty();

	if (!winners.empty()) params["winners"] = std::move(winners);
	if (!losers.empty()) params["losers"] = std::move(losers);
}

void maf::Game_ended::summarise(ostream &os) const {
	for (auto it = game_log().game().players().begin(); it != game_log().game().players().end(); ) {
		const Player &player = *it;
		os << game_log().get_name(player) << (player.has_won() ? " won." : " lost.");
		if (++it != game_log().game().players().end()) {
			os << "\n";
		}
	}
}
