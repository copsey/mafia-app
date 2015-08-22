#include <iterator>
#include <sstream>
#include <utility>

#include "../riketi/algorithm.hpp"
#include "../riketi/random.hpp"

#include "exception.hpp"
#include "game.hpp"

mafia::Game::Game(const std::vector<std::string> &player_names,
                  const std::vector<Role::ID> &role_ids,
                  const std::vector<Joker::ID> &joker_ids,
                  const Rulebook &rulebook)
: _players{}, _rulebook{rulebook} {
   std::vector<Card> cards{};

   for (Role::ID id: role_ids) {
      cards.emplace_back(_rulebook.get_role(id), nullptr);
   }

   for (Joker::ID id: joker_ids) {
      Joker &joker = _rulebook.get_joker(id);
      cards.emplace_back(joker.choose_role(_rulebook), &joker);
   }

   if (player_names.size() != cards.size()) {
      throw players_to_cards_mismatch{player_names.size(), cards.size()};
   }

   rkt::shuffle(cards);
   for (std::size_t i = 0; i < cards.size(); ++i) {
      _players.emplace_back(player_names[i], cards[i]);
   }

   try_to_end();
}

const mafia::Rulebook & mafia::Game::rulebook() const {
   return _rulebook;
}

mafia::Date mafia::Game::date() const {
   return _date;
}

mafia::Time mafia::Game::time() const {
   return _time;
}

std::vector<const mafia::Player *> mafia::Game::mafiosi() const {
   std::vector<const mafia::Player *> v{};

   for (const Player &p : _players) {
      if (p.is_present() && p.role().alignment() == Role::Alignment::mafia) {
         v.push_back(&p);
      }
   }

   return v;
}

std::size_t mafia::Game::num_mafia_left() const {
   return rkt::count_if(_players, [](const Player &p) {
      return p.is_present() && p.role().alignment() == Role::Alignment::mafia;
   });
}

void mafia::Game::pass_time() {
   switch (_time) {
      case Time::day:
         if (_lynch_can_occur) throw Game_cannot_continue(Game_cannot_continue::Reason::lynch_can_occur);

         _time = Time::night;
         break;

      case Time::night:
         ++_date;
         _time = Time::day;

         _lynch_can_occur = true;

         for (Player &p : _players) p.refresh();
         break;
   }
}

void mafia::Game::cast_lynch_vote(const Player &caster, const Player &target) {
   if (!_lynch_can_occur) throw lynch_vote_failed(caster, &target, lynch_vote_failed::Reason::bad_timing);
   if (!caster.is_present()) throw lynch_vote_failed(caster, &target, lynch_vote_failed::Reason::caster_is_not_present);
   if (!target.is_present()) throw lynch_vote_failed(caster, &target, lynch_vote_failed::Reason::target_is_not_present);
   if (&caster == &target) throw lynch_vote_failed(caster, &target, lynch_vote_failed::Reason::caster_is_target);

   const_cast<Player &>(caster).cast_lynch_vote(target);
}

void mafia::Game::clear_lynch_vote(const mafia::Player &caster) {
   if (!_lynch_can_occur) throw lynch_vote_failed(caster, nullptr, lynch_vote_failed::Reason::bad_timing);
   if (!caster.is_present()) throw lynch_vote_failed(caster, nullptr, lynch_vote_failed::Reason::caster_is_not_present);

   const_cast<Player &>(caster).clear_lynch_vote();
}

const mafia::Player * mafia::Game::process_lynch_votes() {
   if (!_lynch_can_occur) throw badly_timed_lynch();

   auto p = const_cast<mafia::Player *>(next_lynch_victim());
   if (p) p->lynch();
   _lynch_can_occur = false;

   return p;
}

const mafia::Player * mafia::Game::next_lynch_victim() const {
   std::map<const Player *, std::size_t> num_votes_per_player{};
   std::size_t num_votes{0};

   for (const Player &p : _players) {
      if (p.is_present() && p.lynch_vote() != nullptr) {
         const auto &target = *p.lynch_vote();

         if (num_votes_per_player.count(&target) != 0) {
            ++num_votes_per_player[&target];
         } else {
            num_votes_per_player[&target] = 1;
         }

         ++num_votes;
      }
   }

   auto it = rkt::max_element(num_votes_per_player,
                              [](const std::pair<const Player *, std::size_t> &p1,
                                 const std::pair<const Player *, std::size_t> &p2) {
                                 return p1.second < p2.second;
                              });

   if (it != num_votes_per_player.end() && (2 * it->second > num_votes)) {
      return it->first;
   } else {
      return nullptr;
   }
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
         switch (player.role().alignment()) {
         case Role::Alignment::village:
            ++num_village_left;
            break;

         case Role::Alignment::mafia:
            ++num_mafia_left;
            break;

         default:
            break;
         }

         switch (player.role().peace_condition()) {
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

   if ((check_for_village_eliminated && num_village_left != 0)
       || (check_for_mafia_eliminated && num_mafia_left != 0)
       || (check_for_last_survivor && num_players_left > 1)) {
      return false;
   }

   for (Player &player : _players) {
      switch (player.role().win_condition()) {
         case Role::Win_condition::survive:
            player.win();
            break;
      }
   }

   _has_ended = true;
   return true;
}
