#include <fstream>

#include "../util/algorithm.hpp"

#include "help_screens.hpp"
#include "names.hpp"


void maf::Role_Info_Screen::set_params(TextParams& params) const {
	params["role"] = escaped(full_name(*role));
}

void maf::List_Roles_Screen::set_params(TextParams& params) const {
	params["show_all"] = (!_filter_alignment);
	params["show_village"] = (_filter_alignment == Alignment::village);
	params["show_mafia"] = (_filter_alignment == Alignment::mafia);
	params["show_freelance"] = (_filter_alignment == Alignment::freelance);

	std::vector<util::ref<const Role>> filtered_roles;

	if (_filter_alignment) {
		switch (*_filter_alignment) {
		case Alignment::village:
			filtered_roles = _rulebook->village_roles();
			break;

		case Alignment::mafia:
			filtered_roles = _rulebook->mafia_roles();
			break;

		case Alignment::freelance:
			filtered_roles = _rulebook->freelance_roles();
			break;
		}
	} else {
		filtered_roles = _rulebook->all_roles();
	}

	auto order_by_full_name = [](auto&& r1, auto&& r2) {
		return full_name(*r1) < full_name(*r2);
	};

	util::sort(filtered_roles, order_by_full_name);

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
