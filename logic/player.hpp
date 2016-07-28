#ifndef MAFIA_LOGIC_PLAYER
#define MAFIA_LOGIC_PLAYER

#include "time.hpp"
#include "wildcard.hpp"

namespace maf {
   // A player in a game of Mafia.
   struct Player {
      using ID = std::size_t;

      Player(std::string name, ID id);

      // The player's name.
      const std::string & name() const;
      // The unique ID given to the player for the game that they are in.
      ID id() const;

      // The current role of the player.
      const Role & role() const;
      // Assigns the player the given role.
      void assign_role(const Role &role);
      // The wildcard from which the player obtained their current role, or
      // nullptr if none exists.
      const Wildcard * wildcard() const;
      // Sets the wildcard which picked the player's current role.
      void set_wildcard(const Wildcard &wildcard);

      // Whether the player is still alive.
      bool is_alive() const;
      bool is_dead() const;
      // The date on which the player died, if they are dead.
      Date date_of_death() const;
      // The time at which the player died, if they are dead.
      Time time_of_death() const;
      // Kills the player, on the given date and at the given time.
      // The player will count as both dead and not present after killing them.
      void kill(Date date, Time time);

      // Whether the player is still present in the town.
      bool is_present() const;
      // Makes the player leave town, without killing them.
      void leave();

      // Whether the player has been kicked from the game.
      bool has_been_kicked() const;
      // Kick the player from the game they are in.
      void kick();

      // Whether the player has been lynched.
      bool has_been_lynched() const;
      // Lynch the player, on the given date.
      void lynch(Date date);

      // The role that the player must claim to have, or nullptr if none is set.
      const Role * fake_role() const;
      // Whether the player has been given a fake role.
      bool has_fake_role() const;
      // Give the player a fake role.
      void give_fake_role(const Role &role);

      // Clears all temporary modifiers from the player, so that they are in a
      // fresh state ready for the next day.
      void refresh();

      // The abilities that the player must respond to before the game can
      // continue.
      const std::vector<Ability> & compulsory_abilities() const;
      // Add/remove a compulsory ability.
      void add_compulsory_ability(Ability ability);
      void remove_compulsory_ability(Ability ability);

      // The player's current lynch vote.
      const Player * lynch_vote() const;
      // Sets the player's lynch vote to the given target.
      void cast_lynch_vote(const Player &target);
      // Removes the player's current lynch vote.
      void clear_lynch_vote();

      // Whether the player has won a duel at any point.
      bool has_won_duel() const;
      // Makes the player win a duel.
      void win_duel();

      // Whether the player has been healed this night.
      bool is_healed() const;
      // Heals the player for the current night.
      void heal();

      // Whether the player appears as suspicious this night.
      bool is_suspicious() const;
      // Give the player some drugs for the current night.
      // This forces the player to appear as suspicious.
      void give_drugs();

      // The player who is haunting this player, or nullptr if no such player
      // exists.
      const Player * haunter() const;
      // Whether the player is being haunted.
      bool is_haunted() const;
      // Haunt the player with the given haunter.
      void haunt(const Player &haunter);

      // Whether the player has won the game they are in.
      bool has_won() const;
      // Makes the player win/lose the game they are in.
      void win();
      void lose();

   private:
      std::string _name;
      ID _id;

      const Role *_role{nullptr};
      const Wildcard *_wildcard{nullptr};
      
      const Role *_fake_role{nullptr};

      bool _is_alive{true};
      bool _is_present{true};
      bool _was_kicked{false};
      Date _date_of_death;
      Time _time_of_death;

      std::vector<Ability> _compulsory_abilities{};

      const Player *_lynch_vote{nullptr};

      bool _was_lynched{false};
      bool _has_won_duel{false};

      bool _is_healed{false};
      bool _is_high{false};

      const Player *_haunter{nullptr};

      bool _has_won{false};
   };
}

#endif
