#include <iterator>
#include <sstream>
#include <utility>

#include "../riketi/algorithm.hpp"
#include "../riketi/random.hpp"

#include "game.hpp"

mafia::Game::Game(const std::vector<std::string> &player_names,
                  const std::vector<Role::ID> &role_ids,
                  const std::vector<Wildcard::ID> &wildcard_ids,
                  const Rulebook &rulebook)
: _players{}, _rulebook{rulebook} {
   std::vector<Card> cards{};

   for (Role::ID id: role_ids) {
      cards.emplace_back(_rulebook.get_role(id), nullptr);
   }

   for (Wildcard::ID id: wildcard_ids) {
      Wildcard &wildcard = _rulebook.get_wildcard(id);
      cards.emplace_back(wildcard.pick_role(_rulebook), &wildcard);
   }

   if (player_names.size() != cards.size()) {
      throw Players_to_cards_mismatch{player_names.size(), cards.size()};
   }

   rkt::shuffle(cards);
   auto p_it = player_names.begin(), p_end = player_names.end();
   auto c_it = cards.begin();
   for ( ; p_it != p_end; ++p_it, ++c_it) {
      _players.emplace_back(*p_it, *c_it);
   }

   try_to_end();
}

const mafia::Rulebook & mafia::Game::rulebook() const {
   return _rulebook;
}

const std::vector<mafia::Player> & mafia::Game::players() const {
   return _players;
}

std::vector<rkt::ref<const mafia::Player>> mafia::Game::remaining_players() const {
   std::vector<rkt::ref<const Player>> v{};
   for (const Player &p: _players) {
      if (p.is_present()) v.emplace_back(p);
   }
   return v;
}

std::vector<rkt::ref<const mafia::Player>> mafia::Game::remaining_players(Role::Alignment alignment) const {
   std::vector<rkt::ref<const Player>> v{};
   for (const Player &p: _players) {
      if (p.is_present() && p.role().alignment == alignment) {
         v.emplace_back(p);
      }
   }
   return v;
}

std::size_t mafia::Game::num_players_left() const {
   return rkt::count_if(_players, [](const Player &p) {
      return p.is_present();
   });
}

std::size_t mafia::Game::num_players_left(Role::Alignment alignment) const {
   return rkt::count_if(_players, [alignment](const Player &p) {
      return p.is_present() && p.role().alignment == alignment;
   });
}

mafia::Date mafia::Game::date() const {
   return _date;
}

mafia::Time mafia::Game::time() const {
   return _time;
}

void mafia::Game::begin_day() {
   /* fix-me: throw if time is not night. */
   if (_has_ended) throw Cannot_continue{Cannot_continue::Reason::game_ended};
   if (_mafia_can_use_kill) throw Cannot_continue{Cannot_continue::Reason::mafia_can_use_kill};

   ++_date;
   _time = Time::day;

   _lynch_can_occur = true;

   _mafia_can_use_kill = false;
   _mafia_kill_caster = nullptr;
   _mafia_kill_target = nullptr;

   for (Player &p: _players) p.refresh();
}

void mafia::Game::cast_lynch_vote(const Player &caster, const Player &target) {
   if (_has_ended) throw Lynch_vote_failed{caster, &target, Lynch_vote_failed::Reason::game_ended};
   if (!_lynch_can_occur) throw Lynch_vote_failed{caster, &target, Lynch_vote_failed::Reason::bad_timing};
   if (!caster.is_present()) throw Lynch_vote_failed{caster, &target, Lynch_vote_failed::Reason::caster_is_not_present};
   if (!target.is_present()) throw Lynch_vote_failed{caster, &target, Lynch_vote_failed::Reason::target_is_not_present};
   if (&caster == &target) throw Lynch_vote_failed{caster, &target, Lynch_vote_failed::Reason::caster_is_target};

   const_cast<Player &>(caster).cast_lynch_vote(target);
}

void mafia::Game::clear_lynch_vote(const mafia::Player &caster) {
   if (_has_ended) throw Lynch_vote_failed{caster, nullptr, Lynch_vote_failed::Reason::game_ended};
   if (!_lynch_can_occur) throw Lynch_vote_failed{caster, nullptr, Lynch_vote_failed::Reason::bad_timing};
   if (!caster.is_present()) throw Lynch_vote_failed{caster, nullptr, Lynch_vote_failed::Reason::caster_is_not_present};

   const_cast<Player &>(caster).clear_lynch_vote();
}

const mafia::Player * mafia::Game::next_lynch_victim() const {
   std::map<const Player *, std::size_t> votes_per_player{};
   std::size_t total_votes{0};

   for (const Player &p : _players) {
      if (p.is_present() && p.lynch_vote() != nullptr) {
         const Player &target = *p.lynch_vote();
         ++votes_per_player[&target];
         ++total_votes;
      }
   }

   auto it = rkt::max_element(votes_per_player, [](const std::pair<const Player *, std::size_t> &p1, const std::pair<const Player *, std::size_t> &p2) {
      return p1.second < p2.second;
   });

   if (it != votes_per_player.end() && (2 * it->second > total_votes)) {
      return it->first;
   } else {
      return nullptr;
   }
}

const mafia::Player * mafia::Game::process_lynch_votes() {
   if (_has_ended) throw Lynch_failed{Lynch_failed::Reason::game_ended};
   if (!_lynch_can_occur) throw Lynch_failed{Lynch_failed::Reason::bad_timing};

   auto p = const_cast<mafia::Player *>(next_lynch_victim());
   if (p) p->lynch(_date, _time);
   _lynch_can_occur = false;

   try_to_end();

   return p;
}

bool mafia::Game::lynch_can_occur() const {
   return _lynch_can_occur;
}

bool mafia::Game::lynch_has_occurred() const {
   return !_lynch_can_occur;
}

void mafia::Game::stage_duel(const mafia::Player &caster, const mafia::Player &target) {
   if (_has_ended) throw Duel_failed{caster, target, Duel_failed::Reason::game_ended};
   if (_time != Time::day) throw Duel_failed{caster, target, Duel_failed::Reason::bad_timing};
   if (!caster.is_present()) throw Duel_failed{caster, target, Duel_failed::Reason::caster_is_not_present};
   if (!target.is_present()) throw Duel_failed{caster, target, Duel_failed::Reason::target_is_not_present};
   if (&caster == &target) throw Duel_failed{caster, target, Duel_failed::Reason::caster_is_target};
   if (caster.role().ability.is_empty() || caster.role().ability.get().id != Role::Ability::ID::duel) throw Duel_failed{caster, target, Duel_failed::Reason::caster_has_no_duel};

   const_cast<Player &>(caster).duel(const_cast<Player &>(target), _date, _time);
   try_to_end();
}

void mafia::Game::begin_night() {
   /* fix-me: throw if time is not day. */
   if (_has_ended) throw Cannot_continue{Cannot_continue::Reason::game_ended};
   if (_lynch_can_occur) throw Cannot_continue{Cannot_continue::Reason::lynch_can_occur};

   _time = Time::night;
   if (num_players_left(Role::Alignment::mafia) > 0) {
      _mafia_can_use_kill = true;
   }
}

void mafia::Game::cast_mafia_kill(const mafia::Player &caster, const mafia::Player &target) {
   if (_has_ended) throw Mafia_kill_failed{caster, target, Mafia_kill_failed::Reason::game_ended};
   if (_time != Time::night) throw Mafia_kill_failed{caster, target, Mafia_kill_failed::Reason::bad_timing};
   if (!_mafia_can_use_kill) throw Mafia_kill_failed{caster, target, Mafia_kill_failed::Reason::already_used};
   if (!caster.is_present()) throw Mafia_kill_failed{caster, target, Mafia_kill_failed::Reason::caster_is_not_present};
   if (caster.role().alignment != Role::Alignment::mafia) throw Mafia_kill_failed{caster, target, Mafia_kill_failed::Reason::caster_is_not_in_mafia};
   if (!target.is_present()) throw Mafia_kill_failed{caster, target, Mafia_kill_failed::Reason::target_is_not_present};
   if (&caster == &target) throw Mafia_kill_failed{caster, target, Mafia_kill_failed::Reason::caster_is_target};

   _mafia_can_use_kill = false;
   _mafia_kill_caster = &const_cast<Player &>(caster);
   _mafia_kill_target = &const_cast<Player &>(target);

   try_to_end_night();
}

void mafia::Game::skip_mafia_kill() {
   if (_has_ended) throw Skip_failed{};
   if (_time != Time::night) throw Skip_failed{};
   if (!_mafia_can_use_kill) throw Skip_failed{};

   _mafia_can_use_kill = false;

   try_to_end_night();
}

bool mafia::Game::mafia_can_use_kill() const {
   return _mafia_can_use_kill;
}

bool mafia::Game::has_ended() const {
   return _has_ended;
}

bool mafia::Game::try_to_end_night() {
   if (_time != Time::night) return false;
   if (_mafia_can_use_kill) return false;

   if (_mafia_kill_caster != nullptr) {
      _mafia_kill_target->kill_by_mafia(_date, _time);
   }

   ++_date;
   _time = Time::day;

   if (!try_to_end()) {
      _lynch_can_occur = true;

      _mafia_kill_caster = nullptr;
      _mafia_kill_target = nullptr;

      for (Player &p: _players) p.refresh();
   }

   return true;
}

bool mafia::Game::try_to_end() {
   if (_has_ended) return true;

   std::size_t num_players_left = 0;
   std::size_t num_village_left = 0;
   std::size_t num_mafia_left = 0;

   bool check_for_village_eliminated = false;
   bool check_for_mafia_eliminated = false;
   bool check_for_last_survivor = false;

   for (const Player &player : _players) {
      if (player.is_present()) {
         ++num_players_left;
         switch (player.role().alignment) {
         case Role::Alignment::village:
            ++num_village_left;
            break;

         case Role::Alignment::mafia:
            ++num_mafia_left;
            break;

         default:
            break;
         }

         switch (player.role().peace_condition) {
         case Role::Peace_condition::always_peaceful:
            break;

         case Role::Peace_condition::village_eliminated:
            check_for_village_eliminated = true;
            break;

         case Role::Peace_condition::mafia_eliminated:
            check_for_mafia_eliminated = true;
            break;

         case Role::Peace_condition::last_survivor:
            check_for_last_survivor = true;
            break;
         }
      }
   }

   if ((check_for_village_eliminated && num_village_left > 0)
       || (check_for_mafia_eliminated && num_mafia_left > 0)
       || (check_for_last_survivor && num_players_left > 1)) {
      return false;
   }

   for (Player &player : _players) {
      switch (player.role().win_condition) {
         case Role::Win_condition::survive:
            player.win();
            break;

         case Role::Win_condition::village_remains:
            if (num_village_left > 0) player.win();
            break;

         case Role::Win_condition::mafia_remains:
            if (num_mafia_left > 0) player.win();
            break;

         case Role::Win_condition::win_duel:
            if (player.has_won_duel()) player.win();
            break;
      }
   }

   _has_ended = true;
   return true;
}
