#include <sstream>

#include "game_log.hpp"

mafia::Game_log::Game_log(const std::vector<std::string> &player_names,
                          const std::vector<Role::ID> &role_ids,
                          const std::vector<Joker::ID> &joker_ids,
                          const Rulebook &rulebook)
: _game{player_names, role_ids, joker_ids, rulebook} {
   for (const Player &player: _game.players()) {
      store_event(new Player_given_initial_role(
         player, player.role(), player.joker()
      ));
   }

   if (_game.has_ended()) {
      store_event(new Game_ended(_game));
      return;
   }

   store_event(new Time_changed(_game.date(), _game.time()));

   if (_game.num_mafia_left() > 0) {
      store_event(new Mafia_meeting(_game.mafiosi(), true));
   } else {
      store_event(new Boring_night());
   }

   _game.pass_time();
   store_event(new Time_changed(_game.date(), _game.time()));
   store_event(new Town_meeting(_game));
}

const mafia::Game & mafia::Game_log::game() const {
   return _game;
}

void mafia::Game_log::advance() {
   if (_log[_log_index]->advance()) {

   } else if (_log_index + 1 < _log.size()) {
      ++_log_index;
   } else {
      _game.pass_time();
      store_event(new Time_changed(_game.date(), _game.time()));
   }
}

std::string mafia::Game_log::transcript() const {
   std::ostringstream ss{};

   for (const auto &e : _log) {
      auto s = e->summary();
      if (!s.empty()) ss << s << '\n';
   }

   return ss.str();
}

void mafia::Game_log::cast_lynch_vote(const mafia::Player &caster,
                                      const mafia::Player &target) {
   _game.cast_lynch_vote(caster, target);
   store_event(new Lynch_vote_cast(caster, &target));
}

void mafia::Game_log::clear_lynch_vote(const mafia::Player &caster) {
   _game.clear_lynch_vote(caster);
   store_event(new Lynch_vote_cast(caster, nullptr));
}

void mafia::Game_log::process_lynch_votes() {
   auto p =_game.process_lynch_votes();
   store_event(new Player_lynched(p, (p == nullptr) ? nullptr : &p->role()));
   if (_game.has_ended()) store_event(new Game_ended(_game));
}

void mafia::Game_log::store_event(mafia::Event *event) {
   _log.emplace_back(event);
}