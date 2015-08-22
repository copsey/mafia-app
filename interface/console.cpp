#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "../riketi/algorithm.hpp"
#include "../riketi/char.hpp"
#include "../riketi/enum.hpp"
#include "../riketi/string.hpp"

#include "alias.hpp"
#include "console.hpp"

bool mafia::Console::do_commands(const std::vector<std::string> &commands) {
   std::ostringstream err_ss{}; // Write an error here if something goes wrong.

   try {
      if (commands_match(commands, {})) {
         // 0 commands is equivalent to refreshing the output.
      } else if (commands_match(commands, {"ok"})) {
         /* fix-me: move game_is_in_progress() test to seperate function. */

         if (!game_is_in_progress()) {
            err_ss << "^HNo game in progress!^hThere is no game in progress to "
            "continue.";
         } else {
            _game_log_ptr->advance();
         }
      } else if (commands_match(commands, {"yes"})) {
         switch (_question) {
            case Question::none:
               err_ss << "^HInvalid input!^hThere is no question to answer "
                         "right now.";
               break;
               
            case Question::confirm_end_game:
               _question = Question::none;
               terminate_game();
               break;
         }
      } else if (commands_match(commands, {"no"})) {
         switch (_question) {
            case Question::none:
               err_ss << "^HInvalid input!^hThere is no question to answer "
                         "right now.";
               break;

            case Question::confirm_end_game:
               _question = Question::none;
               break;
         }
      } else if (commands_match(commands, {"begin"})) {
         if (game_is_in_progress()) {
            err_ss << "^HGame in progress!^hA new game cannot begin until the "
            "current game ends.\nEnter ^cend^h to force the game to end "
            "early, or enter ^cok^h if the game has already ended and you want "
            "to return to the game setup screen.";
         } else {
            begin_new_game();
         }
      } else if (commands_match(commands, {"end"})) {
         if (!game_is_in_progress()) {
            err_ss << "^HNo game in progress!^hThere is no game in progress to "
            "end.";
         } else {
            _question = Question::confirm_end_game;
         }
      } else if (commands_match(commands, {"add", "p", ""})) {
         /* fix-me: make sure that no ^ characters appear in the name! */

         const std::string &name = commands[2];
         if (has_pending_player(name)) {
            err_ss << "^HPlayer already exists!^hA player named ^c"
                   << name
                   << "^h has already been selected to play in the next game. "
                      "(note: names are case-insensitive.)";
         } else {
            _pending_player_names.push_back(std::move(name));
         }
      } else if (commands_match(commands, {"add", "r", ""})) {
         /* fix-me: make sure that a role can be added right now, i.e. that no
          game is in progress currently. */

         _pending_role_ids.push_back(find_role(commands[2]).id());
      } else if (commands_match(commands, {"take", "r", ""})) {
         /* fix-me: make sure that a role can be removed right now, i.e. that no
          game is in progress currently. */


      } else if (commands_match(commands, {"vote", "", ""})) {
         /* fix-me: check game is in progress. */

         const Player &caster = find_player(commands[1]);
         const Player &target = find_player(commands[2]);

         _game_log_ptr->cast_lynch_vote(caster, target);
         _game_log_ptr->advance();
      } else if (commands_match(commands, {"abstain", ""})) {
         /* fix-me: check game is in progress. */

         const Player &caster = find_player(commands[1]);
         
         _game_log_ptr->clear_lynch_vote(caster);
         _game_log_ptr->advance();
      } else if (commands_match(commands, {"lynch"})) {
         /* fix-me: check that game is in progress and lynch can occur. */

         _game_log_ptr->process_lynch_votes();
         _game_log_ptr->advance();
      } else if (commands_match(commands, {"preset"})) {
         if (game_is_in_progress()) {
            show_error_message("^HGame in progress!^hA new game cannot begin until the current game ends.\n(Enter ^cend^h to force the game to end early, or enter ^cok^h if the game has already ended and you want to return to the game setup screen.)");
            return false;
         } else {
            /* fix-me: the command "preset x" should choose preset x, a pre-defined test setup.
             (here, x should be an integer constant.) It is an error if preset x doesn't exist.
             The command "preset" should choose a randomly-selected preset from those available. */

            std::vector<std::string> player_names{"Augustus", "Brutus", "Claudius", "Drusilla"};
            std::vector<Role::ID> role_ids{Role::ID::peasant, Role::ID::racketeer, Role::ID::coward};
            std::vector<Joker::ID> joker_ids{Joker::ID::village_basic};
            Rulebook rulebook{};

            begin_new_game(player_names, role_ids, joker_ids, rulebook);
         }
      } else {
         err_ss << "^HUnrecognised input!^hThe text that you entered couldn't "
                   "be recognised.\n(enter ^chelp^h if you're unsure what to "
                   "do, or ^clist c^h for a full list of commands.)";
      }

      /* fix-me: list p random, a utility command to generate a list of the players in a game, in a random order.
       (for example, when asking people to choose their lynch votes, without the option to change.) */
   }

   catch (const player_not_found &e) {
      err_ss << "^HPlayer not found!^hA player named ^c"
             << e.name()
             << "^h could not be found.";
   }

   catch (const no_role_for_alias &e) {
      err_ss << "^HInvalid role alias!^hNo role could be found whose alias is "
                "^c"
             << e.alias()
             << "^h. (note: aliases are case-sensitive.)\nFor a full list of "
                "role aliases, enter ^clist r^h.";
   }

   catch (const players_to_cards_mismatch &e) {
      err_ss << "^HMismatch!^hYou attempted to start a game with "
             << e.num_players()
             << " player(s) and "
             << e.num_cards()
             << " role(s)/joker(s). These numbers must be equal.";
   }

   catch (const Game_cannot_continue &e) {
      err_ss << "^HCannot continue!^h";

      switch (e.reason()) {
         case Game_cannot_continue::Reason::lynch_can_occur:
            err_ss << "The game cannot continue until a lynching has taken "
                      "place.\nEnter ^clynch^h to submit the current lynch "
                      "votes.";
            break;
      }
   }

   catch (const lynch_vote_failed &e) {
      err_ss << "^HLynch vote failed!^h";

      switch (e.reason()) {
         case lynch_vote_failed::Reason::bad_timing:
            err_ss << "No lynch votes can be cast at this moment in time.";
            break;

         case lynch_vote_failed::Reason::caster_is_not_present:
            err_ss << e.caster().name()
                   << " is unable to cast a lynch vote, as they are no longer "
                      "present in the game.";
            break;

         case lynch_vote_failed::Reason::target_is_not_present:
            err_ss << e.caster().name()
                   << " cannot cast a lynch vote against "
                   << e.target()->name()
                   << ", because "
                   << e.target()->name()
                   << " is no longer present in the game.";
            break;

         case lynch_vote_failed::Reason::caster_is_target:
            err_ss << "A player cannot cast a lynch vote against themself.";
            break;
      }
   }

   catch (const badly_timed_lynch &e) {
      err_ss << "^HLynch not allowed!^hA lynch cannot occur at this moment in "
                "time.";
   }

   std::string err{err_ss.str()};
   if (err.length() == 0) {
      refresh_output();
      clear_error_message();
      return true;
   } else {
      show_error_message(err);
      return false;
   }
}

bool mafia::Console::input(const std::string& input) {
   return do_commands(rkt::split_if_and_prune(input, [](char ch) {
      return std::isspace(ch);
   }));
}

bool mafia::Console::commands_match(const std::vector<std::string> &v1,
                                    const std::vector<std::string> &v2) {
   return rkt::equal(v1, v2,
                     [](const std::string &s1, const std::string &s2) {
                        return s1.empty() || s2.empty() || s1 == s2;
                     });
}

void mafia::Console::show_output(const std::string &s) {
   _output = to_styled_text(s);
}

void mafia::Console::show_game_setup() {
   /* fix-me */

   std::string str{"^HGame Setup^hHere, you can choose the players and roles/jokers that will feature in the next game."};

   if (_pending_player_names.size() == 0) {
      if (_pending_role_ids.size() + _pending_joker_ids.size() == 0) {
         str += "\n\nNo players or roles/jokers have been selected.";
      } else {
         str += "\n\nNo players have been selected.";
      }
   } else {

   }

   show_output(str);
}

void mafia::Console::show_question(Question q) {
   switch (q) {
      case Question::none:
         throw std::invalid_argument("Attempted to show a non-existent"
                                     "question!");

      case Question::confirm_end_game:
         show_output("^HEnd Game?^hYou are about to end the current game.\n"
                     "Are you sure that you want to do this? "
                     "(^cyes^h or ^cno^h)");
         break;
   }
}

void mafia::Console::show_current_question() {
   show_question(_question);
}

void mafia::Console::show_event(const mafia::Event &event) {
   show_output(event.description());
}

void mafia::Console::show_current_event() {
   show_event(current_event());
}

void mafia::Console::refresh_output() {
   switch (_question) {
      case Question::none:
         if (game_is_in_progress()) {
            show_current_event();
         } else {
            show_game_setup();
         }
         break;

      default:
         show_current_question();
   }
}

void mafia::Console::show_error_message(const std::string &s) {
   _error_message = to_styled_text(s);
}

void mafia::Console::clear_error_message() {
   _error_message.clear();
}

void mafia::Console::begin_new_game(const std::vector<std::string> &pl_names,
                                    const std::vector<Role::ID> &r_ids,
                                    const std::vector<Joker::ID> &j_ids,
                                    const mafia::Rulebook &rulebook) {
   // Create a new pointer seperately, in case creating a new game results in an
   // exception being thrown.
   auto ptr = std::make_unique<Game_log>(pl_names, r_ids, j_ids, rulebook);
   _game_log_ptr = std::move(ptr);
}

void mafia::Console::begin_new_game() {
   begin_new_game(_pending_player_names,
                  _pending_role_ids,
                  _pending_joker_ids,
                  _pending_rulebook);
}

void mafia::Console::terminate_game() {
   /* fix-me: set location where history is saved. */

   std::time_t t = std::time(nullptr);
   std::tm tm = *std::localtime(&t);

   std::ofstream fs{"game_history.txt", std::ios_base::app};
   fs << "\n\n====== ";
   fs << std::put_time(&tm, "%F %T");
   fs << " ======\n\n";
   fs << _game_log_ptr->transcript();
   fs.close();

   _game_log_ptr.reset();
}

bool mafia::Console::has_pending_player(const std::string &name) const {
   for (const std::string &existing_name : _pending_player_names) {
      if (rkt::equal_up_to_case(existing_name, name)) {
         return true;
      }
   }
   return false;
}

const mafia::Player & mafia::Console::find_player(const std::string &s) const {
   for (const Player &p : _game_log_ptr->game().players()) {
      if (rkt::equal_up_to_case(s, p.name())) return p;
   }

   throw player_not_found(s);
}

const mafia::Role & mafia::Console::find_role(const std::string &alias) const {
   for (const auto &r : _pending_rulebook.roles()) {
      if (mafia::alias(r) == alias) return r;
   }

   throw no_role_for_alias(alias);
}
