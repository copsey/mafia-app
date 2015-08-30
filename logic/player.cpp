#include "../riketi/random.hpp"

#include "player.hpp"

mafia::Player::Player(std::string name, Card card)
: _name{name} {
   assign(card);
}

const std::string & mafia::Player::name() const {
   return _name;
}

const mafia::Role & mafia::Player::role() const {
   return *_role;
}

const mafia::Wildcard * mafia::Player::wildcard() const {
   return _wildcard;
}

void mafia::Player::assign(const mafia::Role &role) {
   _role = &role;
   _wildcard = nullptr;
}

void mafia::Player::assign(const Card &card) {
   assign(card.first);
   _wildcard = card.second;
}

bool mafia::Player::is_alive() const {
   return _is_alive;
}

bool mafia::Player::is_dead() const {
   return !_is_alive;
}

bool mafia::Player::is_present() const {
   return _is_present;
}

mafia::Time mafia::Player::time_of_death() const {
   return _time_of_death;
}

mafia::Date mafia::Player::date_of_death() const {
   return _date_of_death;
}

void mafia::Player::kill(Date date, Time time) {
   _is_alive = false;
   _is_present = false;
   _date_of_death = date;
   _time_of_death = time;
}

void mafia::Player::kill_by_mafia(Date date, Time time) {
   kill(date, time);
}

void mafia::Player::lynch(Date date, Time time) {
   kill(date, time);
}

void mafia::Player::kill_in_duel(Date date, Time time) {
   kill(date, time);
}

void mafia::Player::leave() {
   _is_present = false;
}

void mafia::Player::refresh() {
   _lynch_vote = nullptr;
}

const mafia::Player * mafia::Player::lynch_vote() const {
   return _lynch_vote;
}

void mafia::Player::cast_lynch_vote(const mafia::Player &target) {
   _lynch_vote = &target;
}

void mafia::Player::clear_lynch_vote() {
   _lynch_vote = nullptr;
}

void mafia::Player::win_duel() {
   _has_won_duel = true;
   if (role().win_condition == Role::Win_condition::win_duel) {
      leave();
   }
}

void mafia::Player::lose_duel(Date date, Time time) {
   kill_in_duel(date, time);
}

bool mafia::Player::has_won_duel() const {
   return _has_won_duel;
}

mafia::Player & mafia::Player::duel(Player &target, Date date, Time time) {
   double s = role().duel_strength + target.role().duel_strength;
   /* fix-me: throw exception if s <= 0 */
   double p = role().duel_strength / s;

   std::bernoulli_distribution bd{p};

   Player *winner, *loser;
   if (bd(rkt::random_engine)) {
      winner = this;
      loser = &target;
   } else {
      winner = &target;
      loser = this;
   }

   winner->win_duel();
   loser->lose_duel(date, time);

   return *winner;
}

bool mafia::Player::has_won() const {
   return _has_won;
}

void mafia::Player::win() {
   _has_won = true;
}

void mafia::Player::lose() {
   _has_won = false;
}