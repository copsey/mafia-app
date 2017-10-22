#include <sstream>

#include "../riketi/algorithm.hpp"
#include "../riketi/string.hpp"

#include "console.hpp"
#include "errors.hpp"
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
      throw error::invalid_name();
   } else if (has_player(name)) {
      throw error::duplicate_player();
   } else {
      _player_names.insert(name);
   }
}

void maf::Setup_screen::add_rolecard(const std::string &alias) {
   const Role& rl = _rulebook.get_role(alias);
   ++_role_ids[rl.id()];
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
      throw error::missing_player();
   } else {
      _player_names.erase(it);
   }
}

void maf::Setup_screen::remove_rolecard(const std::string &alias) {
   const Role &r = _rulebook.get_role(alias);
   if (_role_ids[r.id()] == 0) {
      throw error::unselected_card();
   } else {
      --_role_ids[r.id()];
   }
}

void maf::Setup_screen::remove_wildcard(const std::string &alias) {
   const Wildcard &w = _rulebook.get_wildcard(alias);
   if (_wildcard_ids[w.id()] == 0) {
      throw error::unselected_card();
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
   return std::unique_ptr<Game_log>{new Game_log{player_names(), rolecard_ids(), wildcard_ids(), _rulebook}};
}

void maf::Setup_screen::do_commands(const std::vector<std::string>& commands) {
   if (commands_match(commands, {"add", "p", ""})) {
      auto& p_name = commands[2];

      try {
         add_player(p_name);
      } catch (error::invalid_name) {
         std::string esc_p_name = copy_with_escaped_style_codes(p_name);

         std::stringstream err{};
         err << "^h^HInvalid name!^/The text ^c";
         err << esc_p_name;
         err << "^/ is not a valid name for a player.\n\nThe names of players must be at least 1 character long, and can only contain alphanumeric characters (a-z, A-Z, 0-9).";
         throw error::unresolved_input(err);
      } catch (error::duplicate_player) {
         std::string esc_p_name = copy_with_escaped_style_codes(p_name);

         std::stringstream err{};
         err << "^h^HPlayer already exists!^/A player named ^c";
         err << esc_p_name;
         err << "^/ has already been selected to play in the next game.\n(Note that names are case-insensitive.)";
         throw error::unresolved_input(err);
      }
   }
   else if (commands_match(commands, {"take", "p", ""})) {
      auto& p_name = commands[2];

      try {
         remove_player(commands[2]);
      } catch (error::missing_player) {
         std::string esc_p_name = copy_with_escaped_style_codes(p_name);

         std::stringstream err{};
         err << "^h^HMissing player!^/A player named ^c";
         err << esc_p_name;
         err << "^/ could not be found.";
         throw error::unresolved_input(err);
      }
   }
   else if (commands_match(commands, {"clear", "p"})) {
      clear_all_players();
   }
   else if (commands_match(commands, {"add", "r", ""})) {
      auto& r_alias = commands[2];

      try {
         add_rolecard(r_alias);
      } catch (error::missing_role) {
         std::string esc_r_alias = copy_with_escaped_style_codes(r_alias);

         std::stringstream err{};
         err << "^h^HInvalid alias!^/No role could be found whose alias is ^c";
         err << esc_r_alias;
         err << "^/.\nNote that aliases are case-sensitive.\n(enter ^clist r^/ to see a list of each role and its alias.)";
         throw error::unresolved_input(err);
      }
   }
   else if (commands_match(commands, {"take", "r", ""})) {
      auto& r_alias = commands[2];

      try {
         remove_rolecard(r_alias);
      } catch (error::missing_role) {
         std::string esc_r_alias = copy_with_escaped_style_codes(r_alias);

         std::stringstream err{};
         err << "^h^HInvalid alias!^/No role could be found whose alias is ^c";
         err << esc_r_alias;
         err << "^/.\nNote that aliases are case-sensitive.\n(enter ^clist r^/ to see a list of each role and its alias.)";
         throw error::unresolved_input(err);
      } catch (error::unselected_card) {
         std::string esc_r_alias = copy_with_escaped_style_codes(r_alias);

         std::stringstream err{};
         err << "^h^HRolecard not selected!^/No copies of the rolecard with alias ^c";
         err << esc_r_alias;
         err << "^/ have been selected.";
         throw error::unresolved_input(err);
      }
   }
   else if (commands_match(commands, {"clear", "r", ""})) {
      auto& r_alias = commands[2];

      try {
         clear_rolecards(r_alias);
      } catch (error::missing_role) {
         std::string esc_r_alias = copy_with_escaped_style_codes(r_alias);

         std::stringstream err{};
         err << "^h^HInvalid alias!^/No role could be found whose alias is ^c";
         err << esc_r_alias;
         err << "^/.\nNote that aliases are case-sensitive.\n(enter ^clist r^/ to see a list of each role and its alias.)";
         throw error::unresolved_input(err);
      }
   }
   else if (commands_match(commands, {"clear", "r"})) {
      clear_all_rolecards();
   }
   else if (commands_match(commands, {"add", "w", ""})) {
      auto& w_alias = commands[2];

      try {
         add_wildcard(w_alias);
      } catch (error::missing_wildcard) {
         std::string esc_w_alias = copy_with_escaped_style_codes(w_alias);

         std::stringstream err{};
         err << "^h^HInvalid alias!^/No wildcard could be found whose alias is ^c";
         err << esc_w_alias;
         err << "^/.\nNote that aliases are case-sensitive.\n(enter ^clist w^/ to see a list of each wildcard and its alias.)";
         throw error::unresolved_input(err);
      }
   }
   else if (commands_match(commands, {"take", "w", ""})) {
      auto& w_alias = commands[2];

      try {
         remove_wildcard(w_alias);
      } catch (error::missing_wildcard) {
         std::string esc_w_alias = copy_with_escaped_style_codes(w_alias);

         std::stringstream err{};
         err << "^h^HInvalid alias!^/No wildcard could be found whose alias is ^c";
         err << esc_w_alias;
         err << "^/.\nNote that aliases are case-sensitive.\n(enter ^clist w^/ to see a list of each wildcard and its alias.)";
         throw error::unresolved_input(err);
      } catch (error::unselected_card) {
         std::string esc_w_alias = copy_with_escaped_style_codes(w_alias);

         std::stringstream err{};
         err << "^h^HWildcard not selected!^/No copies of the wildcard with alias ^c";
         err << esc_w_alias;
         err << "^/ have been selected.";
         throw error::unresolved_input(err);
      }
   }
   else if (commands_match(commands, {"clear", "w", ""})) {
      auto& w_alias = commands[2];

      try {
         clear_wildcards(w_alias);
      } catch (error::missing_wildcard) {
         std::string esc_w_alias = copy_with_escaped_style_codes(w_alias);

         std::stringstream err{};
         err << "^h^HInvalid alias!^/No wildcard could be found whose alias is ^c";
         err << esc_w_alias;
         err << "^/.\nNote that aliases are case-sensitive.\n(enter ^clist w^/ to see a list of each wildcard and its alias.)";
         throw error::unresolved_input(err);
      }
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
      throw error::bad_commands();
   }
}

void maf::Setup_screen::write(std::ostream &os) const {
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

   /* FIXME: sort wildcards in some definite order (maybe natural Wildcard::ID order is fine?) */
   for (const auto &p: _wildcard_ids) {
      Wildcard::ID id{p.first};
      std::size_t n{p.second};

      if (n > 0) {
         if (write_nl) os << '\n';
         os << "   " << n << " x ^c" << alias(id) << "^/ wildcard";

         write_nl = true;
      }
   }
}
