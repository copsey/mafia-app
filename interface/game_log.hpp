#ifndef MAFIA_GAME_LOG_H
#define MAFIA_GAME_LOG_H

#include <memory>
#include <vector>

#include "event.hpp"

namespace mafia {
   struct Game_log {
      // Create a new game log, managing a game with the given parameters.
      Game_log(const std::vector<std::string> &player_names,
               const std::vector<Role::ID> &role_ids,
               const std::vector<Joker::ID> &joker_ids,
               const Rulebook &rulebook = Rulebook{});

      // The game being managed.
      const Game & game() const;

      // The current event in the log.
      // At least one event will exist at any time.
      const Event & current_event() const {
         return *_log[_log_index];
      }

      // The total number of events that have occurred in the game.
      std::size_t num_events() const {
         return _log.size();
      }

      // Try to continue forward to the next state in the events.
      // If all of the events in the game log have been depleted, then try to
      // advance the game instead; this can result in an exception.
      void advance();

      // Get a string containing a summary of every event which has occurred so
      // far, in chronological order.
      std::string transcript() const;



      void cast_lynch_vote(const Player &caster, const Player &target);
      void clear_lynch_vote(const Player &caster);
      void process_lynch_votes();


   private:
      Game _game;

      std::vector<std::unique_ptr<Event>> _log{};
      decltype(_log)::size_type _log_index{0};

      // Add a new event to the end of the log.
      // Automatically assumes ownership of the event.
      void store_event(Event *event);
   };
}

#endif
