#include <sstream>

#include "../riketi/string.hpp"

#include "game_log.hpp"

mafia::Game_log::Game_log(const std::vector<std::string> &player_names,
                          const std::vector<Role::ID> &role_ids,
                          const std::vector<Wildcard::ID> &wildcard_ids,
                          const Rulebook &rulebook)
: _game{player_names, role_ids, wildcard_ids, rulebook} {
   for (const Player &player: _game.players()) {
      store_event(new Player_given_initial_role{
         player, player.role(), player.wildcard()
      });
   }

   if (_game.has_ended()) {
      store_event(new Game_ended(_game));
      return;
   }

   log_time_changed();

   if (_game.num_players_left(Role::Alignment::mafia) > 0) {
      store_event(new Mafia_meeting{_game.remaining_players(Role::Alignment::mafia), _game.date(), true});
   } else {
      log_boring_night();
   }

   _game.begin_day();
   log_time_changed();
   log_town_meeting();
}

const mafia::Game & mafia::Game_log::game() const {
   return _game;
}

void mafia::Game_log::advance() {
   if (_log_index + 1 < _log.size()) {
      ++_log_index;
   } else {
      throw Cannot_advance{};
   }
}

void mafia::Game_log::do_commands(const std::vector<std::string> &commands) {
   _log[_log_index]->do_commands(commands, *this);
}

void mafia::Game_log::write_transcript(std::ostream &os) const {
   for (const auto &event: _log) {
      auto pre_pos = os.tellp();
      event->write_summary(os);
      auto post_pos = os.tellp();

      if (pre_pos != post_pos) os << '\n';
   }
}

const mafia::Player & mafia::Game_log::find_player(const std::string &s) const {
   for (const Player &p: _game.players()) {
      if (rkt::equal_up_to_case(s, p.name())) return p;
   }

   throw Player_not_found{s};
}

void mafia::Game_log::begin_day() {
   _game.begin_day();
   log_time_changed();
   log_town_meeting();
}

void mafia::Game_log::cast_lynch_vote(const mafia::Player &caster,
                                      const mafia::Player &target) {
   _game.cast_lynch_vote(caster, target);
   log_town_meeting(&caster, &target);
}

void mafia::Game_log::clear_lynch_vote(const mafia::Player &caster) {
   _game.clear_lynch_vote(caster);
   log_town_meeting(&caster);
}

void mafia::Game_log::process_lynch_votes() {
   const Player *victim = _game.process_lynch_votes();
   log_lynch_result(victim);

   if (_game.has_ended()) {
      log_game_ended();
   } else {
      log_town_meeting();
   }
}

void mafia::Game_log::stage_duel(const mafia::Player &caster, const mafia::Player &target) {
   _game.stage_duel(caster, target);
   log_duel_result(caster, target);

   if (_game.has_ended()) {
      log_game_ended();
   } else {
      log_town_meeting();
   }
}

void mafia::Game_log::begin_night() {
   _game.begin_night();
   log_time_changed();

   if (_game.mafia_can_use_kill()) {
      store_event(new Mafia_meeting{_game.remaining_players(Role::Alignment::mafia), _game.date(), false});
   }

   /* fix-me: make sure to shuffle order of night events. */

   /* fix-me: don't forget to add a "Boring Night" event when appropriate, and
    skip straight to the next day. */
}

void mafia::Game_log::cast_mafia_kill(const mafia::Player &caster, const mafia::Player &target) {
   _game.cast_mafia_kill(caster, target);
   try_to_log_night_ended();
}

void mafia::Game_log::skip_mafia_kill() {
   _game.skip_mafia_kill();
   try_to_log_night_ended();
}

void mafia::Game_log::store_event(mafia::Event *event) {
   _log.emplace_back(event);
}

void mafia::Game_log::log_time_changed() {
   store_event(new Time_changed{_game.date(), _game.time()});
}

void mafia::Game_log::log_obituary(Date date) {
   std::vector<rkt::ref<const Player>> deaths{};
   for (const Player &p: _game.players()) {
      if (p.is_dead() && p.time_of_death() == Time::night && p.date_of_death() == date) {
         deaths.emplace_back(p);
      }
   }
   store_event(new Obituary{deaths});
}

void mafia::Game_log::log_town_meeting(const mafia::Player *recent_vote_caster, const mafia::Player *recent_vote_target) {
   store_event(new Town_meeting{_game.remaining_players(), _game.date(), _game.lynch_can_occur(), recent_vote_caster, recent_vote_target});
}

void mafia::Game_log::log_lynch_result(const mafia::Player *victim) {
   store_event(new Lynch_result{victim, victim ? &victim->role() : nullptr});
}

void mafia::Game_log::log_duel_result(const mafia::Player &caster, const mafia::Player &target) {
   store_event(new Duel_result{caster, target});
}

void mafia::Game_log::log_boring_night() {
   store_event(new Boring_night{_game.date()});
}

void mafia::Game_log::log_game_ended() {
   store_event(new Game_ended{_game});
}

void mafia::Game_log::try_to_log_night_ended() {
   if (_game.time() == Time::day) {
      log_time_changed();
      log_obituary(_game.date() - 1);

      if (_game.has_ended()) {
         log_game_ended();
      } else {
         log_town_meeting();
      }
   }
}