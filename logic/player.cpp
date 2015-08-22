#include "player.hpp"

mafia::Player::Player(std::string name, Card card)
 : _name(name), _role(&static_cast<const Role &>(card.first)),
   _joker(card.second) { }

const std::string & mafia::Player::name() const {
   return _name;
}

const mafia::Role & mafia::Player::role() const {
   return *_role;
}

void mafia::Player::give_role(const mafia::Role &role) {
   _role = &role;
   _joker = nullptr;
}

void mafia::Player::give_card(Card card) {
   _role = &static_cast<const Role &>(card.first);
   _joker = card.second;
}

const mafia::Joker * mafia::Player::joker() const {
   return _joker;
}

bool mafia::Player::is_alive() const {
   return _is_alive;
}

bool mafia::Player::is_present() const {
   return _is_present;
}

void mafia::Player::kill() {
   _is_alive = false;
   _is_present = false;
}

void mafia::Player::lynch() {
   kill();
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

bool mafia::Player::has_won() const {
   return _has_won;
}

void mafia::Player::win() {
   _has_won = true;
}

void mafia::Player::lose() {
   _has_won = false;
}