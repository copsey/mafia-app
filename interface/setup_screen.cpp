#include "../riketi/algorithm.hpp"
#include "../riketi/string.hpp"

#include "console.hpp"
#include "names.hpp"
#include "setup_screen.hpp"

const maf::Rulebook & maf::Setup_screen::rulebook() const {
   return _rulebook;
}

std::vector<std::string> maf::Setup_screen::player_names() const {
   return {_player_names.begin(), _player_names.end()};
}

std::vector<maf::Role::ID> maf::Setup_screen::rolecard_ids() const {
   std::vector<Role::ID> v{};
   for (const auto &p: _role_ids) {
      for (std::size_t i{0}; i < p.second; ++i) v.push_back(p.first);
   }
   return v;
}

std::vector<maf::Wildcard::ID> maf::Setup_screen::wildcard_ids() const {
   std::vector<Wildcard::ID> v{};
   for (const auto &p: _wildcard_ids) {
      for (std::size_t i{0}; i < p.second; ++i) v.push_back(p.first);
   }
   return v;
}

bool maf::Setup_screen::has_player(const std::string &name) const {
   for (const std::string &s: _player_names) {
      if (rkt::equal_up_to_case(s, name)) {
         return true;
      }
   }
   return false;
}

bool maf::Setup_screen::has_rolecard(const std::string &alias) const {
   Role::ID id = _rulebook.get_role(alias).id();
   return _role_ids.count(id) != 0 && _role_ids.at(id) != 0;
}

bool maf::Setup_screen::has_wildcard(const std::string &alias) const {
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

void maf::Setup_screen::add_player(const std::string &name) {
   if (rkt::any_of(name, [](char ch) { return !std::isalnum(ch); })) {
      throw Bad_player_name{name};
   } else if (has_player(name)) {
      throw Player_already_exists{name};
   } else {
      _player_names.insert(name);
   }
}

void maf::Setup_screen::add_rolecard(const std::string &alias) {
   const Role &r = _rulebook.get_role(alias);
   ++_role_ids[r.id()];
}

void maf::Setup_screen::add_wildcard(const std::string &alias) {
   const Wildcard &w = _rulebook.get_wildcard(alias);
   ++_wildcard_ids[w.id()];
}

void maf::Setup_screen::remove_player(const std::string &name) {
   auto it = rkt::find_if(_player_names, [&name](const std::string &s) {
      return rkt::equal_up_to_case(s, name);
   });

   if (it == _player_names.end()) {
      throw Player_missing{name};
   } else {
      _player_names.erase(it);
   }
}

void maf::Setup_screen::remove_rolecard(const std::string &alias) {
   const Role &r = _rulebook.get_role(alias);
   if (_role_ids[r.id()] == 0) {
      throw Rolecard_unselected{r};
   } else {
      --_role_ids[r.id()];
   }
}

void maf::Setup_screen::remove_wildcard(const std::string &alias) {
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

void maf::Setup_screen::clear_rolecards(const std::string &alias) {
   const Role &r = _rulebook.get_role(alias);
   _role_ids[r.id()] = 0;
}

void maf::Setup_screen::clear_all_rolecards() {
   _role_ids.clear();
}

void maf::Setup_screen::clear_wildcards(const std::string &alias) {
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
   return std::make_unique<Game_log>(player_names(), rolecard_ids(), wildcard_ids(), _rulebook);
}

void maf::Setup_screen::do_commands(const std::vector<std::string> &commands) {
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

void maf::Setup_screen::write(std::ostream &os) const {
   os << "^HSetup^iMafia: a game of deduction and deceit...^h\n\nThis is where you can set up the next game to be played.\n\n";

   if (num_players() == 0) {
      if (num_cards() == 0) {
         os << "You haven't chosen any players or cards yet.";
      } else {
         os << "You haven't added any players yet.\n\nThe following cards will be used:\n";
         write_cards_list(os);
         if (num_cards() >= 3) {
            os << "\n\nSo far, you have selected "
            << num_cards()
            << " cards.";
         }
      }
   } else {
      if (num_cards() == 0) {
         os << "The following players will participate:\n";
         write_players_list(os);
         os << "\n\n";
         if (num_players() >= 3) {
            os << "So far, you have selected "
            << num_players()
            << " players. ";
         }
         os << "You haven't chosen any cards yet.";
      } else {
         os << "The following players will participate:\n";
         write_players_list(os);
         os << "\n\nThey will be assigned the following cards:\n";
         write_cards_list(os);
         os << "\n\nSo far, you have selected "
         << num_players()
         << " player";
         if (num_players() > 1) os << "s";
         os << " and "
         << num_cards()
         << " card";
         if (num_cards() > 1) os << "s";
         os << ".";
      }
   }
   os << "\n\nTo see the commands which can be used on this screen, enter ^chelp^h.";
}

void maf::Setup_screen::write_players_list(std::ostream &os) const {
   for (auto it = _player_names.begin(), end = _player_names.end(); ; ) {
      os << "   " << *it;

      if (++it == end) {
         break;
      } else {
         os << '\n';
      }
   }
}

void maf::Setup_screen::write_cards_list(std::ostream &os) const {
   bool write_nl{false};

   for (const auto &p: _role_ids) {
      Role::ID id{p.first};
      std::size_t n{p.second};

      if (n > 0) {
         if (write_nl) os << '\n';
         os << "   " << n << " x " << full_name(id);

         write_nl = true;
      }
   }

   /* fix-me: sort wildcards in some definite order (maybe natural Wildcard::ID order is fine?) */
   for (const auto &p: _wildcard_ids) {
      Wildcard::ID id{p.first};
      std::size_t n{p.second};

      if (n > 0) {
         if (write_nl) os << '\n';
         os << "   " << n << " x ^c" << alias(id) << "^h" << " wildcard";

         write_nl = true;
      }
   }
}