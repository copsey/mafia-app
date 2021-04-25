#include <fstream>

#include "../util/algorithm.hpp"

#include "help_screens.hpp"
#include "names.hpp"

void maf::Role_Info_Screen::set_params(TextParams& params) const {
	params["role"] = escaped(full_name(_role_id));
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

maf::vector_of_refs<const maf::Role> maf::List_Roles_Screen::_get_roles() const {
	auto roles = _rulebook.roles();

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
	auto& game = _game_log.game;

	auto get_investigation_params = [&](const Investigation & investigation) {
		TextParams params;
		params["date"] = static_cast<int>(investigation.date);
		params["target"] = escaped(_game_log.get_name(investigation.target));
		params["target.suspicious"] = investigation.result;
		return params;
	};

	vector<TextParams> investigations;
	for (auto& inv: game.investigations()) {
		if (inv.caster == _player) {
			investigations.push_back(get_investigation_params(inv));
		}
	}

	params["daytime"] = (game.time() == Time::day);
	params["nighttime"] = (game.time() == Time::night);

	params["player"] = escaped(_game_log.get_name(_player));
	params["role"] = escaped(full_name(_player.role()));
	params["has_wildcard"] = (_player.wildcard() != nullptr);
	params["has_lynch_vote"] = (_player.lynch_vote() != nullptr);
	params["investigations"] = investigations;

	if (_player.wildcard()) {
		params["wildcard.alias"] = escaped(_player.wildcard()->alias());
	}

	if (_player.lynch_vote()) {
		params["lynch_vote"] = escaped(_game_log.get_name(*_player.lynch_vote()));
	}
}
