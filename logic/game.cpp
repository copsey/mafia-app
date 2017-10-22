#include "../riketi/algorithm.hpp"
#include "../riketi/memory.hpp"
#include "../riketi/random.hpp"
#include "../riketi/ref.hpp"

#include "error.hpp"
#include "game.hpp"

maf::Game::Game(const std::vector<Role::ID> & role_ids,
                const std::vector<Wildcard::ID> & wildcard_ids,
                const Rulebook & rulebook)
   : _rulebook{rulebook}
{
   using Card = std::pair<rkt::ref<const Role>, const Wildcard*>;

   std::vector<Card> cards{};

   for (Role::ID id: role_ids) {
      cards.emplace_back(_rulebook.get_role(id), nullptr);
   }

   for (Wildcard::ID id: wildcard_ids) {
      Wildcard& wildcard = _rulebook.get_wildcard(id);
      cards.emplace_back(wildcard.pick_role(_rulebook), &wildcard);
   }

   rkt::shuffle(cards);

   for (decltype(cards)::size_type i = 0; i < cards.size(); ++i) {
      _players.emplace_back(i);

      Player& player = _players.back();
      Card& card = cards[i];

      player.assign_role(*card.first);
      if (card.second) player.set_wildcard(*card.second);
   }

   try_to_end();
}

const maf::Rulebook & maf::Game::rulebook() const {
   return _rulebook;
}

const std::vector<maf::Player> & maf::Game::players() const {
   return _players;
}

std::vector<rkt::ref<const maf::Player>> maf::Game::remaining_players() const {
   std::vector<rkt::ref<const Player>> vec{};
   for (const Player& pl: _players) {
      if (pl.is_present()) vec.emplace_back(pl);
   }
   return vec;
}

std::vector<rkt::ref<const maf::Player>> maf::Game::remaining_players(Alignment alignment) const {
   std::vector<rkt::ref<const Player>> vec{};
   for (const Player& pl: _players) {
      if (pl.is_present() && pl.alignment() == alignment) {
         vec.emplace_back(pl);
      }
   }
   return vec;
}

std::size_t maf::Game::num_players_left() const {
   return rkt::count_if(_players, std::mem_fn(&Player::is_present));
}

std::size_t maf::Game::num_players_left(Alignment alignment) const {
   auto pred = [alignment](const Player& pl) {
      return pl.is_present() && pl.alignment() == alignment;
   };

   return rkt::count_if(_players, pred);
}

maf::Date maf::Game::date() const {
   return _date;
}

maf::Time maf::Game::time() const {
   return _time;
}

bool maf::Game::is_day() const {
   return time() == Time::day;
}

void maf::Game::kick_player(Player::ID id) {
   if (this->has_ended())
      throw error::game_has_ended();
   if (!is_day())
      throw error::bad_timing();

   Player & player = find_player(id);

   if (player.has_been_kicked())
      throw error::repeat_action();

   player.kick();
   try_to_end();
}

const maf::Player* maf::Game::next_lynch_victim() const {
   std::map<const Player*, std::size_t> votes_per_player{};
   std::size_t total_votes = 0;

   for (const Player& voter: _players) {
      if (voter.is_present() && voter.has_lynch_vote()) {
         ++votes_per_player[voter.lynch_vote()];
         ++total_votes;
      }
   }

   auto less_votes = [](const std::pair<const Player*, std::size_t>& p1, const std::pair<const Player*, std::size_t>& p2) {
      return p1.second < p2.second;
   };

   auto it = rkt::max_element(votes_per_player, less_votes);

   if (it != votes_per_player.end() && (2 * it->second > total_votes)) {
      return it->first;
   } else {
      return nullptr;
   }
}

bool maf::Game::lynch_can_occur() const {
   return _lynch_can_occur;
}

void maf::Game::cast_lynch_vote(Player::ID voter_id, Player::ID target_id) {
   if (this->has_ended())
      throw error::game_has_ended();
   if (!lynch_can_occur())
      throw error::bad_timing();

   Player& voter = find_player(voter_id);
   Player& target = find_player(target_id);

   if (!voter.is_present())
      throw error::inactive_voter();
   if (!target.is_present())
      throw error::inactive_target();
   if (rkt::identical(voter, target))
      throw error::identical_players();

   voter.cast_lynch_vote(target);
}

void maf::Game::clear_lynch_vote(Player::ID voter_id) {
   if (this->has_ended())
      throw error::game_has_ended();
   if (!lynch_can_occur())
      throw error::bad_timing();

   Player& voter = find_player(voter_id);

   if (!voter.is_present())
      throw error::inactive_voter();

   voter.clear_lynch_vote();
}

const maf::Player * maf::Game::process_lynch_votes() {
   if (this->has_ended())
      throw error::game_has_ended();
   if (!lynch_can_occur())
      throw error::bad_timing();

   auto victim = const_cast<Player*>(next_lynch_victim());
   if (victim) {
      victim->kill(_date, _time);
      if (victim->is_troll()) _pending_haunters.push_back(victim);
   }

   _lynch_can_occur = false;

   try_to_end();

   return victim;
}

void maf::Game::stage_duel(Player::ID caster_id, Player::ID target_id) {
   Player& caster = find_player(caster_id);
   Player& target = find_player(target_id);

   if (this->has_ended())
      throw error::game_has_ended();
   if (!is_day())
      throw error::bad_timing();
   if (!caster.is_present())
      throw error::inactive_caster();
   if (!target.is_present())
      throw error::inactive_target();
   if (rkt::identical(caster, target))
      throw error::identical_players();
   if (!caster.role().has_ability() || caster.role().ability().id != Ability::ID::duel)
      throw error::unavailable_ability();

   double s = caster.duel_strength() + target.duel_strength();
   /* FIXME: throw exception if s <= 0 */
   double p = caster.duel_strength() / s;

   std::bernoulli_distribution bd{p};

   Player *winner, *loser;
   if (bd(rkt::random_engine)) {
      winner = &caster;
      loser = &target;
   } else {
      winner = &target;
      loser = &caster;
   }

   winner->win_duel();
   if (winner->win_condition() == Win_condition::win_duel) {
      winner->leave();
   }
   loser->kill(_date, _time);

   try_to_end();
}

bool maf::Game::is_night() const {
   return time() == Time::night;
}

void maf::Game::begin_night() {
   if (this->has_ended())
      throw error::game_has_ended();
   if (is_night())
      throw error::bad_timing();
   if (lynch_can_occur())
      throw error::pending_lynch();

   _time = Time::night;

   if (_date > 0) {
      _mafia_can_use_kill = (num_players_left(Alignment::mafia) > 0);

      for (Player& player: _players) {
         if (player.is_present() && player.role().has_ability()) {
            Ability ability = player.role().ability();

            switch (ability.id) {
               case Ability::ID::kill:
               case Ability::ID::heal:
               case Ability::ID::investigate:
               case Ability::ID::peddle:
                  player.add_compulsory_ability(ability);
                  break;

               case Ability::ID::duel:
                  break;
            }
         }
      }
   }

   try_to_end_night();
}

void maf::Game::choose_fake_role(Player::ID player_id, Role::ID fake_role_id) {
   Player& player = find_player(player_id);
   const Role& fake_role = _rulebook.get_role(fake_role_id);

   if (this->has_ended())
      throw error::game_has_ended();
   if (!is_night())
      throw error::bad_timing();
   if (!player.is_role_faker())
      throw error::action_invalid_for_player();
   if (player.has_fake_role())
      throw error::repeat_action();

   player.give_fake_role(fake_role);

   try_to_end_night();
}

bool maf::Game::mafia_can_use_kill() const {
   return _mafia_can_use_kill;
}

void maf::Game::cast_mafia_kill(Player::ID caster_id, Player::ID target_id) {
   if (this->has_ended())
      throw error::game_has_ended();
   if (!is_night())
      throw error::bad_timing();
   if (!mafia_can_use_kill())
      throw error::unavailable_ability();

   Player& caster = find_player(caster_id);
   Player& target = find_player(target_id);

   if (!caster.is_present())
      throw error::inactive_caster();
   if (caster.alignment() != Alignment::mafia)
      throw error::wrong_alignment();
   if (!target.is_present())
      throw error::inactive_target();
   if (rkt::identical(caster, target))
      throw error::identical_players();

   _mafia_can_use_kill = false;
   _mafia_kill_caster = &caster;
   _mafia_kill_target = &target;

   try_to_end_night();
}

void maf::Game::skip_mafia_kill() {
   if (this->has_ended())
      throw error::game_has_ended();
   if (!is_night())
      throw error::bad_timing();
   if (!mafia_can_use_kill())
      throw error::unavailable_ability();

   _mafia_can_use_kill = false;

   try_to_end_night();
}

void maf::Game::cast_kill(Player::ID caster_id, Player::ID target_id) {
   auto is_kill = [](const Ability& abl) {
      return abl.id == Ability::ID::kill;
   };

   if (this->has_ended())
      throw error::game_has_ended();

   Player& caster = find_player(caster_id);
   Player& target = find_player(target_id);

   if (rkt::none_of(caster.compulsory_abilities(), is_kill))
      throw error::unavailable_ability();
   if (!target.is_present())
      throw error::inactive_target();
   if (rkt::identical(caster, target))
      throw error::identical_players();

   _pending_kills.emplace_back(&caster, &target);
   caster.remove_compulsory_ability(Ability{Ability::ID::kill});

   try_to_end_night();
}

void maf::Game::skip_kill(Player::ID caster_id) {
   auto is_kill = [](const Ability & abl) {
      return abl.id == Ability::ID::kill;
   };

   if (this->has_ended())
      throw error::game_has_ended();

   Player& caster = find_player(caster_id);

   if (rkt::none_of(caster.compulsory_abilities(), is_kill))
      throw error::unavailable_ability();

   caster.remove_compulsory_ability(Ability{Ability::ID::kill});

   try_to_end_night();
}

void maf::Game::cast_heal(Player::ID caster_id, Player::ID target_id) {
   auto is_heal = [](const Ability & abl) {
      return abl.id == Ability::ID::heal;
   };

   if (this->has_ended())
      throw error::game_has_ended();

   Player& caster = find_player(caster_id);
   Player& target = find_player(target_id);

   if (rkt::none_of(caster.compulsory_abilities(), is_heal))
      throw error::unavailable_ability();
   if (!target.is_present())
      throw error::inactive_target();
   if (rkt::identical(caster, target))
      throw error::identical_players();

   _pending_heals.emplace_back(&caster, &target);
   caster.remove_compulsory_ability(Ability{Ability::ID::heal});

   try_to_end_night();
}

void maf::Game::skip_heal(Player::ID caster_id) {
   auto is_heal = [](const Ability & abl) {
      return abl.id == Ability::ID::heal;
   };

   if (this->has_ended())
      throw error::game_has_ended();

   Player& caster = find_player(caster_id);

   if (rkt::none_of(caster.compulsory_abilities(), is_heal))
      throw error::unavailable_ability();

   caster.remove_compulsory_ability(Ability{Ability::ID::heal});

   try_to_end_night();
}

void maf::Game::cast_investigate(Player::ID caster_id, Player::ID target_id) {
   auto is_investigate = [](const Ability & abl) {
      return abl.id == Ability::ID::investigate;
   };

   if (this->has_ended())
      throw error::game_has_ended();

   Player& caster = find_player(caster_id);
   Player& target = find_player(target_id);

   if (rkt::none_of(caster.compulsory_abilities(), is_investigate))
      throw error::unavailable_ability();
   if (!target.is_present())
      throw error::inactive_target();
   if (rkt::identical(caster, target))
      throw error::identical_players();

   _pending_investigations.emplace_back(&caster, &target);
   caster.remove_compulsory_ability(Ability{Ability::ID::investigate});

   try_to_end_night();
}

void maf::Game::skip_investigate(Player::ID caster_id) {
   auto is_investigate = [](const Ability & abl) {
      return abl.id == Ability::ID::investigate;
   };

   if (this->has_ended())
      throw error::game_has_ended();

   Player& caster = find_player(caster_id);

   if (rkt::none_of(caster.compulsory_abilities(), is_investigate))
      throw error::unavailable_ability();

   caster.remove_compulsory_ability(Ability{Ability::ID::investigate});

   try_to_end_night();
}

void maf::Game::cast_peddle(Player::ID caster_id, Player::ID target_id) {
   auto is_peddle = [](const Ability & abl) {
      return abl.id == Ability::ID::peddle;
   };

   if (this->has_ended())
      throw error::game_has_ended();

   Player& caster = find_player(caster_id);
   Player& target = find_player(target_id);

   if (rkt::none_of(caster.compulsory_abilities(), is_peddle))
      throw error::unavailable_ability();
   if (!target.is_present())
      throw error::inactive_target();

   _pending_peddles.emplace_back(&caster, &target);
   caster.remove_compulsory_ability(Ability{Ability::ID::peddle});

   try_to_end_night();
}

void maf::Game::skip_peddle(Player::ID caster_id) {
   auto is_peddle = [](const Ability & abl) {
      return abl.id == Ability::ID::peddle;
   };

   if (this->has_ended())
      throw error::game_has_ended();

   Player& caster = find_player(caster_id);

   if (rkt::none_of(caster.compulsory_abilities(), is_peddle))
      throw error::unavailable_ability();

   caster.remove_compulsory_ability(Ability{Ability::ID::peddle});

   try_to_end_night();
}

bool maf::Game::has_ended() const {
   return _has_ended;
}

maf::Player& maf::Game::find_player(Player::ID id) {
   if (id < _players.size()) {
      return _players[id];
   } else {
      throw error::missing_player();
   }
}

bool maf::Game::try_to_end_night() {
   if (!is_night()) {
      return false;
   }

   auto faker_needs_role = [](const Player & pl) {
      return pl.is_present()
         && pl.role().is_role_faker()
         && !pl.has_fake_role();
   };

   if (rkt::any_of(_players, faker_needs_role)) {
      return false;
   }

   auto has_compulsory_ability = [](const Player& pl) {
      return pl.compulsory_abilities().size() > 0;
   };

   if (mafia_can_use_kill()) return false;
   if (rkt::any_of(players(), has_compulsory_ability)) return false;

   for (const auto &pair: _pending_heals) {
      Player &target = *pair.second;
      target.heal();
   }

   for (const auto &pair: _pending_peddles) {
      Player& target = *pair.second;
      target.give_drugs();
   }

   if (_mafia_kill_caster) {
      if (!_mafia_kill_target->is_healed()) {
         _mafia_kill_target->kill(_date, _time);
      }
   }

   // FIXME: make kill strengths work correctly.
   for (const auto &pair: _pending_kills) {
      Player &target = *pair.second;
      if (!target.is_healed()) {
         target.kill(_date, _time);
      }
   }

   for (auto& pair: _pending_investigations) {
      Player& caster = *pair.first;
      Player& target = *pair.second;

      if (caster.is_present()) {
         _investigations.emplace_back(caster, target, _date, target.is_suspicious());
      }
   }

   for (Player* haunter: _pending_haunters) {
      std::vector<Player*> possible_victims{};
      for (Player& player: _players) {
         if (player.is_present() && player.lynch_vote() == haunter)   {
            possible_victims.push_back(&player);
         }
      }

      if (possible_victims.size() > 0) {
         Player& victim = **rkt::pick(possible_victims);
         victim.kill(_date, _time);
         victim.haunt(*haunter);
      }
   }

   ++_date;
   _time = Time::day;

   if (!try_to_end()) {
      _lynch_can_occur = true;

      _mafia_kill_caster = nullptr;
      _mafia_kill_target = nullptr;

      _pending_kills.clear();
      _pending_heals.clear();
      _pending_investigations.clear();
      _pending_peddles.clear();

      _pending_haunters.clear();

      for (Player& player: _players) player.refresh();
   }

   return true;
}

bool maf::Game::try_to_end() {
   if (_has_ended) return true;

   std::size_t num_players_left = 0;
   std::size_t num_village_left = 0;
   std::size_t num_mafia_left = 0;

   bool check_for_village_eliminated = false;
   bool check_for_mafia_eliminated = false;
   bool check_for_last_survivor = false;

   for (const Player& player : _players) {
      if (player.is_present()) {
         ++num_players_left;
         switch (player.alignment()) {
            case Alignment::village:
               ++num_village_left;
               break;

            case Alignment::mafia:
               ++num_mafia_left;
               break;

            default:
               break;
         }

         switch (player.peace_condition()) {
         case Peace_condition::always_peaceful:
            break;

         case Peace_condition::village_eliminated:
            check_for_village_eliminated = true;
            break;

         case Peace_condition::mafia_eliminated:
            check_for_mafia_eliminated = true;
            break;

         case Peace_condition::last_survivor:
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

   for (Player& player: _players) {
      bool has_won = false;

      if (!player.has_been_kicked()) {
         switch (player.win_condition()) {
            case Win_condition::survive:
               has_won = player.is_alive();
               break;

            case Win_condition::village_remains:
               has_won = (num_village_left > 0);
               break;

            case Win_condition::mafia_remains:
               has_won = (num_mafia_left > 0);
               break;

            case Win_condition::be_lynched:
               has_won = player.has_been_lynched();
               break;

            case Win_condition::win_duel:
               has_won = player.has_won_duel();
               break;
         }
      }

      if (has_won) player.win(); else player.lose();
   }

   _has_ended = true;
   return true;
}
