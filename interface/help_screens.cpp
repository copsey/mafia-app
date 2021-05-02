#include <fstream>

#include "../util/algorithm.hpp"

#include "command.hpp"
#include "console.hpp"
#include "help_screens.hpp"
#include "names.hpp"

namespace maf {
	void Help_Screen::do_commands(const CmdSequence & commands) {
		if (commands_match(commands, {"ok"})) {
			console().dismiss_help_screen();
		} else {
			Screen::do_commands(commands);
		}
	}

	void Role_Info_Screen::set_params(TextParams & params) const {
		params["role"] = escaped(full_name(_role_id));
	}

	bool List_Roles_Screen::_compare_by_name(const core::Role & role_1, const core::Role & role_2) {
		return full_name(role_1) < full_name(role_2);
	}

	TextParams List_Roles_Screen::_get_params(const core::Role & role) {
		TextParams params;
		params["role"] = escaped(full_name(role));
		params["role.alias"] = escaped(role.alias());
		params["role.aligned_to_village"] = (role.alignment() == core::Alignment::village);
		params["role.aligned_to_mafia"] = (role.alignment() == core::Alignment::mafia);
		params["role.aligned_to_freelance"] = (role.alignment() == core::Alignment::freelance);
		return params;
	}

	vector_of_refs<const core::Role> List_Roles_Screen::_get_roles() const {
		auto roles = console().active_rulebook().roles();

		if (_filter_alignment) {
			util::remove_if(roles, [&](const core::Role & role) {
				return role.alignment() != *_filter_alignment;
			});
		}

		util::sort(roles, _compare_by_name);

		return roles;
	}

	void maf::List_Roles_Screen::set_params(TextParams& params) const {
		params["show_all"] = (!_filter_alignment);
		params["show_village"] = (_filter_alignment == core::Alignment::village);
		params["show_mafia"] = (_filter_alignment == core::Alignment::mafia);
		params["show_freelance"] = (_filter_alignment == core::Alignment::freelance);

		auto roles = this->_get_roles();
		params["roles"] = util::transformed_copy(roles, _get_params);
	}

	void maf::Player_Info_Screen::set_params(TextParams& params) const {
		auto& game_log = console().game_log();
		auto& game = game_log.game();

		auto get_investigation_params = [&](const core::Investigation & investigation) {
			TextParams params;
			params["date"] = static_cast<int>(investigation.date);
			params["target"] = escaped(game_log.get_name(investigation.target));
			params["target.suspicious"] = investigation.result;
			return params;
		};

		vector<TextParams> investigations;
		for (auto& inv: game.investigations()) {
			if (inv.caster == _player) {
				investigations.push_back(get_investigation_params(inv));
			}
		}

		params["daytime"] = (game.time() == core::Time::day);
		params["nighttime"] = (game.time() == core::Time::night);

		params["player"] = escaped(game_log.get_name(_player));
		params["role"] = escaped(full_name(_player.role()));
		params["has_lynch_vote"] = (_player.lynch_vote() != nullptr);
		params["investigations"] = investigations;

		if (_player.lynch_vote()) {
			params["lynch_vote"] = escaped(game_log.get_name(*_player.lynch_vote()));
		}
}
}
