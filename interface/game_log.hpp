#ifndef MAFIA_GAME_LOG_H
#define MAFIA_GAME_LOG_H

#include <memory>

#include "events.hpp"

namespace mafia {
   struct Game_log {
      // Signifies that no player could be found with the given name.
      struct Player_not_found {
         std::string name;
      };

      // Signifies that there are no more events to advance to.
      struct Cannot_advance { };

      // Creates a new game log, managing a game with the given parameters.
      Game_log(const std::vector<std::string> &player_names,
               const std::vector<Role::ID> &role_ids,
               const std::vector<Wildcard::ID> &wildcard_ids,
               const Rulebook &rulebook = Rulebook{});

      // The game being managed.
      const Game & game() const;

      // The current event in the log.
      // There will always be at least one event.
      const Event & current_event() const {
         return *_log[_log_index];
      }

      // The total number of events that have occurred in the game.
      std::size_t num_events() const {
         return _log.size();
      }

      // Tries to continue forward to the next state in the events.
      // Throws an exception if all of the events in the game log have been
      // depleted.
      void advance();

      // Handles the given commands, by passing them to the current event.
      // Throws an exception if the commands couldn't be handled.
      void do_commands(const std::vector<std::string> &commands);

      // Writes a transcript to os, containing a summary of every event which
      // has occurred so far, in chronological order.
      void write_transcript(std::ostream &os) const;

      // Finds the player whose name matches the given string, up to case.
      // Throws an exception if no such player could be found.
      const Player & find_player(const std::string &name) const;




      void begin_day();

      void cast_lynch_vote(const Player &caster, const Player &target);
      void clear_lynch_vote(const Player &caster);
      void process_lynch_votes();

      void stage_duel(const Player &caster, const Player &target);

      void begin_night();

      void cast_mafia_kill(const Player &caster, const Player &target);
      void skip_mafia_kill();





   private:
      Game _game;

      std::vector<std::unique_ptr<Event>> _log{};
      decltype(_log)::size_type _log_index{0};

      // Adds an event to the end of the log.
      // Automatically assumes ownership of the event.
      void store_event(Event *event);
      // Adds the specified event to the end of the log.
      void log_time_changed();
      void log_obituary(Date date);
      void log_town_meeting(const Player *recent_vote_caster = nullptr, const Player *recent_vote_target = nullptr);
      void log_lynch_result(const Player *victim);
      void log_duel_result(const Player &caster, const Player &target);
      void log_boring_night();
      void log_game_ended();
      // Try to log that the night has ended, if this has actually happened.
      void try_to_log_night_ended();
   };
}

#endif
