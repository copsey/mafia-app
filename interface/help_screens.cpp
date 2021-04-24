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

maf::vector<std::reference_wrapper<const maf::Role>> maf::List_Roles_Screen::_get_roles() const {
	auto roles = _rulebook->roles();

	if (_filter_alignment) {
		util::remove_if(roles, [&](Role const& role) {
			return role.alignment() != *_filter_alignment;
		});
	}

	util::sort(roles, _compare_by_name);

	return roles;
}

void maf::List_Roles_Screen::set_params(TextParams& params) const {
	params["show_all"] = (!_filter_alignment);
	params["show_village"] = (_filter_alignment == Alignment::village);
	params["show_mafia"] = (_filter_alignment == Alignment::mafia);
	params["show_freelance"] = (_filter_alignment == Alignment::freelance);

	auto roles = this->_get_roles();
	params["roles"] = util::transformed_copy(roles, _get_params);
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
