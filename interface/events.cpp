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
	os << "^TMissing Help Screen^/^hNo help has been written for the current game event.\n(this counts as a bug!)\n\nEnter ^cok^/ to leave this screen.";
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
	auto & glog = game_log();

	params["player"] = glog.get_name(*_p);
	params["role.name"] = full_name(*_r);
	params["role.alias"] = _r->alias();

	if (_w != nullptr) {
		params["wildcard.alias"] = _w->alias();
	}

	os << "^T{player}'s Role^/";

	if (_is_private) {
		os << "{player}, your role is the {role.name}.";

		if (_w != nullptr) {
			/* FIXME */
			os << "\nYou were randomly given this role from the ^c{wildcard.alias}^/ wildcard.";
		}

		os << "^h\n\nTo see a full description of your role, enter ^chelp r {role.alias}^/.^/";
	} else {
		os << "{player}, you are about to be shown your role.";
	}
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

	switch (time) {
		case Time::day: {
			os << "^TDay {date}^/^iDawn breaks, and dim sunlight beams onto the weary townsfolk...\n\n^/It is now day {date}. Anybody still asleep can wake up.";
			break;
		}

		case Time::night: {
			os << "^TNight {date}^/^iAs darkness sets in, the townsfolk return to the comforts of their shelters...\n\n^/It is now night {date}. Everybody still in the game should go to sleep.";
			break;
		}
	}
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
	params["num_deaths"] = _deaths.size();

	if (_deaths_index >= 0) {
		Player const& deceased = *_deaths[_deaths_index];
		params["deceased"] = game_log().get_name(deceased);

		if (deceased.haunter()) {
			params["haunter"] = game_log().get_name(*deceased.haunter());
		}
	}

	os << "^TObituary^/";

	if (_deaths_index < 0) {
		if (_deaths.size() == 0) {
			os << "Nobody died during the night.";
		} else {
			/* FIXME: reword to remove use of "us". */
			os << "It appears that {num_deaths} of us did not survive the night...";
		}
	} else {
		const Player& death = *_deaths[_deaths_index];

		os << "{deceased} died during the night!";
		if (death.is_haunted()) {
			os << "\n\nA slip of paper was found by their bed. On it has been written the name \"{haunter}\" over and over...";
		}
	}
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
		else if (commands_match(commands, {"vote", "", ""})) {
			const Player &voter = glog.find_player(commands[1]);
			const Player &target = glog.find_player(commands[2]);

			glog.cast_lynch_vote(voter.id(), target.id());
			glog.advance();
		}
		else if (commands_match(commands, {"abstain", ""})) {
			const Player &voter = glog.find_player(commands[1]);

			glog.clear_lynch_vote(voter.id());
			glog.advance();
		}
		else if (commands_match(commands, {"lynch"})) {
			glog.process_lynch_votes();
			glog.advance();
		}
		else if (commands_match(commands, {"duel", "", ""})) {
			const Player &caster = glog.find_player(commands[1]);
			const Player &target = glog.find_player(commands[2]);

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
		else if (commands_match(commands, {"duel", "", ""})) {
			const Player & caster = glog.find_player(commands[1]);
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
	/* FIXME: add in proper content. */

	params["date"] = std::to_string(_date);

	os << "^TDay {date}^/";

	if (_lynch_can_occur) {
		os << "Gathered outside the town hall are:\n";

		for (auto it = _players.begin(); it != _players.end(); ) {
			auto& p_ref = *it;
			os << "   ";
			os << game_log().get_name(*p_ref);
			if (p_ref->lynch_vote() != nullptr) {
				os << ", voting to lynch ";
				os << game_log().get_name(*(p_ref->lynch_vote()));
			}
			os << ((++it == _players.end()) ? "." : ",\n");
		}

		os << "\n\nAs it stands, "
		<< (_next_lynch_victim ? game_log().get_name(*_next_lynch_victim) : "nobody")
		<< " will be lynched.^h\n\nEnter ^clynch^/ to submit the current lynch votes. Daytime abilities may also be used at this point.";
	} else {
		os << "^iWith little time left in the day, the townsfolk prepare themselves for another night of uncertainty...^/\n\n";
		os << "Gathered outside the town hall are:\n";

		for (auto it = _players.begin(); it != _players.end(); ) {
			auto& p_ref = *it;
			os << "   ";
			os << game_log().get_name(*p_ref);
			os << ((++it == _players.end()) ? "." : ",\n");
		}

		os << "^h\n\nAnybody who wishes to use a daytime ability may do so now. Otherwise, enter ^cnight^/ to continue.";
	}
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
	params["player"] = game_log().get_name(*player);
	params["role"] = full_name(player->role());

	os << "^T{player} kicked^/";
	os << "{player} was kicked from the game!\n";
	os << "They were the {role}.";
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
	params["victim"] = game_log().get_name(*victim);

	if (victim_role) {
		params["victim.role"] = full_name(victim_role->id());
	}


	os << "^TLynch Result^/";

	if (victim) {
		os << "{victim} was lynched!\n";

		if (victim_role) {
			os << "They were a {victim.role}.";
			if (victim_role->is_troll()) {
				os << "^i\n\nA chill blows through the air. The townsfolk who voted to lynch {victim} look nervous...";
			}
		} else {
			os << "Their role could not be determined.";
		}
	} else {
		os << "Nobody was lynched today.";
	}
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
	auto& winner = (caster->is_alive() ? caster : target);
	auto& loser = (caster->is_alive() ? target : caster);

	auto& glog = game_log();

	params["caster"] = glog.get_name(*caster);
	params["target"] = glog.get_name(*target);
	params["winner"] = glog.get_name(*winner);
	params["loser"] = glog.get_name(*loser);

	os << "^TDuel^/";
	os << "{caster} has challenged {target} to a duel!^i\n\nThe pistols are loaded, and the participants take ten paces in opposite directions...\n\n3... 2... 1... BANG!!^/\n\nThe lifeless body of {loser} falls to the ground. {winner} lets out a sigh of relief.";

	if (!winner->is_present()) {
		os << "\n\nWith that, {winner} throws their gun to the ground and flees from the village.";
	}

	os << "^h\n\nWhen you have finished with this screen, enter ^cok^/.";
}

void maf::Duel_result::write_summary(ostream &os) const {
	auto & glog = game_log();
	auto caster_name = glog.get_name(*caster);
	auto target_name = glog.get_name(*target);

	if (caster->is_alive()) { // FIXME: unstable code, as the caster may not be alive later in the game if the rules change, yet still won the duel
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
	params["player"] = game_log().get_name(*_player);

	if (_fake_role) {
		params["fakeRole.name"] = full_name(*_fake_role);
		params["fakeRole.alias"] = _fake_role->alias();
	}

	os << "^TChoose Fake Role^/";

	if (_go_to_sleep) {
		os << "{player} should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^/ to continue.";
	}
	else if (_fake_role) {
		os << "{player}, you have been given the {fakeRole.name} as your fake role.\n\nYou must pretend that this is your real role for the remainder of the game. Breaking this rule will result in you being kicked from the game!\n\nNow would be a good time to study your fake role.^h\n\nEnter ^chelp r {fakeRole.alias}^/ to see more information about your fake role.\nWhen you are ready, enter ^cok^/ to continue.";
	}
	else {
		os << "{player} needs to be given a fake role, which they must pretend is their true role for the rest of the game.^h\n\nIf they break the rules by contradicting their fake role, then they should be kicked from the game by entering ^ckick {player}^/ during the day.\n\nTo choose the role with alias ^cA^/, enter ^cchoose A^/.";
	}
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
		if (commands_match(commands, {"kill", "", ""})) {
			const Player &caster = glog.find_player(commands[1]);
			const Player &target = glog.find_player(commands[2]);

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

void maf::Mafia_meeting::write_full(ostream &os, TextParams& params) const
{
	/* FIXME */
	os << "^TMafia Meeting^/";

	if (_go_to_sleep) {
		os << "The mafia have nothing more to discuss for now, and should go back to sleep.^h\n\nEnter ^cok^/ when you are ready to continue.";
	} else if (_initial) {
		os << "The mafia consists of:\n";

		for (auto it = _mafiosi.begin(); it != _mafiosi.end(); ) {
			auto& p_ref = *it;
			os << "   " << game_log().get_name(*p_ref) << ", the " << full_name(p_ref->role());
			os << ((++it == _mafiosi.end()) ? "." : ",\n");
		}

		os << "\n\nThere is not enough time left to organise a murder.";
	} else {
		os << "Seated around a polished walnut table are:\n";

		for (auto it = _mafiosi.begin(); it != _mafiosi.end(); ) {
			auto& p_ref = *it;
			os << "   " << game_log().get_name(*p_ref) << ", the " << full_name(p_ref->role());
			os << ((++it == _mafiosi.end()) ? "." : ",\n");
		}

		os << "\n\nThe mafia are ready to choose their next victim.^h\n\nEntering ^ckill A B^/ will make player ^cA^/ attempt to kill player ^cB^/. Player ^cA^/ must be a member of the mafia.\n\nIf the mafia have chosen not to kill anybody this night, enter ^cskip^/.";
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

void maf::Kill_use::write_full(ostream &os, TextParams& params) const {
	auto & glog = game_log();

	params["caster"] = glog.get_name(*_caster);

	os << "^TKill Use^/";

	if (_go_to_sleep) {
		os << "{caster} should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^/ to continue.";
	}
	else {
		os << "{caster}, you can choose to kill somebody this night.^h\n\nEnter ^ckill A^/ to kill player ^cA^/, or enter ^cskip^/ if you don't wish to kill anybody.";
	}
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
	auto & glog = game_log();

	params["caster"] = glog.get_name(*_caster);

	os << "^THeal Use^/";

	if (_go_to_sleep) {
		os << "{caster} should now go back to sleep.";
		os << "^h\n\nWhen you are ready, enter ^cok^/ to continue.";
	} else {
		os << "{caster}, you can choose to heal somebody this night.";
		os << "^h\n\nEnter ^cheal A^/ to heal player ^cA^/, or enter ^cskip^/ if you don't wish to heal anybody.";
	}
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
	auto & glog = game_log();

	params["caster"] = glog.get_name(*_caster);

	os << "^TInvestigation^/";

	if (_go_to_sleep) {
		os << "{caster} should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^/ to continue.";
	} else {
		os << "{caster}, you can choose to investigate somebody this night.^h\n\nEnter ^ccheck A^/ to investigate player ^cA^/, or enter ^cskip^/ if you don't wish to investigate anybody.";
	}
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
	auto & glog = game_log();

	params["caster"] = glog.get_name(*_caster);

	os << "^TPeddle^/";

	if (_go_to_sleep) {
		os << "{caster} should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^/ to continue.";
	}
	else {
		os << "{caster}, you can choose to peddle drugs to somebody this night.^h\n\nEnter ^ctarget A^/ to peddle drugs to player ^cA^/, or enter ^cskip^/ if you don't wish to peddle drugs to anybody.";
	}
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

	os << "^TCalm Night^/^iIt is warm outside. The moon shines brightly. The gentle chirping of crickets is carried by a pleasant breeze...^/\n\nNothing of interest happened this night, although you should still wait a few moments before continuing, to maintain the illusion that something happened.^h\n\nEnter ^cok^/ to continue.";
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
	auto & glog = game_log();

	params["caster"] = glog.get_name(investigation.caster());

	if (investigation.result()) {
		params["target"] = glog.get_name(investigation.target());
	}

	os << "^TInvestigation Result^/";

	if (_go_to_sleep) {
		os << "{caster} should now go back to sleep.^h\n\nWhen you are ready, enter ^cok^/ to continue.";
	}
	else {
		os << "{caster}, you have completed your investigation of {target}.\n\n";

		if (investigation.result()) {
			os << "{target} was behaving very suspiciously this night!";
		}
		else {
			os << "The investigation was fruitless. {target} appears to be innocent.";
		}

		os << "^h\n\nWhen you are ready, enter ^cok^/ to continue.";
	}
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
	vector<const Player *> winners{};
	vector<const Player *> losers{};
	for (const Player &player: game_log().game().players()) {
		if (player.has_won()) {
			winners.push_back(&player);
		} else {
			losers.push_back(&player);
		}
	}

	os << "^TGame Over^/The game has ended!";

	if (winners.size() > 0) {
		os << "\n\nThe following players won:\n";

		for (auto it = winners.begin(); it != winners.end(); ) {
			const Player &player = **it;
			os << "   " << game_log().get_name(player) << ", the " << full_name(player.role());
			os << ((++it == winners.end()) ? "." : ",\n");
		}
	} else {
		os << "\n\nNobody won.";
	}

	if (losers.size() > 0) {
		os << "\n\nCommiserations go out to:\n";

		for (auto it = losers.begin(); it != losers.end(); ) {
			const Player &player = **it;
			os << "   " << game_log().get_name(player) << ", the " << full_name(player.role());
			os << ((++it == losers.end()) ? "." : ",\n");
		}
	} else {
		os << "\n\nNobody lost.";
	}

	os << "^h\n\nTo return to the setup screen, enter ^cend^/.";
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
