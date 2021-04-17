#include "command.hpp"
#include "console.hpp"
#include "events.hpp"
#include "names.hpp"
#include "../common/stdlib.h"

void maf::Event::write_summary(ostream &os) const
{
	// By default, write nothing.
}

void maf::Event::write_help(ostream &os, TextParams& params) const
{
	os << "=Missing Help Screen=\n\n%No help has been written for the current game event.\n(this counts as a bug!)\n\nEnter @ok@ to leave this screen.";
}

std::string maf::Event::escaped_name(Player const& player) const {
	return escaped(game_log().get_name(player));
}

std::string maf::Event::escaped_name(Role const& role) const {
	return escaped(full_name(role));
}

void maf::Player_given_initial_role::do_commands(const vector<string_view> & commands)
{
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

void maf::Player_given_initial_role::write_full(ostream &os, TextParams& params) const
{
	params["from_wildcard"] = (_w != nullptr);
	params["player"] = escaped(game_log().get_name(*_p));
	params["private"] = _is_private;
	params["role"] = escaped(full_name(*_r));
	params["role.alias"] = escaped(_r->alias());

	if (_w != nullptr) {
		params["wildcard.alias"] = escaped(_w->alias());
	}

	os << "={player}'s Role=\n\n{!if private}\n{player}, your role is the {role}.\n{!if from_wildcard}\n\nYou were randomly given this role from the @{wildcard.alias}@ wildcard.\n{!end}\n\n%To see a full description of your role, enter @help r {role.alias}@.\n{!else}\n{player}, you are about to be shown your role.\n{!end}";
}

void maf::Player_given_initial_role::write_summary(ostream &os) const {
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

void maf::Time_changed::write_full(ostream & os, TextParams& params) const
{
	params["date"] = std::to_string(date);
	params["daytime"] = (time == Time::day);
	params["nighttime"] = (time == Time::night);
	
	os << "{!if daytime}\n=Day {date}=\n\n_Dawn breaks, and dim sunlight beams onto the weary townsfolk..._\n\nIt is now day {date}. Anybody still asleep can wake up.\n{!else}\n=Night {date}=\n\n_As darkness sets in, the townsfolk return to the comforts of their shelters..._\n\nIt is now night {date}. Everybody still in the game should go to sleep.\n{!end}";
}

void maf::Time_changed::write_summary(ostream &os) const {
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

void maf::Obituary::write_full(ostream &os, TextParams& params) const
{
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

//	os << "=Obituary=\n\n{!if show_death}\n{deceased} died during the night!\n{!if deceased.is_haunted}\n\nA slip of paper was found by their bed. On it has been written the name \"{ghost}\" over and over...\n{!end}\n{!else_if anyone_died}\n{--- FIXME: reword to remove use of \"us\".}\nIt appears that {num_deaths} of us did not survive the night...\n{!else}\nNobody died during the night.\n{!end}";
	os << "=Obituary=\n\n{!if show_death}\n{deceased} died during the night!\n{!if deceased.is_haunted}\n\nA slip of paper was found by their bed. On it has been written the name \"{ghost}\" over and over...\n{!end}\n{!else_if anyone_died}\nIt appears that {num_deaths} of us did not survive the night...\n{!else}\nNobody died during the night.\n{!end}";
}

void maf::Obituary::write_summary(ostream &os) const {
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

void maf::Town_meeting::write_full(ostream &os, TextParams& params) const {
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

	os << "=Day {date}=\n\n{!if lynch_can_occur}\nGathered outside the town hall are:\n{!list townsfolk}\n   {player}{!if player.has_voted}, voting to lynch {player.vote}{!end}\n{!end}\n\nAs it stands, {!if lynch_target.exists}{lynch_target}{!else}nobody{!end} will be lynched.\n\n%Enter @lynch@ to submit the current lynch votes. Daytime abilities may also be used at this point.\n{!else}\n_With little time left in the day, the townsfolk prepare themselves for another night of uncertainty..._\n\nGathered outside the town hall are:\n{!list townsfolk}   {player}\n{!end}\n\n%Anybody who wishes to use a daytime ability may do so now. Otherwise, enter @night@ to continue.\n{!end}";
}

void maf::Town_meeting::write_summary(ostream &os) const {
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

void maf::Player_kicked::write_full(ostream &os, TextParams& params) const
{
	params["player"] = escaped(game_log().get_name(*player));
	params["role"] = escaped(full_name(player->role()));

	os << "={player} kicked=\n\n{player} was kicked from the game!\nThey were the {role}.";
}

void maf::Player_kicked::write_summary(ostream &os) const {
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

void maf::Lynch_result::write_full(ostream &os, TextParams& params) const {
	params["victim.exists"] = (victim != nullptr);
	
	if (victim) {
		params["victim"] = escaped(game_log().get_name(*victim));
		params["victim.role.hidden"] = (victim_role == nullptr);
		
		if (victim_role) {
			params["victim.role"] = escaped(full_name(victim_role->id()));
			params["victim.role.is_troll"] = victim_role->is_troll();
		}
	}
	
	os << "=Lynch Result=\n\n{!if victim.exists}\n{victim} was lynched!\n\n{!if victim.role.hidden}\nTheir role could not be determined.\n{!else}\nThey were a {victim.role}.\n{!if victim.role.is_troll}\n\n_A chill blows through the air. The townsfolk who voted to lynch {victim} look nervous..._\n{!end}\n{!end}\n{!else}\nNobody was lynched today.\n{!end}";
}

void maf::Lynch_result::write_summary(ostream &os) const
{
	if (victim) {
		os << game_log().get_name(*victim) << " was lynched.";
	}
	else {
		os << "Nobody was lynched.";
	}
}

void maf::Duel_result::do_commands(const vector<string_view> & commands)
{
	if (commands_match(commands, {"ok"})) {
		game_log().advance();
	}
	else {
		throw Bad_commands{};
	}
}

void maf::Duel_result::write_full(ostream &os, TextParams& params) const {
	params["caster"] = escaped(game_log().get_name(*caster));
	params["target"] = escaped(game_log().get_name(*target));
	params["winner"] = escaped(game_log().get_name(*winner));
	params["loser"] = escaped(game_log().get_name(*loser));
	params["winner.fled"] = !winner->is_present();

	os << "=Duel=\n\n{caster} has challenged {target} to a duel!\n\n_The pistols are loaded, and the participants take ten paces in opposite directions...\n\n3... 2... 1... BANG!!_\n\nThe lifeless body of {loser} falls to the ground. {winner} lets out a sigh of relief.\n{!if winner.fled}\n\nWith that, {winner} throws their gun to the ground and flees from the village.\n{!end}\n\n%When you have finished with this screen, enter @ok@.";
}

void maf::Duel_result::write_summary(ostream &os) const {
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

void maf::Choose_fake_role::write_full(ostream &os, TextParams& params) const
{
	params["finished"] = _go_to_sleep;
	params["player"] = escaped(game_log().get_name(*_player));
	params["fake_role.chosen"] = (_fake_role != nullptr);

	if (_fake_role) {
		params["fake_role"] = escaped_name(*_fake_role);
		params["fake_role.alias"] = escaped(_fake_role->alias());
	}
	
	os << "=Choose Fake Role=\n\n{!if finished}\n%{player} should now go back to sleep.\n\nWhen you are ready, enter @ok@ to continue.\n{!else_if fake_role.chosen}\n{player}, you have been given the {fake_role} as your fake role.\n\nYou must pretend that this is your real role for the remainder of the game. Breaking this rule will result in you being kicked from the game!\n\nNow would be a good time to study your fake role.\n\n%Enter @help role {fake_role.alias}@ to see more information about the role. When you are ready, enter @ok@ to continue.\n{!else}\n%{player} needs to be given a fake role, which they must pretend is their true role for the rest of the game.\n\nIf they break the rules by contradicting their fake role, then they should be kicked from the game by entering @kick {player}@ during the day.\n\nTo choose the role with alias @A@, enter @choose A@.\n{!end}";
}

void maf::Choose_fake_role::write_summary(ostream &os) const {
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

void maf::Mafia_meeting::write_full(std::ostream & out, TextParams& params) const
{
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
	
	out << "=Mafia Meeting=\n\n{!if finished}\n%The mafia have nothing more to discuss for now, and should go back to sleep.\n\nEnter @ok@ when you are ready to continue.\n{!else_if first_meeting}\n{!if single_member}\nSeated alone at a polished walnut table is {player}.\n{!else}\nThe mafia consists of:\n{!list mafia}\n - {player}, the {role}\n{!end}\n{!end}\n\nThere is not enough time left to organise a murder.\n{!else}\n{!if single_member}\nSeated alone at a polished walnut table is {player}.\n{!else}\nSeated around a polished walnut table are:\n{!list mafia}\n - {player}, the {role}\n{!end}\n{!end}\n\nThe mafia are ready to choose their next victim.\n\n{!if single_member}\n%Entering @kill A@ will make {player} attempt to kill player @A@. If {player} doesn't want to kill anybody this night, enter @skip@ instead.\n{!else}\n%Entering @A kill B@ will make player @A@ attempt to kill player @B@. Player @A@ must be a member of the mafia. If the mafia have chosen not to kill anybody this night, enter @skip@ instead.\n{!end}\n{!end}";
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

void maf::Kill_use::write_full(ostream &os, TextParams& params) const {
	params["caster"] = escaped(game_log().get_name(*_caster));
	params["finished"] = _go_to_sleep;

	os << "=Kill Use=\n\n{!if finished}\n{caster} should now go back to sleep.\n\n%When you are ready, enter @ok@ to continue.\n{!else}\n{caster}, you can choose to kill somebody this night.\n\n%Enter @kill A@ to kill player @A@, or enter @skip@ if you don't wish to kill anybody.\n{!end}";
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

void maf::Heal_use::write_full(ostream &os, TextParams& params) const {
	params["caster"] = escaped(game_log().get_name(*_caster));
	params["finished"] = _go_to_sleep;

	os << "=Heal Use=\n\n{!if finished}\n{caster} should now go back to sleep.\n\n%When you are ready, enter @ok@ to continue.\n{!else}\n{caster}, you can choose to heal somebody this night.\n\n%Enter @heal A@ to heal player @A@, or enter @skip@ if you don't wish to heal anybody.\n{!end}";
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

void maf::Investigate_use::write_full(ostream &os, TextParams& params) const {
	params["caster"] = escaped(game_log().get_name(*_caster));
	params["finished"] = _go_to_sleep;

	os << "=Investigation=\n\n{!if finished}\n{caster} should now go back to sleep.\n\n%When you are ready, enter @ok@ to continue.\n{!else}\n{caster}, you can choose to investigate somebody this night.\n\n%Enter @check A@ to investigate player @A@, or enter @skip@ if you don't wish to investigate anybody.\n{!end}";
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

void maf::Peddle_use::write_full(ostream &os, TextParams& params) const {
	params["caster"] = escaped(game_log().get_name(*_caster));
	params["finished"] = _go_to_sleep;

	os << "=Peddle=\n\n{!if finished}\n{caster} should now go back to sleep.\n\n%When you are ready, enter @ok@ to continue.\n{!else}\n{caster}, you can choose to peddle drugs to somebody this night.\n\n%Enter @target A@ to peddle drugs to player @A@, or enter @skip@ if you don't wish to peddle drugs to anybody.\n{!end}";
}

void maf::Boring_night::do_commands(const vector<string_view> & commands) {
	if (commands_match(commands, {"ok"})) {
		game_log().advance();
	}
	else {
		throw Bad_commands{};
	}
}

void maf::Boring_night::write_full(ostream &os, TextParams& params) const {
	/* FIXME: show current date in title. (e.g. "Night 1") */

	os << "=Calm Night=\n\n_It is warm outside. The moon shines brightly. The gentle chirping of crickets is carried by a pleasant breeze..._\n\n%Nothing of interest happened this night, although you should still wait a few moments before continuing, to maintain the illusion that something happened.\n\nEnter @ok@ when you're ready to continue.";
}

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

void maf::Investigation_result::write_full(ostream &os, TextParams& params) const {
	params["caster"] = escaped(game_log().get_name(investigation.caster()));
	params["finished"] = _go_to_sleep;
	params["target"] = escaped(game_log().get_name(investigation.target()));
	params["target.suspicious"] = investigation.result();
	
	os << "=Investigation Result=\n\n{!if finished}\n%{caster} should now go back to sleep.\n\nWhen you are ready, enter @ok@ to continue.\n{!else}\n{caster}, you have completed your investigation of {target}.\n\n{!if target.suspicious}\n{target} was behaving very suspiciously this night!\n{!else}\nThe investigation was fruitless. {target} appears to be innocent.\n{!end}\n\n%When you are ready, enter @ok@ to continue.\n{!end}";
}

void maf::Investigation_result::write_summary(ostream &os) const
{
	os << game_log().get_name(investigation.caster())
	<< " decided that "
	<< game_log().get_name(investigation.target())
	<< " was "
	<< (investigation.result() ? "suspicious" : "innocent")
	<< ".";
}

void maf::Game_ended::do_commands(const vector<string_view> & commands)
{
	throw Bad_commands{};
}

void maf::Game_ended::write_full(ostream &os, TextParams& params) const
{
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

	os << "=Game Over=\n\nThe game has ended!\n\n{!if any_winners}\nThe following players won:\n{!list winners}\n - {player}, the {role}\n{!end}\n{!else}\nNobody won.\n{!end}\n\n{!if any_losers}\nCommiserations go out to:\n{!list losers}\n - {player}, the {role}\n{!end}\n{!else}\nNobody lost.\n{!end}\n\n%To return to the setup screen, enter @end@.";
}

void maf::Game_ended::write_summary(ostream &os) const
{
	for (auto it = game_log().game().players().begin(); it != game_log().game().players().end(); ) {
		const Player &player = *it;
		os << game_log().get_name(player) << (player.has_won() ? " won." : " lost.");
		if (++it != game_log().game().players().end()) {
			os << "\n";
		}
	}
}
