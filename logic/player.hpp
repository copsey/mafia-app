#ifndef MAFIA_PLAYER_H
#define MAFIA_PLAYER_H

#include <functional>
#include <string>
#include <utility>

#include "role.hpp"
#include "joker.hpp"

namespace mafia {
   // A role card that can be assigned to a player.
   using Card = std::pair<std::reference_wrapper<const Role>, const Joker *>;


   // A player in a game of Mafia.
   struct Player {
      Player(std::string name, Card card);

      const std::string& name() const;

      const Role & role() const;
      void give_role(const Role &role);
      void give_card(Card card);

      // The joker which gave the player their current role, or nullptr if
      // no such joker exists.
      const Joker * joker() const;

      bool is_alive() const;
      bool is_present() const;

      // Kill the player, through a generic death.
      void kill();

      // Lynch the player.
      void lynch();

      // Clear all temporary modifiers from the player, so that they are in a
      // fresh state ready for the next day.
      void refresh();

      const Player * lynch_vote() const;
      void cast_lynch_vote(const Player &target);
      void clear_lynch_vote();

      bool has_won() const;
      void win();
      void lose();

   private:
      std::string _name;

      const Role *_role{nullptr};
      const Joker *_joker{nullptr};

      bool _is_alive{true};
      bool _is_present{true};
      bool _has_won{false};

      const Player *_lynch_vote{nullptr};
   };
}

#endif
