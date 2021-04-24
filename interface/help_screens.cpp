#include <fstream>
#include <functional>

#include "../util/algorithm.hpp"

#include "help_screens.hpp"
#include "names.hpp"

void maf::Role_Info_Screen::set_params(TextParams& params) const {
	params["role"] = escaped(full_name(*role));
}

bool maf::List_Roles_Screen::_compare_by_name(Role const& role_1, Role const& role_2) {
	return full_name(role_1) < full_name(role_2);
}

maf::TextParams maf::List_Roles_Screen::_get_params(Role const& role) {
	TextParams params;
	params["role"] = escaped(full_name(role));
	params["role.alias"] = escaped(role.alias());
	params["role.aligned_to_village"] = (role.alignment() == Alignment::village);
	params["role.aligned_to_mafia"] = (role.alignment() == Alignment::mafia);
	params["role.aligned_to_freelance"] = (role.alignment() == Alignment::freelance);
	return params;
}

void maf::List_Roles_Screen::set_params(TextParams& params) const {
	params["show_all"] = (!_filter_alignment);
	params["show_village"] = (_filter_alignment == Alignment::village);
	params["show_mafia"] = (_filter_alignment == Alignment::mafia);
	params["show_freelance"] = (_filter_alignment == Alignment::freelance);

	vector<std::reference_wrapper<const Role>> filtered_roles;

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

	util::sort(filtered_roles, _compare_by_name);

	params["roles"] = util::transformed_copy(filtered_roles, _get_params);
}

void maf::Player_Info_Screen::set_params(TextParams& params) const {
	auto& game_log = *_game_log_ref;
	auto& game = game_log.game();
	auto& player = *_player_ref;

	vector<TextParams> investigations;
	for (auto& inv: game.investigations()) {
		if (inv.caster() == player) {
			auto& subparams = investigations.emplace_back();
			subparams["date"] = static_cast<int>(inv.date());
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
	params["investigations"] = move(investigations);

	if (player.wildcard()) {
		params["wildcard.alias"] = escaped(player.wildcard()->alias());
	}

	if (player.lynch_vote()) {
		params["lynch_vote"] = escaped(game_log.get_name(*player.lynch_vote()));
	}
}
