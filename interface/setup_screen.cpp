#include "../riketi/algorithm.hpp"
#include "../riketi/string.hpp"

#include "command.hpp"
#include "console.hpp"
#include "names.hpp"
#include "setup_screen.hpp"


using std::ostream;
using size = std::size_t;
using std::string;
using std::string_view;
using std::vector;


const maf::Rulebook & maf::Setup_screen::rulebook() const {
	return _rulebook;
}

vector<string> maf::Setup_screen::player_names() const {
	return {_player_names.begin(), _player_names.end()};
}

vector<maf::Role::ID> maf::Setup_screen::rolecard_ids() const {
	vector<Role::ID> v{};
	for (const auto &p: _role_ids) {
		for (size i{0}; i < p.second; ++i) v.push_back(p.first);
	}
	return v;
}

vector<maf::Wildcard::ID> maf::Setup_screen::wildcard_ids() const {
	vector<Wildcard::ID> v{};
	for (const auto &p: _wildcard_ids) {
		for (size i{0}; i < p.second; ++i) v.push_back(p.first);
	}
	return v;
}

bool maf::Setup_screen::has_player(string_view name) const {
	for (string_view s: _player_names) {
		if (rkt::equal_up_to_case(s, name)) {
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

size maf::Setup_screen::num_players() const {
	return _player_names.size();
}

size maf::Setup_screen::num_rolecards() const {
	size n{0};
	for (auto &p: _role_ids) n += p.second;
	return n;
}

size maf::Setup_screen::num_wildcards() const {
	size n{0};
	for (auto &p: _wildcard_ids) n += p.second;
	return n;
}

size maf::Setup_screen::num_cards() const {
	return num_rolecards() + num_wildcards();
}

void maf::Setup_screen::add_player(string_view name) {
	if (rkt::any_of(name, [](char ch) { return !std::isalnum(ch); })) {
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
	auto it = rkt::find_if(_player_names, [&name](string_view s) {
		return rkt::equal_up_to_case(s, name);
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

std::unique_ptr<maf::Game_log> maf::Setup_screen::new_game_log() const {
	return std::unique_ptr<Game_log>{new Game_log{player_names(), rolecard_ids(), wildcard_ids(), _rulebook}};
}

void maf::Setup_screen::do_commands(const vector<string_view> & commands) {
	if (commands_match(commands, {"add", "p", ""})) {
		add_player(commands[2]);
	}
	else if (commands_match(commands, {"take", "p", ""})) {
		remove_player(commands[2]);
	}
	else if (commands_match(commands, {"clear", "p"})) {
		clear_all_players();
	}
	else if (commands_match(commands, {"add", "r", ""})) {
		add_rolecard(commands[2]);
	}
	else if (commands_match(commands, {"take", "r", ""})) {
		remove_rolecard(commands[2]);
	}
	else if (commands_match(commands, {"clear", "r", ""})) {
		clear_rolecards(commands[2]);
	}
	else if (commands_match(commands, {"clear", "r"})) {
		clear_all_rolecards();
	}
	else if (commands_match(commands, {"add", "w", ""})) {
		add_wildcard(commands[2]);
	}
	else if (commands_match(commands, {"take", "w", ""})) {
		remove_wildcard(commands[2]);
	}
	else if (commands_match(commands, {"clear", "w", ""})) {
		clear_wildcards(commands[2]);
	}
	else if (commands_match(commands, {"clear", "w"})) {
		clear_all_wildcards();
	}
	else if (commands_match(commands, {"clear", "c"})) {
		clear_all_cards();
	}
	else if (commands_match(commands, {"clear"})) {
		clear_all();
	}
	else {
		throw Bad_commands{};
	}
}

void maf::Setup_screen::write(ostream &os) const {
	os << "^h^TSetup^/^iMafia: a game of deduction and deceit...^/\n\nThis is where you can set up the next game to be played.\n\n";

	if (num_players() == 0) {
		if (num_cards() == 0) {
			os << "You haven't chosen any players or cards yet.";
		} else {
			os << "You haven't added any players yet.\n\nThe following cards will be used:\n";
			write_cards_list(os);
			if (num_cards() >= 3) {
				os << "\n\nSo far, you have selected ";
				os << num_cards();
				os << " cards.";
			}
		}
	} else {
		if (num_cards() == 0) {
			os << "The following players will participate:\n";
			write_players_list(os);
			os << "\n\n";
			if (num_players() >= 3) {
				os << "So far, you have selected ";
				os << num_players();
				os << " players. ";
			}
			os << "You haven't chosen any cards yet.";
		} else {
			os << "The following players will participate:\n";
			write_players_list(os);

			os << "\n\nThey will be assigned the following cards:\n";
			write_cards_list(os);

			os << "\n\nSo far, you have selected ";
			os << num_players();
			os << " player" << ((num_players() != 1) ? "s" : "");
			os << " and ";
			os << num_cards();
			os << " card" << ((num_cards() != 1) ? "s" : "");
			os << ".";
		}
	}
	os << "\n\nTo see the commands which can be used on this screen, enter ^chelp^/.";
}

void maf::Setup_screen::write_players_list(ostream &os) const {
	for (auto it = _player_names.begin(), end = _player_names.end(); ; ) {
		os << "   " << *it;

		if (++it == end) {
			break;
		} else {
			os << '\n';
		}
	}
}

void maf::Setup_screen::write_cards_list(ostream &os) const {
	bool write_nl{false};

	for (const auto &p: _role_ids) {
		Role::ID id{p.first};
		size n{p.second};

		if (n > 0) {
			if (write_nl) os << '\n';
			os << "   " << n << " x " << full_name(id);

			write_nl = true;
		}
	}

	/* FIXME: sort wildcards in some definite order (maybe natural Wildcard::ID order is fine?) */
	for (const auto &p: _wildcard_ids) {
		Wildcard::ID id{p.first};
		size n{p.second};

		if (n > 0) {
			if (write_nl) os << '\n';
			os << "   " << n << " x ^c" << alias(id) << "^/ wildcard";

			write_nl = true;
		}
	}
}
