#ifndef MAFIA_CONSOLE_H
#define MAFIA_CONSOLE_H

#include <array>
#include <memory>
#include <unordered_set>
#include <utility>

#include "../riketi/algorithm.hpp"

#include "game_log.hpp"
#include "help_screens.hpp"
#include "questions.hpp"
#include "setup_screen.hpp"
#include "styled_string.hpp"

namespace maf {
   class Game_Screen;


   // Decide whether or not the given container of string-like objects matches the
   // given array of commands, which is true exactly when `std::size(c) == std::size(arr)`
   // and at each position `i`, either `std::empty(arr[i])` or `c[i] == arr[i]`.
   template <typename Cont, std::size_t N>
   bool commands_match(const Cont & c, const std::string_view (&arr)[N]);

   template <typename Str, std::size_t N>
   bool commands_match(const std::vector<Str> & v, const std::string_view (&arr)[N]);

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
      // Reads the tagged string contained in is, updating the output to display
      // the styled text obtained.
      void read_output(std::istream &is);
      // Updates the output to display the appropriate screen.
      void refresh_output();

      // The most recent error message. Usually empty.
      // The only styles that will ever appear here are help, help_title and
      // command.
      const Styled_text & error_message() const;
      // Reads the tagged string contained in is, updating the error message to
      // display the styled text obtained.
      void read_error_message(std::istream &is);
      // Removes the current error message.
      void clear_error_message();

      // Get the screen currently being displayed.
      const Base_Screen & current_screen() const;
      // Add `screen` onto the stack.
      // It will become the screen currently being displayed.
      void push_screen(Base_Screen & screen);
      // Remove the screen currently being displayed from the stack.
      void pop_screen();

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
      screen::Setup _setup_screen;
      std::unique_ptr<Help_Screen> _help_screen{};
      std::unique_ptr<Question> _question{};

      std::vector<rkt::ref<Base_Screen>> _screen_stack;
      std::unordered_set<std::unique_ptr<Base_Screen>> _owned_screens{};

      friend class Game_Screen;
   };
}



template <typename Cont, std::size_t N>
bool maf::commands_match(const Cont & c, const std::string_view (&arr)[N])
{
   auto eq = [](auto& s1, std::string_view s2) {
      return std::empty(s2) || s1 == s2;
   };

   return rkt::matches(c, arr, eq);
}

template <typename Str, std::size_t N>
bool maf::commands_match(const std::vector<Str> & v, const::std::string_view (&arr)[N])
{
   auto eq = [](auto& s1, std::string_view s2) {
      return std::empty(s2) || s1 == s2;
   };

   if constexpr(N == 0) {
      return std::size(v) == 0;
   } else if constexpr(N == 1) {
      return std::size(v) == 1 && eq(v[0], arr[0]);
   } else if constexpr(N == 2) {
      return std::size(v) == 2 && eq(v[0], arr[0]) && eq(v[1], arr[1]);
   } else if constexpr(N == 3) {
      return std::size(v) == 3 && eq(v[0], arr[0]) && eq(v[1], arr[1]) && eq(v[2], arr[2]);
   } else if constexpr(N == 4) {
      return std::size(v) == 4 && eq(v[0], arr[0]) && eq(v[1], arr[1]) && eq(v[2], arr[2]) && eq(v[3], arr[3]);
   } else if constexpr(N == 5) {
      return std::size(v) == 5 && eq(v[0], arr[0]) && eq(v[1], arr[1]) && eq(v[2], arr[2]) && eq(v[3], arr[3]) && eq(v[4], arr[4]);
   } else {
      return rkt::matches(v, arr, eq);
   }
}

#endif
