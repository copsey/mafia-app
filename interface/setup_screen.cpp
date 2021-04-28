#include "../util/algorithm.hpp"
#include "../util/string.hpp"

#include "command.hpp"
#include "console.hpp"
#include "names.hpp"
#include "setup_screen.hpp"

struct Game_parameters {
	maf::vector<maf::string> player_names;
	maf::vector<maf::Role::ID> role_ids;
	maf::vector<maf::Wildcard::ID> wildcard_ids;
	maf::Rulebook rulebook;
};

const std::array<Game_parameters, 3> _presets = {
	Game_parameters{
		{"Augustus", "Brutus", "Claudius", "Drusilla"},
		{
			maf::Role::ID::peasant,
			maf::Role::ID::racketeer,
			maf::Role::ID::coward
		},
		{maf::Wildcard::ID::village_basic},
		{}
	},
	Game_parameters{
		{"Nine", "Ten", "Jack", "Queen", "King", "Ace"},
		{
			maf::Role::ID::peasant,
			maf::Role::ID::peasant,
			maf::Role::ID::doctor,
			maf::Role::ID::detective,
			maf::Role::ID::dealer,
			maf::Role::ID::musketeer
		},
		{},
		{}
	},
	Game_parameters{
		{"Alice", "Bob", "Charlie", "Daisy", "Eduardo", "Fiona"},
		{
			maf::Role::ID::godfather,
			maf::Role::ID::actor,
			maf::Role::ID::serial_killer,
			maf::Role::ID::village_idiot,
			maf::Role::ID::village_idiot,
			maf::Role::ID::village_idiot
		},
		{},
		{}
	}
};


const maf::Rulebook & maf::Setup_screen::rulebook() const {
	return _rulebook;
}

maf::vector<maf::string> maf::Setup_screen::player_names() const {
	return {_player_names.begin(), _player_names.end()};
}

maf::vector<maf::Role::ID> maf::Setup_screen::rolecard_ids() const {
	vector<Role::ID> v{};
	for (const auto &p: _role_ids) {
		for (std::size_t i{0}; i < p.second; ++i) v.push_back(p.first);
	}
	return v;
}

maf::vector<maf::Wildcard::ID> maf::Setup_screen::wildcard_ids() const {
	vector<Wildcard::ID> v{};
	for (const auto &p: _wildcard_ids) {
		for (std::size_t i{0}; i < p.second; ++i) v.push_back(p.first);
	}
	return v;
}

bool maf::Setup_screen::has_player(string_view name) const {
	for (string_view s: _player_names) {
		if (util::equal_up_to_case(s, name)) {
			return true;
		}
	}
	return false;
}

bool maf::Setup_screen::has_rolecard(string_view alias) const {
	RoleRef r_ref = alias;

	try {
		auto& role = _rulebook.look_up(r_ref);
		auto id = role.id();

		return _role_ids.count(id) != 0 && _role_ids.at(id) != 0;
	} catch (std::out_of_range) {
		throw Rulebook::Missing_role_alias{string(alias)};
	}
}

bool maf::Setup_screen::has_wildcard(string_view alias) const {
	Wildcard::ID id = _rulebook.get_wildcard(alias).id();
	return _wildcard_ids.count(id) != 0 && _wildcard_ids.at(id) != 0;
}

std::size_t maf::Setup_screen::num_players() const {
	return _player_names.size();
}

std::size_t maf::Setup_screen::num_rolecards() const {
	std::size_t n{0};
	for (auto &p: _role_ids) n += p.second;
	return n;
}

std::size_t maf::Setup_screen::num_wildcards() const {
	std::size_t n{0};
	for (auto &p: _wildcard_ids) n += p.second;
	return n;
}

std::size_t maf::Setup_screen::num_cards() const {
	return num_rolecards() + num_wildcards();
}

void maf::Setup_screen::add_player(string_view name) {
	if (util::any_of(name, [](char ch) { return !std::isalnum(ch); })) {
		throw Bad_player_name{string{name}};
	} else if (has_player(name)) {
		throw Player_already_exists{string{name}};
	} else {
		_player_names.insert(string{name});
	}
}

void maf::Setup_screen::add_rolecard(string_view alias) {
	RoleRef r_ref = alias;

	try {
		const Role &r = _rulebook.look_up(r_ref);
		++_role_ids[r.id()];
	} catch (std::out_of_range) {
		throw Rulebook::Missing_role_alias{string(alias)};
	}
}

void maf::Setup_screen::add_wildcard(string_view alias) {
	const Wildcard &w = _rulebook.get_wildcard(alias);
	++_wildcard_ids[w.id()];
}

void maf::Setup_screen::remove_player(string_view name) {
	auto it = util::find_if(_player_names, [&name](string_view s) {
		return util::equal_up_to_case(s, name);
	});

	if (it == _player_names.end()) {
		throw Player_missing{string{name}};
	} else {
		_player_names.erase(it);
	}
}

void maf::Setup_screen::remove_rolecard(string_view alias) {
	RoleRef r_ref = alias;

	try {
		auto& role = _rulebook.look_up(r_ref);
		if (_role_ids[role.id()] == 0) {
			throw Rolecard_unselected{role};
		} else {
			--_role_ids[role.id()];
		}
	} catch (std::out_of_range) {
		throw Rulebook::Missing_role_alias{string(alias)};
	}
}

void maf::Setup_screen::remove_wildcard(string_view alias) {
	const Wildcard &w = _rulebook.get_wildcard(alias);
	if (_wildcard_ids[w.id()] == 0) {
		throw Wildcard_unselected{w};
	} else {
		--_wildcard_ids[w.id()];
	}
}

void maf::Setup_screen::clear_all_players() {
	_player_names.clear();
}

void maf::Setup_screen::clear_rolecards(string_view alias) {
	RoleRef r_ref = alias;

	try {
		auto& role = _rulebook.look_up(r_ref);
		_role_ids[role.id()] = 0;
	} catch (std::out_of_range) {
		throw Rulebook::Missing_role_alias{string(alias)};
	}
}

void maf::Setup_screen::clear_all_rolecards() {
	_role_ids.clear();
}

void maf::Setup_screen::clear_wildcards(string_view alias) {
	const Wildcard &w = _rulebook.get_wildcard(alias);
	_wildcard_ids[w.id()] = 0;
}

void maf::Setup_screen::clear_all_wildcards() {
	_wildcard_ids.clear();
}

void maf::Setup_screen::clear_all_cards() {
	clear_all_rolecards();
	clear_all_wildcards();
}

void maf::Setup_screen::clear_all() {
	clear_all_players();
	clear_all_cards();
}

maf::unique_ptr<maf::Game_log> maf::Setup_screen::begin_pending_game() {
	return make_unique<Game_log>(console(),
								 player_names(),
								 rolecard_ids(),
								 wildcard_ids(),
								 rulebook());
}

maf::unique_ptr<maf::Game_log> maf::Setup_screen::begin_preset(int i) {
	if (i >= 0 && i < std::size(_presets)) {
		Game_parameters params = _presets[i];
		return make_unique<Game_log>(console(),
									 params.player_names,
									 params.role_ids,
									 params.wildcard_ids,
									 params.rulebook);
	} else {
		throw Missing_preset{i};
	}
}

void maf::Setup_screen::do_commands(const vector<string_view> & commands) {
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
		auto& str = commands[1];

		if (auto result = std::from_chars(std::begin(str), std::end(str), i);
			result.ec == std::errc{})
		{
			begin_preset(i);
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

void maf::Setup_screen::set_params(TextParams & params) const {
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
