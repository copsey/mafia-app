#ifndef MAFIA_CONSOLE_H
#define MAFIA_CONSOLE_H

#include <memory>
#include <utility>

#include "game_log.hpp"
#include "styled_string.hpp"

namespace mafia {
   // Signifies that no player could be found with the given name.
   struct player_not_found {
      player_not_found(std::string name): _s{name} { }

      const std::string &name() const { return _s; }

   private:
      std::string _s;
   };


   // Signifies that no role could be found with the given alias.
   struct no_role_for_alias {
      no_role_for_alias(std::string alias): _alias{alias} { }

      const std::string &alias() const { return _alias; }

   private:
      std::string _alias;
   };


   struct Console {
      // An enum representing a question that can be displayed.
      enum class Question { none, confirm_end_game };


      // Construct a new console, displaying the game setup screen.
      Console() {
         show_game_setup();
      }

      // Process the given commands.
      // Returns true if the commands are accepted. In this case,
      // error_message() is empty.
      // Returns false if the commands are invalid for some reason. In this
      // case, output() remains unchanged and error_message() is non-empty.
      bool do_commands(const std::vector<std::string> &commands);

      // Process the given input string, by seperating it into commands
      // delimited by whitespace. For example, the input "add p Brutus" becomes
      // {"add", "p", "Brutus"}.
      // Returns true if the input is accepted. In this case, error_message()
      // is empty.
      // Returns false if the input is invalid for some reason. In this case,
      // output() remains unchanged and error_message() is non-empty.
      bool input(const std::string &input);

      // The most recent output. Never empty.
      const Styled_text & output() const {
         return _output;
      }

      // The most recent error message. Usually empty.
      // The only styles that will ever appear here are help, help_title and
      // command.
      const Styled_text & error_message() const {
         return _error_message;
      }

      // Whether a game is currently in progress.
      bool game_is_in_progress() const {
         return (bool)_game_log_ptr;
      }

      // The event currently being displayed.
      // There will always be an event if a game is in progress.
      const Event & current_event() const {
         /* fix-me: check that a game is actually in progress, i.e.,
          check that _game_log_ptr.get() != nullptr. */
         return _game_log_ptr->current_event();
      }



   private:
      Styled_text _output{};
      Styled_text _error_message{};

      std::unique_ptr<Game_log> _game_log_ptr{};

      std::vector<std::string> _pending_player_names{};
      std::vector<Role::ID> _pending_role_ids{};
      std::vector<Joker::ID> _pending_joker_ids{};
      Rulebook _pending_rulebook{};

      Question _question{Question::none};


      // Decide whether or not the two given vectors of commands match, which is
      // true exactly when both vectors are of the same length and at each
      // position either one of the strings is empty or both strings are
      // identical.
      static bool commands_match(const std::vector<std::string>& v1,
                                 const std::vector<std::string>& v2);

      // Set the output to the result of converting tagged_string into
      // Styled_text.
      void show_output(const std::string &tagged_string);

      void show_game_setup();
      void show_question(Question question);
      void show_current_question();
      void show_event(const Event &event);
      void show_current_event();

      // Update the output to ensure that it is showing the most recent item.
      // This is useful for when a help screen is being displayed.
      void refresh_output();

      void show_error_message(const std::string &tagged_string);
      void clear_error_message();

      // Begin a new game with the given parameters.
      void begin_new_game(const std::vector<std::string> &player_names,
                          const std::vector<Role::ID> &role_ids,
                          const std::vector<Joker::ID> &joker_ids,
                          const Rulebook &rulebook);

      // Begin a new game with the pending parameters.
      void begin_new_game();

      // Terminate the current game in progress, without ending it.
      void terminate_game();

      // Check if a pending player already exists with the given non-case-
      // sensitive name.
      bool has_pending_player(const std::string &name) const;

      // Find the player whose name matches the given string, up to case.
      // Throw an exception if no player could be found.
      // It is implicitly assumed that a game is already in progress.
      const Player & find_player(const std::string &name) const;

      // Find the role in _pending_rulebook with the given alias.
      // Throw an exception if no such role could be found.
      // (note that aliases are case-sensitive.)
      const Role & find_role(const std::string &alias) const;
   };
}

#endif
