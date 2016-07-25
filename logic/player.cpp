#include "player.hpp"

maf::Player::Player(std::string name, ID id)
 : _name{name}, _id{id} {

}

const std::string & maf::Player::name() const {
   return _name;
}

maf::Player::ID maf::Player::id() const {
   return _id;
}

const maf::Role & maf::Player::role() const {
   return *_role;
}

void maf::Player::assign_role(const Role &role) {
   _role = &role;

   _fake_role = nullptr;
}

const maf::Wildcard * maf::Player::wildcard() const {
   return _wildcard;
}

void maf::Player::set_wildcard(const Wildcard &wildcard) {
   _wildcard = &wildcard;
}

bool maf::Player::is_alive() const {
   return _is_alive;
}

bool maf::Player::is_dead() const {
   return !_is_alive;
}

maf::Date maf::Player::date_of_death() const {
   return _date_of_death;
}

maf::Time maf::Player::time_of_death() const {
   return _time_of_death;
}

void maf::Player::kill(Date date, Time time) {
   _is_alive = false;
   _is_present = false;
   _date_of_death = date;
   _time_of_death = time;
}

bool maf::Player::is_present() const {
   return _is_present;
}

void maf::Player::leave() {
   _is_present = false;
}

bool maf::Player::has_been_kicked() const {
   return _was_kicked;
}

void maf::Player::kick() {
   leave();
   _was_kicked = true;
}

bool maf::Player::has_been_lynched() const {
   return _was_lynched;
}

void maf::Player::lynch(Date date) {
   kill(date, Time::day);
   _was_lynched = true;
}

const maf::Role * maf::Player::fake_role() const {
   return _fake_role;
}

bool maf::Player::has_fake_role() const {
   return _fake_role;
}

void maf::Player::give_fake_role(const maf::Role &role) {
   _fake_role = &role;
}

void maf::Player::refresh() {
   _compulsory_abilities.clear();

   _lynch_vote = nullptr;
   
   _is_healed = false;
   _is_high = false;
}

const std::vector<maf::Ability> & maf::Player::compulsory_abilities() const {
   return _compulsory_abilities;
}

void maf::Player::add_compulsory_ability(Ability ability) {
   _compulsory_abilities.push_back(ability);
}

void maf::Player::remove_compulsory_ability(Ability ability) {
   for (auto it = _compulsory_abilities.begin(); it != _compulsory_abilities.end(); ++it) {
      if ((*it).id == ability.id) {
         _compulsory_abilities.erase(it);
         return;
      }
   }
   /* fix-me: throw exception if compulsory ability is not currently stored. */
}

const maf::Player * maf::Player::lynch_vote() const {
   return _lynch_vote;
}

void maf::Player::cast_lynch_vote(const Player &target) {
   _lynch_vote = &target;
}

void maf::Player::clear_lynch_vote() {
   _lynch_vote = nullptr;
}

bool maf::Player::has_won_duel() const {
   return _has_won_duel;
}

void maf::Player::win_duel() {
   _has_won_duel = true;
}

bool maf::Player::is_healed() const {
   return _is_healed;
}

void maf::Player::heal() {
   _is_healed = true;
}

bool maf::Player::is_suspicious() const {
   return role().is_suspicious() || _is_high;
}

void maf::Player::give_drugs() {
   _is_high = true;
}

const maf::Player * maf::Player::haunter() const {
   return _haunter;
}

bool maf::Player::is_haunted() const {
   return _haunter;
}

void maf::Player::haunt(const Player &haunter) {
   _haunter = &haunter;
}

bool maf::Player::has_won() const {
   return _has_won;
}

void maf::Player::win() {
   _has_won = true;
}

void maf::Player::lose() {
   _has_won = false;
}
