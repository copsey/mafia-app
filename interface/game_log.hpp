#ifndef MAFIA_GAME_LOG_H
#define MAFIA_GAME_LOG_H

#include <memory>

#include "events.hpp"

namespace maf {
   struct Game_log {
      // Signifies that no player could be found with the given name.
      struct Player_not_found {
         std::string name;
      };

      // An exception signifying that an ability use has not been programmed in
      // yet.
      struct Unexpected_ability {
         Ability ability;
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

      // Finds the player with the given ID.
      // Throws an exception if no such player could be found.
      const Player & find_player(Player::ID id) const;

      // Finds the player whose name matches the given string, up to case.
      // Throws an exception if no such player could be found.
      const Player & find_player(const std::string &name) const;

      /// Get the name of the given player.
      const std::string & get_name(const Player & player) const;

      /// Get the name of the player with the given ID.
      const std::string & get_name(Player::ID id) const;




      void kick_player(Player::ID id);

      void cast_lynch_vote(Player::ID voter_id, Player::ID target_id);
      void clear_lynch_vote(Player::ID voter_id);
      void process_lynch_votes();

      void stage_duel(Player::ID caster_id, Player::ID target_id);

      void begin_night();

      void choose_fake_role(Player::ID player_id, Role::ID fake_role_id);

      void cast_mafia_kill(Player::ID caster_id, Player::ID target_id);
      void skip_mafia_kill();

      void cast_kill(Player::ID caster_id, Player::ID target_id);
      void skip_kill(Player::ID caster_id);

      void cast_heal(Player::ID caster_id, Player::ID target_id);
      void skip_heal(Player::ID caster_id);

      void cast_investigate(Player::ID caster_id, Player::ID target_id);
      void skip_investigate(Player::ID caster_id);

      void cast_peddle(Player::ID caster_id, Player::ID target_id);
      void skip_peddle(Player::ID caster_id);




   private:
      Game _game;

      std::vector<std::string> _player_names;

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
      void log_investigation_result(Game::Investigation investigation);
      void log_game_ended();
      // Try to log that the night has ended, if this has actually happened.
      void try_to_log_night_ended();
   };
}

#endif
