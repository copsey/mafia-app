#include <sstream>

#include "../riketi/random.hpp"
#include "../riketi/string.hpp"

#include "game_log.hpp"

maf::Game_log::Game_log(const std::vector<std::string> &player_names,
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

   _game.begin_night();
   store_event(new Time_changed{0, Time::night});

   std::vector<Event *> new_events{};

   if (_game.num_players_left(Role::Alignment::mafia) > 0) {
      new_events.push_back(new Mafia_meeting{_game.remaining_players(Role::Alignment::mafia), true});
   }

   for (const Player &player: _game.remaining_players()) {
      if (player.role().is_role_faker && !player.has_fake_role()) {
         new_events.push_back(new Choose_fake_role{player});
      }
   }

   rkt::shuffle(new_events);
   if (new_events.size() == 0) {
      log_boring_night();
   } else {
      for (Event *event: new_events) _log.emplace_back(event);
   }

   try_to_log_night_ended();
}

const maf::Game & maf::Game_log::game() const {
   return _game;
}

void maf::Game_log::advance() {
   if (_log_index + 1 < _log.size()) {
      ++_log_index;
   } else {
      throw Cannot_advance{};
   }
}

void maf::Game_log::do_commands(const std::vector<std::string> &commands) {
   _log[_log_index]->do_commands(commands, *this);
}

void maf::Game_log::write_transcript(std::ostream &os) const {
   for (const auto &event: _log) {
      auto pre_pos = os.tellp();
      event->write_summary(os);
      auto post_pos = os.tellp();

      if (pre_pos != post_pos) os << '\n';
   }
}

const maf::Player & maf::Game_log::find_player(Player::ID id) const {
   for (const Player &player: _game.players()) {
      if (id == player.id()) return player;
   }

   /* fix-me: throw exception in Game_log namespace, or remove this function. */
   throw Game::Player_not_found{id};
}

const maf::Player & maf::Game_log::find_player(const std::string &name) const {
   for (const Player &player: _game.players()) {
      if (rkt::equal_up_to_case(name, player.name())) return player;
   }

   throw Player_not_found{name};
}

void maf::Game_log::kick_player(Player::ID id) {
   _game.kick_player(id);
   const Player &player = find_player(id);
   store_event(new Player_kicked{player, player.role()});

   if (_game.has_ended()) {
      log_game_ended();
   } else {
      log_town_meeting();
   }
}

void maf::Game_log::cast_lynch_vote(Player::ID voter_id, Player::ID target_Id) {
   const Player &voter = find_player(voter_id);
   const Player &target = find_player(target_Id);

   _game.cast_lynch_vote(voter.id(), target.id());
   log_town_meeting(&voter, &target);
}

void maf::Game_log::clear_lynch_vote(Player::ID voter_id) {
   const Player &voter = find_player(voter_id);

   _game.clear_lynch_vote(voter.id());
   log_town_meeting(&voter);
}

void maf::Game_log::process_lynch_votes() {
   const Player *victim = _game.process_lynch_votes();
   log_lynch_result(victim);

   if (_game.has_ended()) {
      log_game_ended();
   } else {
      log_town_meeting();
   }
}

void maf::Game_log::stage_duel(Player::ID caster_id, Player::ID target_id) {
   const Player &caster = find_player(caster_id);
   const Player &target = find_player(target_id);

   _game.stage_duel(caster.id(), target.id());
   log_duel_result(caster, target);

   if (_game.has_ended()) {
      log_game_ended();
   } else {
      log_town_meeting();
   }
}

void maf::Game_log::begin_night() {
   _game.begin_night();
   log_time_changed();

   std::vector<Event *> new_events{};

   if (_game.mafia_can_use_kill()) {
      new_events.push_back(new Mafia_meeting{_game.remaining_players(Role::Alignment::mafia), false});
   }

   /* fix-me: minimise number of events when a player has multiple things to do this night. */
   for (const Player &player: _game.players()) {
      for (Role::Ability ability: player.compulsory_abilities()) {
         switch (ability.id) {
            case Role::Ability::ID::kill:
               new_events.push_back(new Kill_use{player});
               break;

            case Role::Ability::ID::heal:
               new_events.push_back(new Heal_use{player});
               break;

            case Role::Ability::ID::investigate:
               new_events.push_back(new Investigate_use{player});
               break;

            case Role::Ability::ID::peddle:
               new_events.push_back(new Peddle_use{player});
               break;

            default:
               throw Unexpected_ability{ability};
         }
      }
   }

   rkt::shuffle(new_events);

   if (new_events.size() == 0) {
      log_boring_night();
   } else {
      for (Event *event: new_events) _log.emplace_back(event);
   }

   try_to_log_night_ended();
}

void maf::Game_log::choose_fake_role(Player::ID player_id, Role::ID fake_role_id) {
   _game.choose_fake_role(player_id, fake_role_id);
   try_to_log_night_ended();
}

void maf::Game_log::cast_mafia_kill(Player::ID caster_id, Player::ID target_id) {
   _game.cast_mafia_kill(caster_id, target_id);
   try_to_log_night_ended();
}

void maf::Game_log::skip_mafia_kill() {
   _game.skip_mafia_kill();
   try_to_log_night_ended();
}

void maf::Game_log::cast_kill(Player::ID caster_id, Player::ID target_id) {
   _game.cast_kill(caster_id, target_id);
   try_to_log_night_ended();
}

void maf::Game_log::skip_kill(Player::ID caster_id) {
   _game.skip_kill(caster_id);
   try_to_log_night_ended();
}

void maf::Game_log::cast_heal(Player::ID caster_id, Player::ID target_id) {
   _game.cast_heal(caster_id, target_id);
   try_to_log_night_ended();
}

void maf::Game_log::skip_heal(Player::ID caster_id) {
   _game.skip_heal(caster_id);
   try_to_log_night_ended();
}

void maf::Game_log::cast_investigate(Player::ID caster_id, Player::ID target_id) {
   _game.cast_investigate(caster_id, target_id);
   try_to_log_night_ended();
}

void maf::Game_log::skip_investigate(Player::ID caster_id) {
   _game.skip_investigate(caster_id);
   try_to_log_night_ended();
}

void maf::Game_log::cast_peddle(Player::ID caster_id, Player::ID target_id) {
   _game.cast_peddle(caster_id, target_id);
   try_to_log_night_ended();
}

void maf::Game_log::skip_peddle(Player::ID caster_id) {
   _game.skip_peddle(caster_id);
   try_to_log_night_ended();
}

void maf::Game_log::store_event(Event *event) {
   _log.emplace_back(event);
}

void maf::Game_log::log_time_changed() {
   store_event(new Time_changed{_game.date(), _game.time()});
}

void maf::Game_log::log_obituary(Date date) {
   std::vector<rkt::ref<const Player>> deaths{};
   for (const Player &p: _game.players()) {
      if (p.is_dead() && p.time_of_death() == Time::night && p.date_of_death() == date) {
         deaths.emplace_back(p);
      }
   }
   store_event(new Obituary{deaths});
}

void maf::Game_log::log_town_meeting(const Player *recent_vote_caster, const Player *recent_vote_target) {
   store_event(new Town_meeting{_game.remaining_players(), _game.date(), _game.lynch_can_occur(), _game.next_lynch_victim(), recent_vote_caster, recent_vote_target});
}

void maf::Game_log::log_lynch_result(const Player *victim) {
   store_event(new Lynch_result{victim, victim ? &victim->role() : nullptr});
}

void maf::Game_log::log_duel_result(const Player &caster, const Player &target) {
   store_event(new Duel_result{caster, target});
}

void maf::Game_log::log_boring_night() {
   store_event(new Boring_night{});
}

void maf::Game_log::log_investigation_result(Game::Investigation investigation) {
   store_event(new Investigation_result{investigation});
}

void maf::Game_log::log_game_ended() {
   store_event(new Game_ended{_game});
}

void maf::Game_log::try_to_log_night_ended() {
   if (_game.time() == Time::day) {
      auto investigations = _game.investigations();
      for (Game::Investigation investigation: investigations) {
         log_investigation_result(investigation);
      }

      log_time_changed();

      if (_game.date() > 1) {
         log_obituary(_game.date() - 1);
      }

      if (_game.has_ended()) {
         log_game_ended();
      } else {
         log_town_meeting();
      }
   }
}