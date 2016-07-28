#ifndef MAFIA_LOGIC_PLAYER
#define MAFIA_LOGIC_PLAYER

#include "time.hpp"
#include "wildcard.hpp"

namespace maf {
   struct Player {
      using ID = std::size_t;

      /// Create a player with the given name and ID.
      ///
      /// Each status is set to its default value.
      Player(std::string name, ID id);

      /// The player's name.
      const std::string & name() const;

      /// The unique ID given to the player for the game that they are in.
      ID id() const;

      /// The current role of the player.
      ///
      /// @warning Undefined behaviour if the player has never been assigned a role.
      const Role & role() const;

      /// Assign the given role to the player.
      void assign_role(const Role & role);

      /// The wildcard from which the player obtained their current role, or
      /// `nullptr` if none exists.
      const Wildcard * wildcard() const;

      /// Set the wildcard from which the player obtained their current role.
      void set_wildcard(const Wildcard & wildcard);

      /// Whether the player is still alive.
      bool is_alive() const;

      /// Whether the player has been killed.
      bool is_dead() const;

      /// The date on which the player died.
      ///
      /// @warning Undefined behaviour if the player is still alive.
      Date date_of_death() const;

      /// The time at which the player died.
      ///
      /// @warning Undefined behaviour if the player is still alive.
      Time time_of_death() const;

      /// Kill the player, effectively removing them from the game.
      ///
      /// @param date The date on which the player should be killed.
      /// @param time The time at which the player should be killed.
      void kill(Date date, Time time);

      /// Whether the player is still present in the town.
      bool is_present() const;

      /// Make the player leave the town, without killing them.
      void leave();

      /// Whether the player has been kicked from the game.
      bool has_been_kicked() const;

      /// Kick the player from the game they are in.
      void kick();

      /// Whether the player has been lynched.
      bool has_been_lynched() const;

      /// Lynch the player, effectively removing them from the game.
      ///
      /// @param date The date on which the player should be lynched.
      void lynch(Date date);

      /// The role that the player must claim to have, or `nullptr` if none
      /// has been set.
      const Role * fake_role() const;

      /// Whether the player has been given a fake role.
      bool has_fake_role() const;

      /// Set the given role to be the player's fake role.
      void give_fake_role(const Role & role);

      /// Clear all temporary modifiers from the player, so that they are in a
      /// fresh state ready for the next day.
      void refresh();

      /// The abilities that the player must respond to before the game can
      /// continue.
      const std::vector<Ability> & compulsory_abilities() const;

      /// Add a compulsory ability that the player must respond to before the
      /// game can continue.
      void add_compulsory_ability(Ability ability);

      /// Remove a compulsory ability from the player.
      ///
      /// This should be done when the player has responded to the compulsory
      /// ability.
      void remove_compulsory_ability(Ability ability);

      /// The player's current lynch vote, or nullptr if the player is not
      /// voting to lynch anybody.
      const Player * lynch_vote() const;

      /// Choose a target for the player to vote against in the upcoming lynch.
      void cast_lynch_vote(const Player & target);

      /// Remove the player's current lynch vote.
      void clear_lynch_vote();

      /// Whether the player has won a duel at any point.
      bool has_won_duel() const;

      /// Make the player win a duel.
      void win_duel();

      /// Whether the player has been healed this night.
      bool is_healed() const;

      /// Heal the player for the current night.
      void heal();

      /// Whether the player currently appears as suspicious.
      bool is_suspicious() const;

      /// Give the player some drugs for the current night.
      ///
      /// This forces the player to appear as suspicious.
      void give_drugs();

      /// The ghost which is haunting this player, or nullptr if
      /// none exists.
      const Player * haunter() const;

      /// Whether the player is being haunted by a ghost.
      bool is_haunted() const;

      /// Haunt the player with a ghost!
      void haunt(const Player & haunter);

      /// Whether the player has won the game they are in.
      bool has_won() const;

      /// Make the player win the game they are in.
      void win();

      /// Make the player lose the game they are in.
      void lose();

   private:
      std::string _name;
      ID _id;

      const Role * _role_ptr{nullptr};
      const Wildcard * _wildcard_ptr{nullptr};
      
      const Role * _fake_role_ptr{nullptr};

      bool _alive{true};
      bool _present{true};
      bool _kicked{false};
      Date _date_of_death;
      Time _time_of_death;

      std::vector<Ability> _compulsory_abilities{};

      const Player * _lynch_vote{nullptr};
      const Player * _haunter{nullptr};

      bool _lynched{false};
      bool _won_duel{false};
      bool _won_game{false};

      bool _healed{false};
      bool _high{false};
   };
}

#endif
