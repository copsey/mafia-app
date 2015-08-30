#ifndef MAFIA_LOGIC_PLAYER
#define MAFIA_LOGIC_PLAYER

#include "../riketi/ref.hpp"

#include "wildcard.hpp"

namespace mafia {
   // An integer representing the number of in-game days which have passed.
   using Date = unsigned;

   // A period of in-game time.
   enum class Time { day, night };

   // A card which can be assigned to a player.
   using Card = std::pair<rkt::ref<const Role>, const Wildcard *>;

   // A player in a game of Mafia.
   struct Player {
      // Creates a new player with the given name, assigning them the given
      // card.
      Player(std::string name, Card card);

      // The name of the player.
      const std::string& name() const;

      // The current role of the player.
      const Role & role() const;
      // The wildcard from which the player obtained their current role, or
      // nullptr if none exists.
      const Wildcard * wildcard() const;
      // Assigns the player the given role.
      void assign(const Role &role);
      // Assigns the player the role on the given card, also storing the
      // wildcard.
      void assign(const Card &card);

      // Whether the player is still alive.
      bool is_alive() const;
      bool is_dead() const;
      // Whether the player is still present in the town.
      bool is_present() const;
      // The time at which the player died, if they are dead.
      Time time_of_death() const;
      // The date on which the player died, if they are dead.
      Date date_of_death() const;

      // Kills the player, through a generic death, on the given date at the given time.
      void kill(Date date, Time time);
      // Kills the player via the mafia's nightly kill.
      void kill_by_mafia(Date date, Time time);
      // Lynches the player.
      void lynch(Date date, Time time);
      // Kills the player in a duel.
      void kill_in_duel(Date date, Time time);
      // Makes the player leave, without killing them.
      void leave();

      // Clears all temporary modifiers from the player, so that they are in a
      // fresh state ready for the next day.
      void refresh();

      // The player's current lynch vote.
      const Player * lynch_vote() const;
      // Sets the player's lynch vote to the given target.
      void cast_lynch_vote(const Player &target);
      // Removes the player's current lynch vote.
      void clear_lynch_vote();

      // Makes the player win a duel.
      void win_duel();
      // Makes the player lose a duel, killing them in the process.
      void lose_duel(Date date, Time time);
      // Whether the player has won a duel at any point.
      bool has_won_duel() const;
      // Stage a duel between this player and the given player.
      // Returns the player that won the duel.
      Player & duel(Player &target, Date date, Time time);

      // Whether the player has won the game they are in.
      bool has_won() const;
      // Makes the player win the game they are in.
      void win();
      // Makes the player lose the game they are in.
      void lose();

   private:
      std::string _name;

      const Role *_role{nullptr};
      const Wildcard *_wildcard{nullptr};

      bool _is_alive{true};
      bool _is_present{true};
      Time _time_of_death;
      Date _date_of_death;

      const Player *_lynch_vote{nullptr};

      bool _has_won_duel{false};

      bool _has_won{false};
   };
}

#endif
