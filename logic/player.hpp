#ifndef MAFIA_LOGIC_PLAYER
#define MAFIA_LOGIC_PLAYER

#include "time.hpp"
#include "wildcard.hpp"

namespace maf {
   struct Player {
      using ID = std::size_t;

      /// Create a player with the given ID.
      ///
      /// Each status is set to its default value.
      Player(ID id)
       : _id{id} { }

      /// The unique ID given to the player for the game that they are in.
      ID id() const {
         return _id;
      }

      /// The current role of the player.
      ///
      /// @warning Undefined behaviour if the player has never been assigned
      /// a role.
      const Role & role() const {
         return *_role_ptr;
      }

      /// Assign the given role to the player.
      void assign_role(const Role & role);

      // The player's alignment.
      // This is fully determined by their role.
      //
      // @warning Undefined behaviour if the player hasn't been assigned
      // a role.
      Alignment alignment() const {
         return _role_ptr->alignment();
      }

      // Get the player's win condition.
	  // This is fully determined by their role.
	  //
	  // Note that the behaviour of this function is undefined if the player
	  // has not been assigned a role yet.
	  Win_condition win_condition() const {
	     return _role_ptr->win_condition();
	  }

      // Get the player's peace condition.
      // This is fully determined by their role.
      //
      // Note that the behaviour of this function is undefined if the player
      // has not been assigned a role yet.
      Peace_condition peace_condition() const {
         return _role_ptr->peace_condition();
      }

      // Get the player's duel strength.
      // This is fully determined by their role.
      //
      // Note that the behaviour of this function is undefined if the player
      // has not been assigned a role yet.
      double duel_strength() const {
         return _role_ptr->duel_strength();
      }

      // Whether the player must pretend to have a role that possibly differs from their own.
      //
      // Note that the behaviour of this function is undefined if the player
      // has not been assigned a role yet.
      bool is_role_faker() const {
         return _role_ptr->is_role_faker();
      }

      // Check whether the player is a troll.
      // This is fully determined by their role.
      //
      // Note that the behaviour of this function is undefined if the player
      // has not been assigned a role yet.
      bool is_troll() const {
         return _role_ptr->is_troll();
      }

      /// The wildcard from which the player obtained their current role, or
      /// `nullptr` if none exists.
      const Wildcard * wildcard() const {
         return _wildcard_ptr;
      }

      /// Set the wildcard from which the player obtained their current role.
      void set_wildcard(const Wildcard & wildcard);

      /// Whether the player is still alive.
      bool is_alive() const {
         return _alive;
      }

      /// Whether the player has been killed.
      bool is_dead() const {
         return !_alive;
      }

      /// The date on which the player died.
      ///
      /// @warning Undefined behaviour if the player is still alive.
      Date date_of_death() const {
         return _date_of_death;
      }

      /// The time at which the player died.
      ///
      /// @warning Undefined behaviour if the player is still alive.
      Time time_of_death() const {
         return _time_of_death;
      }

      /// Kill the player, effectively removing them from the game.
      ///
      /// @param date The date on which the player should be killed.
      /// @param time The time at which the player should be killed.
      void kill(Date date, Time time);

      /// Whether the player is still present in the town.
      bool is_present() const {
         return _present;
      }

      /// Make the player leave the town, without killing them.
      void leave();

      /// Whether the player has been kicked from the game.
      bool has_been_kicked() const {
         return _kicked;
      }

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
      const Role * fake_role() const {
         return _fake_role_ptr;
      }

      /// Whether the player has been given a fake role.
      bool has_fake_role() const {
         return _fake_role_ptr;
      }

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
      const Player * lynch_vote() const {
         return _lynch_vote;
      }

      /// Whether the player has a lynch vote.
      bool has_lynch_vote() const {
         return _lynch_vote != nullptr;
      }

      /// Choose a target for the player to vote against in the upcoming lynch.
      void cast_lynch_vote(const Player & target);

      /// Remove the player's current lynch vote.
      void clear_lynch_vote() {
         _lynch_vote = nullptr;
      }

      /// Whether the player has won a duel at any point.
      bool has_won_duel() const {
         return _won_duel;
      }

      /// Make the player win a duel.
      void win_duel() {
         _won_duel = true;
      }

      /// Whether the player has been healed this night.
      bool is_healed() const {
         return _healed;
      }

      /// Heal the player for the current night.
      void heal() {
         _healed = true;
      }

      /// Whether the player currently appears as suspicious.
      bool is_suspicious() const;

      /// Give the player some drugs for the current night.
      ///
      /// This forces the player to appear as suspicious.
      void give_drugs();

      /// The ghost which is haunting this player, or nullptr if
      /// none exists.
      const Player * haunter() const {
         return _haunter;
      }

      /// Whether the player is being haunted by a ghost.
      bool is_haunted() const {
         return _haunter;
      }

      /// Haunt the player with a ghost!
      void haunt(const Player & haunter);

      /// Whether the player has won the game they are in.
      bool has_won() const {
         return _won_game;
      }

      /// Make the player win the game they are in.
      void win() {
         _won_game = true;
      }

      /// Make the player lose the game they are in.
      void lose() {
         _won_game = false;
      }

   private:
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

   /// Check if two players are the same.
   ///
   /// This is fully determined by checking if they have the same ID.
   bool operator==(const Player & p1, const Player & p2);
}

#endif
