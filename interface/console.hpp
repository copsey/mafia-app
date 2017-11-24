#ifndef MAFIA_CONSOLE_H
#define MAFIA_CONSOLE_H

#include <array>
#include <deque>
#include <memory>
#include <string_view>
#include <utility>

#include "../riketi/algorithm.hpp"

#include "game_log.hpp"
#include "help_screens.hpp"
#include "questions.hpp"
#include "setup_screen.hpp"
#include "styled_string.hpp"

namespace maf {
   class Game_Screen;

   // Signifies that there is no game in progress at the moment.
   struct No_game_in_progress { };

   // Signifies that a new game cannot begin right now.
   struct Begin_game_failed {
      enum class Reason {
         game_already_in_progress
      };

      Reason reason;
   };


   struct Console {
      // Creates a new console, displaying some initial output.
      Console();

      // Processes the given commands.
      // Returns true if the commands were accepted. In this case,
      // error_message() is empty.
      // Returns false if the commands are invalid for some reason. In this
      // case, output() remains unchanged and error_message() is non-empty.
      bool do_commands(const std::vector<std::string_view> & commands);
      // Process the given input string, by seperating it into commands
      // delimited by whitespace. For example, the input "add p Brutus" becomes
      // {"add", "p", "Brutus"}.
      // Returns true if the input was accepted. In this case, error_message()
      // is empty.
      // Returns false if the input is invalid for some reason. In this case,
      // output() remains unchanged and error_message() is non-empty.
      bool input(std::string_view input);

      // The most recent output. Never empty.
      const Styled_text & output() const;
      // Read the tagged string `str`, updating the output to display
      // the styled text obtained.
      void read_output(std::string_view str);
      // Updates the output to display the appropriate screen.
      void refresh_output();

      // The most recent error message. Usually empty.
      // The only styles that will ever appear here are help, help_title and
      // command.
      const Styled_text & error_message() const;
      // Reads the tagged string `str`, updating the error message to
      // display the styled text obtained.
      void read_error_message(std::string_view str);
      // Removes the current error message.
      void clear_error_message();

      // Add `scr` onto the front of the view-stack.
      // Ownership of the screen is automatically assumed by `this`.
      //
      // Should be followed by a call to `this->refresh_output()`.
      void push_viewstack(Base_Screen & scr);
      // Remove the screen at the front of the view-stack.
      // The screen is automatically deleted, if owned by `this`.
      //
      // Should be followed by a call to `this->refresh_output()`.
      //
      // @throws `error::empty_stack` if the view-stack is empty.
      void pop_viewstack();
      // Remove all screens from the view-stack.
      // The screens owned by `this` are automatically deleted.
      void clear_viewstack();

      // Add `screen` onto the back of the game-stack.
      // Ownership of the screen is automatically assumed by `this`.
      void push_gamestack(Game_Screen & screen);
      // Remove the screen at the front of the game-stack.
      // The screen is automatically deleted.
      //
      // @throws `error::empty_stack` if the game-stack is empty.
      void pop_gamestack();
      // Move the screen at the front of the game-stack onto the front of the view-stack.
      //
      // Should be followed by a call to `this->refresh_output()`.
      //
      // @throws `error::empty_stack` if the game-stack is empty.
      void advance_gamestack();
      // Remove all screens from the game-stack.
      // The screens are automatically deleted.
      void clear_gamestack();

      // Clear both the view-stack and the game-stack.
      // All screens owned by `this` are deleted.
      void clear_stacks();

      // Gets the help screen currently being stored, or nullptr if none exists.
      const Help_Screen * help_screen() const;
      // Checks if a help screen is currently being stored.
      bool has_help_screen() const;
      // Stores hs, deleting any help screen that is already being stored.
      // Automatically assumes ownership of hs.
      void store_help_screen(Help_Screen *hs);
      // Deletes the current help screen, if one is being stored.
      void clear_help_screen();

      // Gets the question currently being stored, or nullptr if none exists.
      const Question * question() const;
      // Checks if a question is currently being stored.
      bool has_question() const;
      // Stores q, deleting any question that is already being stored.
      // Automatically assumes ownership of q.
      void store_question(Question *q);
      // Deletes the current question, if one is being stored.
      void clear_question();

      // The current game in progress.
      // Throws an exception if no game is in progress.
      const Game & game() const;
      // The game log managing the current game in progress.
      // Throws an exception if no game is in progress.
      const Game_log & game_log() const;
      // Checks if a game is currently in progress.
      bool has_game() const;

      // Begin a new game using the given parameters.
      void begin_game(const std::vector<std::string> &player_names,
                      const std::vector<Role::ID> &role_ids,
                      const std::vector<Wildcard::ID> &wildcard_ids,
                      const Rulebook &rulebook);
      // Instantly ends any game in progress, and saves a transcript of it.
      // (the game may not have actually reached an end state.)
      void end_game();

      // The rulebook currently being used: either the current game's rulebook
      // if a game is in progress, or the setup screen's rulebook.
      const Rulebook & active_rulebook() const;

   private:
      Styled_text _output{};
      Styled_text _error_message{};

      std::unique_ptr<Game_log> _game_log{};
      screen::Setup _setup_screen{*this};
      std::unique_ptr<Help_Screen> _help_screen{};
      std::unique_ptr<Question> _question{};

      std::vector<rkt::ref<Base_Screen>> _view_stack{rkt::ref<Base_Screen>(_setup_screen)};
      std::deque<rkt::ref<Game_Screen>> _game_stack{};
      std::vector<std::unique_ptr<Base_Screen>> _owned_screens{};

      // Assume ownership of the given screen.
      void store_screen(Base_Screen & screen);
      // Delete the given screen.
      // Has no effect if `screen` is not owned by `this`.
      void delete_screen(Base_Screen & screen);
      // Delete all of the screens owned by `this`.
      void delete_all_screens();

      friend class Game_Screen;
   };
}

#endif
