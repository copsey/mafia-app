#ifndef MAFIA_GAME_H
#define MAFIA_GAME_H

#include "player.hpp"
#include "rulebook.hpp"

namespace mafia {
   // An integer representing the number of in-game days which have passed.
   using Date = unsigned;

   // A period of in-game time.
   enum class Time { day, night };


   // A game of Mafia, with a series of players, a rulebook, and various other
   // information.
   // Many of the game's non-const functions will throw exceptions if called
   // with invalid arguments. These are declared in exception.hpp.
   struct Game {
      // Start a new game with the given parameters, assigning each player an
      // initial role.
      // Note that this could lead to the game immediately ending.
      Game(const std::vector<std::string> &player_names,
           const std::vector<Role::ID> &role_ids,
           const std::vector<Joker::ID> &joker_ids,
           const Rulebook &rulebook = Rulebook{});

      const Rulebook & rulebook() const;

      // The participating players, both present and not present.
      const std::vector<Player> & players() const {
         return _players;
      }

      // Whether or not the game has ended.
      bool has_ended() const {
         return _has_ended;
      }

      // A vector containing every member of the Mafia remaining.
      std::vector<const Player *> mafiosi() const;

      // The number of Mafia-aligned players remaining.
      std::size_t num_mafia_left() const;

      Date date() const;
      Time time() const;
      void pass_time();

      void cast_lynch_vote(const Player &caster, const Player &target);
      void clear_lynch_vote(const Player &caster);

      // Submit the lynch votes, and lynch the next victim. Returns the player
      // lynched if applicable, or nullptr if no player was lynched.
      // This can result in the game ending.
      const Player * process_lynch_votes();

      // Determine the player that would be lynched if the lynch votes were to
      // be processed now, or nullptr if no player would be lynched.
      const Player * next_lynch_victim() const;

      // Whether the lynch votes have already been processed today.
      bool lynch_has_occurred() const {
         return !_lynch_can_occur;
      }


   private:
      std::vector<Player> _players;
      Rulebook _rulebook;

      bool _has_ended{false};

      Date _date{0};
      Time _time{Time::night};

      bool _lynch_can_occur{false};

      // Check if the game has ended - that is, if every player still present
      // in the game has had their peace condition resolved.
      // Returns true if the game has ended, in which case the winning players
      // are determined. Returns false if the game has not ended.
      bool try_to_end();
   };
}

#endif
