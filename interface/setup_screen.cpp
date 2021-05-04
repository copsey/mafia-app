#include "../util/algorithm.hpp"
#include "../util/misc.hpp"
#include "../util/parse.hpp"
#include "../util/string.hpp"

#include "command.hpp"
#include "console.hpp"
#include "names.hpp"
#include "setup_screen.hpp"
#include <charconv>

namespace maf {
	struct Game_parameters {
		vector<string> player_names;
		vector<core::Role::ID> role_ids;
		vector<core::Wildcard::ID> wildcard_ids;
		core::Rulebook rulebook;
	};

	const std::array<Game_parameters, 3> _presets = {
		Game_parameters{
			{"Augustus", "Brutus", "Claudius", "Drusilla"},
			{
				core::Role::ID::peasant,
				core::Role::ID::racketeer,
				core::Role::ID::coward
			},
			{core::Wildcard::ID::village_basic},
			{}
		},
		Game_parameters{
			{"Nine", "Ten", "Jack", "Queen", "King", "Ace"},
			{
				core::Role::ID::peasant,
				core::Role::ID::peasant,
				core::Role::ID::doctor,
				core::Role::ID::detective,
				core::Role::ID::dealer,
				core::Role::ID::musketeer
			},
			{},
			{}
		},
		Game_parameters{
			{"Alice", "Bob", "Charlie", "Daisy", "Eduardo", "Fiona"},
			{
				core::Role::ID::godfather,
				core::Role::ID::actor,
				core::Role::ID::serial_killer,
				core::Role::ID::village_idiot,
				core::Role::ID::village_idiot,
				core::Role::ID::village_idiot
			},
			{},
			{}
		}
	};

	const core::Rulebook & Setup_screen::rulebook() const {
		return _rulebook;
	}

	vector<string> Setup_screen::player_names() const {
		return {_player_names.begin(), _player_names.end()};
	}

	vector<core::Role::ID> Setup_screen::rolecard_ids() const {
		vector<core::Role::ID> v{};
		for (const auto &p: _role_ids) {
			for (std::size_t i{0}; i < p.second; ++i) v.push_back(p.first);
		}
		return v;
	}

	vector<core::Wildcard::ID> Setup_screen::wildcard_ids() const {
		vector<core::Wildcard::ID> v{};
		for (const auto &p: _wildcard_ids) {
			for (std::size_t i{0}; i < p.second; ++i) v.push_back(p.first);
		}
		return v;
	}

	bool Setup_screen::has_player(string_view name) const {
		for (string_view s: _player_names) {
			if (util::equal_up_to_case(s, name)) {
				return true;
			}
		}
		return false;
	}

	bool Setup_screen::has_rolecard(string_view alias) const {
		core::RoleRef r_ref = alias;

		try {
			auto& role = _rulebook.look_up(r_ref);
			auto id = role.id();

			return _role_ids.count(id) != 0 && _role_ids.at(id) != 0;
		} catch (std::out_of_range) {
			throw core::Rulebook::Missing_role_alias{string(alias)};
		}
	}

	bool Setup_screen::has_wildcard(string_view alias) const {
		core::Wildcard::ID id = _rulebook.get_wildcard(alias).id();
		return _wildcard_ids.count(id) != 0 && _wildcard_ids.at(id) != 0;
	}

	std::size_t Setup_screen::num_players() const {
		return _player_names.size();
	}

	std::size_t Setup_screen::num_rolecards() const {
		std::size_t n{0};
		for (auto &p: _role_ids) n += p.second;
		return n;
	}

	std::size_t Setup_screen::num_wildcards() const {
		std::size_t n{0};
		for (auto &p: _wildcard_ids) n += p.second;
		return n;
	}

	std::size_t Setup_screen::num_cards() const {
		return num_rolecards() + num_wildcards();
	}

	void Setup_screen::add_player(string_view name) {
		if (util::any_of(name, [](char ch) { return !std::isalnum(ch); })) {
			throw Bad_player_name{string{name}};
		} else if (has_player(name)) {
			throw Player_already_exists{string{name}};
		} else {
			_player_names.insert(string{name});
		}
	}

	void Setup_screen::add_rolecard(string_view alias) {
		core::RoleRef r_ref = alias;

		try {
			const core::Role & r = _rulebook.look_up(r_ref);
			++_role_ids[r.id()];
		} catch (std::out_of_range) {
			throw core::Rulebook::Missing_role_alias{string(alias)};
		}
	}

	void Setup_screen::add_wildcard(string_view alias) {
		const core::Wildcard & w = _rulebook.get_wildcard(alias);
		++_wildcard_ids[w.id()];
	}

	void Setup_screen::remove_player(string_view name) {
		auto it = util::find_if(_player_names, [&name](string_view s) {
			return util::equal_up_to_case(s, name);
		});

		if (it == _player_names.end()) {
			throw Player_missing{string{name}};
		} else {
			_player_names.erase(it);
		}
	}

	void Setup_screen::remove_rolecard(string_view alias) {
		core::RoleRef r_ref = alias;

		try {
			auto& role = _rulebook.look_up(r_ref);
			if (_role_ids[role.id()] == 0) {
				throw Rolecard_unselected{role};
			} else {
				--_role_ids[role.id()];
			}
		} catch (std::out_of_range) {
			throw core::Rulebook::Missing_role_alias{string(alias)};
		}
	}

	void Setup_screen::remove_wildcard(string_view alias) {
		const core::Wildcard & w = _rulebook.get_wildcard(alias);
		if (_wildcard_ids[w.id()] == 0) {
			throw Wildcard_unselected{w};
		} else {
			--_wildcard_ids[w.id()];
		}
	}

	void Setup_screen::clear_all_players() {
		_player_names.clear();
	}

	void Setup_screen::clear_rolecards(string_view alias) {
		core::RoleRef r_ref = alias;

		try {
			auto& role = _rulebook.look_up(r_ref);
			_role_ids[role.id()] = 0;
		} catch (std::out_of_range) {
			throw core::Rulebook::Missing_role_alias{string(alias)};
		}
	}

	void Setup_screen::clear_all_rolecards() {
		_role_ids.clear();
	}

	void Setup_screen::clear_wildcards(string_view alias) {
		const core::Wildcard & w = _rulebook.get_wildcard(alias);
		_wildcard_ids[w.id()] = 0;
	}

	void Setup_screen::clear_all_wildcards() {
		_wildcard_ids.clear();
	}

	void Setup_screen::clear_all_cards() {
		clear_all_rolecards();
		clear_all_wildcards();
	}

	void Setup_screen::clear_all() {
		clear_all_players();
		clear_all_cards();
	}

	unique_ptr<Game_log> Setup_screen::begin_pending_game() {
		return make_unique<Game_log>(console(), player_names(),
			rolecard_ids(), wildcard_ids(), rulebook());
	}

	unique_ptr<Game_log> Setup_screen::begin_preset(int i) {
		if (i >= 0 && i < std::size(_presets)) {
			Game_parameters params = _presets[i];
			return make_unique<Game_log>(console(), params.player_names,
				params.role_ids, params.wildcard_ids, params.rulebook);
		} else {
			throw Missing_preset{i};
		}
	}

	void Setup_screen::do_commands(const vector<string_view> & commands) {
		if (commands_match(commands, {"begin"})) {
			auto new_game = begin_pending_game();
			console().store_game(move(new_game));
		} else if (commands_match(commands, {"preset"})) {
			int num_presets = std::size(_presets);
			std::uniform_int_distribution<int> uid{0, num_presets - 1};
			int random_preset = uid(util::random_engine);
			auto new_game = begin_preset(random_preset);
			console().store_game(move(new_game));
		} else if (commands_match(commands, {"preset", ""})) {
			int i;
			string_view str = commands[1];

			if (auto result = util::from_chars(str, i);
			    result.ec == std::errc{})
			{
				auto new_game = begin_preset(i);
				console().store_game(move(new_game));
			} else {
				string msg = "=Error!=\n\nThe string @{str}@ could not be converted into a preset index. (i.e. a relatively-small integer)";
				auto params = TextParams{};
				params["str"] = escaped(str);

				throw Generic_error{move(msg), move(params)};
			}
		} else if (commands_match(commands, {"add", "p", ""})) {
			add_player(commands[2]);
		} else if (commands_match(commands, {"take", "p", ""})) {
			remove_player(commands[2]);
		} else if (commands_match(commands, {"clear", "p"})) {
			clear_all_players();
		} else if (commands_match(commands, {"add", "r", ""})) {
			add_rolecard(commands[2]);
		} else if (commands_match(commands, {"take", "r", ""})) {
			remove_rolecard(commands[2]);
		} else if (commands_match(commands, {"clear", "r", ""})) {
			clear_rolecards(commands[2]);
		} else if (commands_match(commands, {"clear", "r"})) {
			clear_all_rolecards();
		} else if (commands_match(commands, {"add", "w", ""})) {
			add_wildcard(commands[2]);
		} else if (commands_match(commands, {"take", "w", ""})) {
			remove_wildcard(commands[2]);
		} else if (commands_match(commands, {"clear", "w", ""})) {
			clear_wildcards(commands[2]);
		} else if (commands_match(commands, {"clear", "w"})) {
			clear_all_wildcards();
		} else if (commands_match(commands, {"clear", "c"})) {
			clear_all_cards();
		} else if (commands_match(commands, {"clear"})) {
			clear_all();
		} else if (commands_match(commands, {"help"})) {
			console().show_help_screen<Setup_Help_Screen>();
		} else {
			Screen::do_commands(commands);
		}
	}

	void Setup_screen::set_params(TextParams & params) const {
		vector<TextParams> players;
		vector<TextParams> cards;

		for (auto&& player_name: _player_names) {
			auto& subparams = players.emplace_back();
			subparams["player"] = player_name;
		}

		for (auto&& [role_id, count]: _role_ids) {
			if (count > 0) {
				auto& subparams = cards.emplace_back();
				subparams["card"] = escaped(full_name(role_id));
				subparams["count"] = static_cast<int>(count);
				subparams["type"] = 1;
			}
		}

		for (auto&& [wildcard_id, count]: _wildcard_ids) {
			if (count > 0) {
				auto& subparams = cards.emplace_back();
				subparams["card"] = escaped(alias(wildcard_id));
				subparams["count"] = static_cast<int>(count);
				subparams["type"] = 2;
			}
		}

		params["players.size"] = static_cast<int>(players.size());
		params["players"] = move(players);
		params["cards.size"] = static_cast<int>(cards.size());
		params["cards"] = move(cards);
	}
}
