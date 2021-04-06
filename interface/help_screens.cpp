#include "../riketi/algorithm.hpp"

#include "../common/stdlib.h"
#include "help_screens.hpp"
#include "names.hpp"


void maf::Event_Help_Screen::write(ostream &os, TextParams& params) const {
	event->write_help(os, params);
}

void maf::Role_Info_Screen::write(ostream &os, TextParams& params) const {
	/* FIXME */

	params["role"] = escaped(full_name(*role));

	os << "=Missing Role Info=\n\nNo extra help could be found for the {role}.\n(this counts as a bug!)";
//	os << "=Help: {role}=\n\n...\n\nTo leave this screen, enter @ok@.";
}

void maf::List_Roles_Screen::write(ostream &os, TextParams& params) const {
	vector<rkt::ref<const Role>> filtered_roles{};

	switch (_filter_alignment) {
		case Filter_Alignment::all:
			filtered_roles = _rulebook->all_roles();
			break;

		case Filter_Alignment::village:
			filtered_roles = _rulebook->village_roles();
			break;

		case Filter_Alignment::mafia:
			filtered_roles = _rulebook->mafia_roles();
			break;

		case Filter_Alignment::freelance:
			filtered_roles = _rulebook->freelance_roles();
			break;
	}

	auto order_by_full_name = [](const rkt::ref<const Role> & r1, const rkt::ref<const Role> & r2) {
		return full_name(*r1) < full_name(*r2);
	};

	rkt::sort(filtered_roles, order_by_full_name);

	switch (_filter_alignment) {
		case Filter_Alignment::all:
			os << "=Roles=\n\nThe following is an alphabetical listing of every role in the rulebook:\n";
			break;

		case Filter_Alignment::village:
			os << "=Village Roles=\n\nThe following is an alphabetical listing of all of the roles in the rulebook aligned to the village:\n";
			break;

		case Filter_Alignment::mafia:
			os << "=Mafia Roles=\n\nThe following is an alphabetical listing of all of the roles in the rulebook aligned to the mafia:\n";
			break;

		case Filter_Alignment::freelance:
			os << "=Freelance Roles=\n\nThe following is an alphabetical listing of all of the freelance roles in the rulebook:\n";
			break;
	}

	for (auto role_ref : filtered_roles) {
		Role const& role = *role_ref;

		os << "   the " << escaped(full_name(role)) << ", alias @" << escaped(role.alias()) << "@";

		if (_filter_alignment == Filter_Alignment::all) {
			os << ", ";

			switch (role.alignment()) {
				case Alignment::village:
					os << "aligned to the village";
					break;

				case Alignment::mafia:
					os << "aligned to the mafia";
					break;

				case Alignment::freelance:
					os << "a freelance role";
					break;
			}
		}
		
		os << "\n";
	}

	os << "\nTo see more information about the role with alias @that@, enter @help r that@.\n\n";

	switch (_filter_alignment) {
		case Filter_Alignment::all:
			os << "To list only the village roles, enter @list r v@. Similarly, @list r m@ will list the mafia roles, and @list r f@ will list the freelance roles.";
			break;

		case Filter_Alignment::village:
			os << "To list the mafia roles, enter @list r m@, and to list the freelance roles, enter @list r f@.";
			break;

		case Filter_Alignment::mafia:
			os << "To list the village roles, enter @list r v@, and to list the freelance roles, enter @list r f@.";
			break;

		case Filter_Alignment::freelance:
			os << "To list the village roles, enter @list r v@, and to list the mafia roles, enter @list r m@.";
			break;
	}

	os << "\n\nTo leave this screen, enter @ok@.";
}

void maf::Setup_Help_Screen::write(ostream &os, TextParams& params) const {
	os << "=Help: Setup=\n\nThe setup screen is where you can choose the players and cards that will feature in the next game of Mafia.\n\nTo add a player called @name@ to the next game, enter @add p name@. The player can be removed again by entering @take p name@. To remove all of the players that have been selected, enter @clear p@.\n\nA single copy of the rolecard with alias @that@ can be added by entering @add r that@, and a single copy removed by entering @take r that@. You can remove all copies of the rolecard by entering @clear r that@, and you can remove every rolecard that has been selected by entering @clear r@.\n\nSimilar effects can be achieved for the wildcard with alias @that@ by using the commands @add w that@, @take w that@, @clear w that@, and @clear w@ respectively. In addition, every card that has been selected (both rolecards and wildcards) can be removed through the use of the command @clear c@.\n\nTo clear absolutely everything (both players and cards), enter @clear@.\n\nOnce you have finished choosing players and cards, you can enter @begin@ to start a new game. Alternatively, you can enter @preset i@ to start a particular preconfigured game, or just @preset@ to start a random preset. (Note: at the moment, presets exist primarily for testing if the app works; you are unlikely to ever use them.)\n\nYou can get extra information on the role with alias @that@ by entering @help r that@, and you can see a list of every role in the rulebook by entering @list r@. To see a list of only the village roles, you can enter @list r v@. Similarly, the command @list r m@ will list the mafia roles, and the command @list r f@ will list the freelance roles.\n\nThe commands @help w that@, @list w@, @list w v@, @list w m@, and @list w f@ have similar effects for wildcards.\n\nTo leave this screen, enter @ok@.";
}

void maf::Player_Info_Screen::write(ostream &os, TextParams& params) const {
	const Player & player = *_player_ref;
	const Game & game = _game_log_ref->game();
	const Game_log & game_log = *_game_log_ref;
	
	params["player"] = escaped(game_log.get_name(player));
	params["role"] = escaped(full_name(player.role()));
	
	if (player.has_fake_role()) {
		params["wildcard.alias"] = player.wildcard()->alias();
	}
	
	if (player.lynch_vote()) {
		params["lynchVote"] = escaped(game_log.get_name(*player.lynch_vote()));
	}

	os << "=Info: {player}=\n\nYour role is the {role}.";
	if (player.has_fake_role()) {
		// FIXME
		os << " You were randomly given this role from the @{wildcard.alias}@ wildcard.";
	}
	
	os << "\n\n";

	if (game.time() == Time::day) {
		if (player.lynch_vote()) {
			os << "You are voting to lynch {lynchVote}.";
		} else {
			os << "You are not voting to lynch anyone.";
		}
		os << "\n\n";
	}

	for (const Investigation& inv: game.investigations()) {
		if (inv.caster() == player) {
			os << "You checked ";
			os << escaped(game_log.get_name(inv.target()));
			os << " on night ";
			os << inv.date();
			os << ", who appeared to be ";
			os << ((inv.result()) ? "suspicious" : "innocent");
			os << ".\n";
		}
	}
}
