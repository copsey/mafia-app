#include <fstream>

#include "../riketi/algorithm.hpp"

#include "../common/stdlib.h"
#include "help_screens.hpp"
#include "names.hpp"


void maf::Help_Screen::write(std::ostream & output) const {
	// FIXME: This is horrendously fragile.
	std::string fname = "/Users/Jack/Documents/Developer/Projects/mafia/resources/txt/help/";
	fname += this->id();
	fname += ".txt";

	auto input = std::ifstream{fname};
	if (input) {
		output << input.rdbuf();
	} else {
		output << "=Missing Screen=\n\nERROR: No text found for the \"";
		output << this->id();
		output << "\" help screen.\n\n%Enter @ok@ to return to the previous screen.";
	}
}

void maf::Role_Info_Screen::set_params(TextParams& params) const {
	params["role"] = escaped(full_name(*role));
}

void maf::List_Roles_Screen::set_params(TextParams& params) const {
	params["show_all"] = (_filter_alignment == Filter_Alignment::all);
	params["show_village"] = (_filter_alignment == Filter_Alignment::village);
	params["show_mafia"] = (_filter_alignment == Filter_Alignment::mafia);
	params["show_freelance"] = (_filter_alignment == Filter_Alignment::freelance);

	vector<rkt::ref<const Role>> filtered_roles;

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

	std::vector<TextParams> roles;
	for (auto role_ref: filtered_roles) {
		auto& subparams = roles.emplace_back();
		auto& role = *role_ref;
		subparams["role"] = escaped(full_name(role));
		subparams["role.alias"] = escaped(role.alias());
		subparams["role.aligned_to_village"] = (role.alignment() == Alignment::village);
		subparams["role.aligned_to_mafia"] = (role.alignment() == Alignment::mafia);
		subparams["role.aligned_to_freelance"] = (role.alignment() == Alignment::freelance);
	}

	params["roles"] = std::move(roles);
}

void maf::Player_Info_Screen::set_params(TextParams& params) const {
	auto& game_log = *_game_log_ref;
	auto& game = game_log.game();
	auto& player = *_player_ref;

	std::vector<TextParams> investigations;
	for (auto& inv: game.investigations()) {
		if (inv.caster() == player) {
			auto& subparams = investigations.emplace_back();
			subparams["date"] = std::to_string(inv.date());
			subparams["target"] = escaped(game_log.get_name(inv.target()));
			subparams["target.suspicious"] = inv.result();
		}
	}

	params["daytime"] = (game.time() == Time::day);
	params["nighttime"] = (game.time() == Time::night);
	
	params["player"] = escaped(game_log.get_name(player));
	params["role"] = escaped(full_name(player.role()));
	params["has_wildcard"] = (player.wildcard() != nullptr);
	params["has_lynch_vote"] = (player.lynch_vote() != nullptr);
	params["investigations"] = std::move(investigations);
	
	if (player.wildcard()) {
		params["wildcard.alias"] = escaped(player.wildcard()->alias());
	}
	
	if (player.lynch_vote()) {
		params["lynch_vote"] = escaped(game_log.get_name(*player.lynch_vote()));
	}
}
