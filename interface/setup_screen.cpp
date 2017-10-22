#include "../riketi/algorithm.hpp"
#include "../riketi/random.hpp"
#include "../riketi/string.hpp"

#include "console.hpp"
#include "help_screens.hpp"
#include "names.hpp"
#include "setup_screen.hpp"

const std::array<maf::Setup_Screen::Game_parameters, maf::Setup_Screen::num_presets> maf::Setup_Screen::_presets{
   maf::Setup_Screen::Game_parameters{
      {"Augustus", "Brutus", "Claudius", "Drusilla"},
      {maf::Role::ID::peasant, maf::Role::ID::racketeer, maf::Role::ID::coward},
      {maf::Wildcard::ID::village_basic},
      maf::Rulebook{}
   },
   maf::Setup_Screen::Game_parameters{
      {"Nine", "Ten", "Jack", "Queen", "King", "Ace"},
      {maf::Role::ID::peasant, maf::Role::ID::peasant,maf::Role::ID::doctor, maf::Role::ID::detective, maf::Role::ID::dealer, maf::Role::ID::musketeer},
      {},
      maf::Rulebook{}
   }
};

const maf::Rulebook & maf::Setup_Screen::rulebook() const {
   return _rulebook;
}

std::vector<std::string> maf::Setup_Screen::player_names() const {
   return {_player_names.begin(), _player_names.end()};
}

std::vector<maf::Role::ID> maf::Setup_Screen::rolecard_ids() const {
   std::vector<Role::ID> v{};
   for (auto& p: _role_ids) {
      for (std::size_t i{0}; i < p.second; ++i) v.push_back(p.first);
   }
   return v;
}

std::vector<maf::Wildcard::ID> maf::Setup_Screen::wildcard_ids() const {
   std::vector<Wildcard::ID> v{};
   for (const auto &p: _wildcard_ids) {
      for (std::size_t i{0}; i < p.second; ++i) v.push_back(p.first);
   }
   return v;
}

bool maf::Setup_Screen::has_player(const std::string &name) const {
   for (const std::string &s: _player_names) {
      if (rkt::equal_up_to_case(s, name)) {
         return true;
      }
   }
   return false;
}

bool maf::Setup_Screen::has_rolecard(const std::string &alias) const {
   Role::ID id = _rulebook.get_role(alias).id();
   return _role_ids.count(id) != 0 && _role_ids.at(id) != 0;
}

bool maf::Setup_Screen::has_wildcard(const std::string &alias) const {
   Wildcard::ID id = _rulebook.get_wildcard(alias).id();
   return _wildcard_ids.count(id) != 0 && _wildcard_ids.at(id) != 0;
}

std::size_t maf::Setup_Screen::num_players() const {
   return _player_names.size();
}

std::size_t maf::Setup_Screen::num_rolecards() const {
   std::size_t n{0};
   for (auto &p: _role_ids) n += p.second;
   return n;
}

std::size_t maf::Setup_Screen::num_wildcards() const {
   std::size_t n{0};
   for (auto &p: _wildcard_ids) n += p.second;
   return n;
}

std::size_t maf::Setup_Screen::num_cards() const {
   return num_rolecards() + num_wildcards();
}

void maf::Setup_Screen::add_player(const std::string &name) {
   if (rkt::any_of(name, [](char ch) { return !std::isalnum(ch); })) {
      throw Bad_player_name{name};
   } else if (has_player(name)) {
      throw Player_already_exists{name};
   } else {
      _player_names.insert(name);
   }
}

void maf::Setup_Screen::add_rolecard(const std::string &alias) {
   const Role &r = _rulebook.get_role(alias);
   ++_role_ids[r.id()];
}

void maf::Setup_Screen::add_wildcard(const std::string &alias) {
   const Wildcard &w = _rulebook.get_wildcard(alias);
   ++_wildcard_ids[w.id()];
}

void maf::Setup_Screen::remove_player(const std::string &name) {
   auto it = rkt::find_if(_player_names, [&name](const std::string &s) {
      return rkt::equal_up_to_case(s, name);
   });

   if (it == _player_names.end()) {
      throw Player_missing{name};
   } else {
      _player_names.erase(it);
   }
}

void maf::Setup_Screen::remove_rolecard(const std::string &alias) {
   const Role &r = _rulebook.get_role(alias);
   if (_role_ids[r.id()] == 0) {
      throw Rolecard_unselected{r};
   } else {
      --_role_ids[r.id()];
   }
}

void maf::Setup_Screen::remove_wildcard(const std::string &alias) {
   const Wildcard &w = _rulebook.get_wildcard(alias);
   if (_wildcard_ids[w.id()] == 0) {
      throw Wildcard_unselected{w};
   } else {
      --_wildcard_ids[w.id()];
   }
}

void maf::Setup_Screen::clear_all_players() {
   _player_names.clear();
}

void maf::Setup_Screen::clear_rolecards(const std::string &alias) {
   const Role &r = _rulebook.get_role(alias);
   _role_ids[r.id()] = 0;
}

void maf::Setup_Screen::clear_all_rolecards() {
   _role_ids.clear();
}

void maf::Setup_Screen::clear_wildcards(const std::string &alias) {
   const Wildcard &w = _rulebook.get_wildcard(alias);
   _wildcard_ids[w.id()] = 0;
}

void maf::Setup_Screen::clear_all_wildcards() {
   _wildcard_ids.clear();
}

void maf::Setup_Screen::clear_all_cards() {
   clear_all_rolecards();
   clear_all_wildcards();
}

void maf::Setup_Screen::clear_all() {
   clear_all_players();
   clear_all_cards();
}

void maf::Setup_Screen::begin_pending_game() {
   auto& con = this->console();
   con.begin_game(this->player_names(), this->rolecard_ids(), this->wildcard_ids(), this->rulebook());
}

void maf::Setup_Screen::begin_preset(int i) {
   auto& con = this->console();

   if (i >= 0 && i < num_presets) {
      Game_parameters params = _presets[i];
      con.begin_game(params.player_names, params.role_ids, params.wildcard_ids, params.rulebook);
   } else {
      throw Missing_preset{i};
   }
}

void maf::Setup_Screen::begin_random_preset() {
   std::uniform_int_distribution<int> uid{0, static_cast<int>(num_presets) - 1};
   auto i = uid(rkt::random_engine);

   auto& con = this->console();
   auto& params = _presets[i];
   con.begin_game(params.player_names, params.role_ids, params.wildcard_ids, params.rulebook);
}

bool maf::Setup_Screen::handle_commands(const std::vector<std::string> & commands) {
   if (Base_Screen::handle_commands(commands)) return true;

   if (commands_match(commands, {"begin"})) {
      begin_pending_game();
   } else if (commands_match(commands, {"preset"})) {
      begin_random_preset();
   } else if (commands_match(commands, {"preset", ""})) {
      auto& i_str = commands[1];

      try {
         auto i = std::stoi(i_str);
         begin_preset(i);
      } catch (std::logic_error) {
         throw Bad_preset_string{i_str};
      }
   } else if (commands_match(commands, {"add", "p", ""})) {
      auto& pl_name = commands[2];
      add_player(pl_name);
   } else if (commands_match(commands, {"take", "p", ""})) {
      auto& pl_name = commands[2];
      remove_player(pl_name);
   } else if (commands_match(commands, {"clear", "p"})) {
      clear_all_players();
   } else if (commands_match(commands, {"add", "r", ""})) {
      auto& r_alias = commands[2];
      add_rolecard(r_alias);
   } else if (commands_match(commands, {"take", "r", ""})) {
      auto& r_alias = commands[2];
      remove_rolecard(r_alias);
   } else if (commands_match(commands, {"clear", "r", ""})) {
      auto& r_alias = commands[2];
      clear_rolecards(r_alias);
   } else if (commands_match(commands, {"clear", "r"})) {
      clear_all_rolecards();
   } else if (commands_match(commands, {"add", "w", ""})) {
      auto& w_alias = commands[2];
      add_wildcard(w_alias);
   } else if (commands_match(commands, {"take", "w", ""})) {
      auto& w_alias = commands[2];
      remove_wildcard(w_alias);
   } else if (commands_match(commands, {"clear", "w", ""})) {
      auto& w_alias = commands[2];
      clear_wildcards(w_alias);
   } else if (commands_match(commands, {"clear", "w"})) {
      clear_all_wildcards();
   } else if (commands_match(commands, {"clear", "c"})) {
      clear_all_cards();
   } else if (commands_match(commands, {"clear"})) {
      clear_all();
   } else {
      throw Bad_commands{};
   }

   return true;
}

void maf::Setup_Screen::write(std::ostream &os) const {
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

void maf::Setup_Screen::write_players_list(std::ostream &os) const {
   for (auto it = _player_names.begin(), end = _player_names.end(); ; ) {
      os << "   " << *it;

      if (++it == end) {
         break;
      } else {
         os << '\n';
      }
   }
}

void maf::Setup_Screen::write_cards_list(std::ostream &os) const {
   bool write_nl{false};

   for (auto& pair: _role_ids) {
      Role::ID id = pair.first;
      auto n = pair.second;

      if (n > 0) {
         if (write_nl) os << '\n';
         os << "   " << n << " x " << full_name(id);

         write_nl = true;
      }
   }

   /* FIXME: sort wildcards in some definite order (maybe natural Wildcard::ID order is fine?) */
   for (auto& pair: _wildcard_ids) {
      Wildcard::ID id = pair.first;
      auto n = pair.second;

      if (n > 0) {
         if (write_nl) os << '\n';
         os << "   " << n << " x ^c" << alias(id) << "^/ wildcard";

         write_nl = true;
      }
   }
}

maf::Help_Screen * maf::Setup_Screen::get_help_screen() const {
   auto& con = const_cast<Console &>(this->console());
   return new Setup_Help_Screen{con};
}
