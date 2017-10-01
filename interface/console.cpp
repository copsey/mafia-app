#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "../riketi/algorithm.hpp"
#include "../riketi/char.hpp"
#include "../riketi/enum.hpp"
#include "../riketi/experimental/algorithm.hpp"
#include "../riketi/random.hpp"
#include "../riketi/string.hpp"

#include "error.hpp"
#include "names.hpp"
#include "console.hpp"

const std::array<maf::Console::Game_parameters, maf::Console::num_presets> maf::Console::_presets{
   maf::Console::Game_parameters{
      {"Augustus", "Brutus", "Claudius", "Drusilla"},
      {maf::Role::ID::peasant, maf::Role::ID::racketeer, maf::Role::ID::coward},
      {maf::Wildcard::ID::village_basic},
      maf::Rulebook{}
   },
   maf::Console::Game_parameters{
      {"Nine", "Ten", "Jack", "Queen", "King", "Ace"},
      {maf::Role::ID::peasant, maf::Role::ID::peasant,maf::Role::ID::doctor, maf::Role::ID::detective, maf::Role::ID::dealer, maf::Role::ID::musketeer},
      {},
      maf::Rulebook{}
   }
};

bool maf::commands_match(const std::vector<std::string> &v1,
                           const std::vector<std::string> &v2) {
   return rkt::matches(v1, v2, [](const std::string &s1, const std::string &s2) {
      return s1.empty() || s2.empty() || s1 == s2;
   });
}

maf::Console::Console() {
   refresh_output();
}

bool maf::Console::do_commands(const std::vector<std::string>& commands) {
   try {
      if (commands.size() == 0) {
         throw error::unresolved_input("^HMissing input!^hEntering a blank input has no effect.\n(enter ^chelp^h if you're unsure what to do.)");
      }
      else if (commands_match(commands, {"help"})) {
         if (has_game()) {
            store_help_screen(new Event_help_screen(_game_log->current_event()));
         } else {
            store_help_screen(new Setup_help_screen());
         }
      }
      else if (commands_match(commands, {"help", "r", ""})) {
         auto& r_alias = commands[2];

         try {
            const Role& r = active_rulebook().get_role(r_alias);
            store_help_screen(new Role_info_screen(r));
         } catch (error::missing_role()) {
            auto esc_r_alias = copy_with_escaped_style_codes(r_alias);

            std::stringstream err{};
            err << "^HInvalid alias!^hNo role could be found whose alias is ^c";
            err << esc_r_alias;
            err << "^h.\nNote that aliases are case-sensitive.\n(enter ^clist r^h to see a list of each role and its alias.)";
            throw error::unresolved_input(err);
         }
      }
      else if (commands_match(commands, {"list", "r"})) {
         store_help_screen(new List_roles_screen(active_rulebook()));
      }
      else if (commands_match(commands, {"list", "r", "v"})) {
         store_help_screen(new List_roles_screen(active_rulebook(), Alignment::village));
      }
      else if (commands_match(commands, {"list", "r", "m"})) {
         store_help_screen(new List_roles_screen(active_rulebook(), Alignment::mafia));
      }
      else if (commands_match(commands, {"list", "r", "f"})) {
         store_help_screen(new List_roles_screen(active_rulebook(), Alignment::freelance));
      }
      else if (commands_match(commands, {"info", ""})) {
         if (!has_game()) {
            throw error::unresolved_input("^HNo game in progress!^hThere is no game in progress to display information about.");
         } else {
            auto& pl_name = commands[1];

            try {
               auto& player = _game_log->find_player(pl_name);
               store_help_screen(new Player_Info_Screen{player, *_game_log});
            } catch (error::missing_player) {
               auto pl_name_esc = copy_with_escaped_style_codes(pl_name);

               std::stringstream err{};
               err << "^HMissing player!^h";
               err << "A player named ^c";
               err << pl_name_esc;
               err << "^h could not be found.";
               throw error::unresolved_input(err);
            }
         }
      }
      else if (has_help_screen()) {
         if (commands_match(commands, {"ok"})) {
            clear_help_screen();
         } else {
            throw error::unresolved_input("^HInvalid input!^hPlease leave the help screen that is currently being displayed before trying to do anything else.\n(this is done by entering ^cok^h)");
         }
      }
      else if (has_question()) {
         if (_question->do_commands(commands, *this)) {
            clear_question();
         }
      }
      else if (commands_match(commands, {"end"})) {
         if (!has_game()) {
            throw error::unresolved_input("^HNo game in progress!^hThere is no game in progress to end.");
         } else if (dynamic_cast<const Game_ended *>(&_game_log->current_event())) {
            end_game();
         } else {
            store_question(new Confirm_end_game());
         }
      }
      else if (has_game()) {
         _game_log->do_commands(commands);
      }
      else if (commands_match(commands, {"begin"})) {
         try {
            begin_pending_game();
         } catch (error::cards_mismatch) {
            throw error::unresolved_input("^HMismatch!^hA new game cannot begin with an unequal number of players and cards.");
         }
      }
      else if (commands_match(commands, {"preset"})) {
         std::uniform_int_distribution<int> uid{0, static_cast<int>(num_presets) - 1};
         begin_preset(uid(rkt::random_engine));
      }
      else if (commands_match(commands, {"preset", ""})) {
         auto& i_str = commands[1];

         try {
            auto i = std::stoi(i_str);

            try {
               begin_preset(i);
            } catch (error::invalid_preset) {
               std::stringstream err{};
               err << "^HMissing preset!^hThere is no preset defined with index ";
               err << i;
               err << ".";
               throw error::unresolved_input(err);
            }
         } catch (std::invalid_argument) {
            auto esc_i_str = copy_with_escaped_style_codes(i_str);

            std::stringstream err{};
            err << "^HInvalid input!^hThe string ^c";
            err << esc_i_str;
            err << "^h could not be converted into a preset index. (i.e. a relatively-small integer)";
            throw error::unresolved_input(err);
         } catch (std::out_of_range) {
            auto esc_i_str = copy_with_escaped_style_codes(i_str);

            std::stringstream err{};
            err << "^HInvalid input!^hThe string ^c";
            err << esc_i_str;
            err << "^h could not be converted into a preset index. (i.e. a relatively-small integer)";
            throw error::unresolved_input(err);
         }
      }
      else {
         try {
            _setup_screen.do_commands(commands);
         } catch (error::bad_commands) {
            throw error::unresolved_input("^HUnrecognised input!^hThe text that you entered couldn't be recognised.\n(enter ^chelp^h if you're unsure what to do.)");
         }
      }

      /* fix-me: add  "list w", "list w v", "list w m", "list w f". */

      /* fix-me: "add p A B C" should result in players A, B, C all being chosen. */

      /* fix-me: enter "auto" to automatically choose enough random cards for the currently-selected players to start a new game. */

      /* fix-me: list p random, a utility command to generate a list of the players in a game, in a random order.
       (for example, when asking people to choose their lynch votes, without the option to change.)
       list p should be context-aware, i.e. it should show pending players if no game is in progress, and actual players if a game is in progress. */

      /* fix-me: enter "skip" to skip a player's ability use at night and the mafia's kill. This should result in a yes/no screen to be safe. */

      refresh_output();
      clear_error_message();
      return true;
   }
//   catch (const Game::Lynch_failed &e) {
//      err << "^HLynch failed!^h";
//
//      switch (e.reason) {
//         case Game::Lynch_failed::Reason::game_ended:
//            err << "The game has already ended.";
//            break;
//
//         case Game::Lynch_failed::Reason::bad_timing:
//            err << "A lynch cannot occur at this moment in time.";
//            break;
//      }
//   }
//   catch (const Game::Lynch_vote_failed &e) {
//      err << "^HLynch vote failed!^h";
//
//      switch (e.reason) {
//         case Game::Lynch_vote_failed::Reason::game_ended:
//            err << "The game has already ended.";
//            break;
//
//         case Game::Lynch_vote_failed::Reason::bad_timing:
//            err << "No lynch votes can be cast at this moment in time.";
//            break;
//
//         case Game::Lynch_vote_failed::Reason::voter_is_not_present:
//            err << _game_log->get_name(*e.voter)
//            << " is unable to cast a lynch vote, as they are no longer present in the game.";
//            break;
//
//         case Game::Lynch_vote_failed::Reason::target_is_not_present:
//            err << _game_log->get_name(*e.voter)
//            << " cannot cast a lynch vote against "
//            << _game_log->get_name(*e.target)
//            << ", because "
//            << _game_log->get_name(*e.target)
//            << " is no longer present in the game.";
//            break;
//
//         case Game::Lynch_vote_failed::Reason::voter_is_target:
//            err << "A player cannot cast a lynch vote against themself.";
//            break;
//      }
//   }
//   catch (const Game::Duel_failed &e) {
//      err << "^HDuel failed!^h";
//
//      switch (e.reason) {
//         case Game::Duel_failed::Reason::game_ended:
//            err << "The game has already ended.";
//            break;
//
//         case Game::Duel_failed::Reason::bad_timing:
//            err << "A duel can only take place during the day.";
//            break;
//
//         case Game::Duel_failed::Reason::caster_is_not_present:
//            err << _game_log->get_name(*e.caster)
//            << " is unable to initiate a duel, as they are no longer present in the game.";
//            break;
//
//         case Game::Duel_failed::Reason::target_is_not_present:
//            err << _game_log->get_name(*e.caster)
//            << " cannot initiate a duel against "
//            << _game_log->get_name(*e.target)
//            << ", because "
//            << _game_log->get_name(*e.target)
//            << " is no longer present in the game.";
//            break;
//
//         case Game::Duel_failed::Reason::caster_is_target:
//            err << "A player cannot duel themself.";
//            break;
//
//         case Game::Duel_failed::Reason::caster_has_no_duel:
//            err << _game_log->get_name(*e.caster)
//            << " has no duel ability to use.";
//            break;
//      }
//   }
//   catch (const Game::Begin_night_failed &e) {
//      err << "^HCannot begin night!^h";
//
//      switch (e.reason) {
//         case Game::Begin_night_failed::Reason::game_ended:
//            err << "The game has ended, and so cannot be continued.\n(enter ^cend^h to return to the game setup screen.)";
//            break;
//
//         case Game::Begin_night_failed::Reason::already_night:
//            err << "It is already nighttime.";
//
//         case Game::Begin_night_failed::Reason::lynch_can_occur:
//            err << "The next night cannot begin until a lynch has taken place.\n(enter ^clynch^h to submit the current lynch votes.)";
//            break;
//      }
//   }
//   catch (const Game::Choose_fake_role_failed &e) {
//      err << "^HChoose fake role failed!^h";
//
//      switch (e.reason) {
//         case Game::Choose_fake_role_failed::Reason::game_ended:
//            err << "The game has already ended.";
//            break;
//
//         case Game::Choose_fake_role_failed::Reason::bad_timing:
//            err << "Wait until night before choosing a fake role.";
//            break;
//
//         case Game::Choose_fake_role_failed::Reason::player_is_not_faker:
//            err << _game_log->get_name(*e.player)
//            << " doesn't need to be given a fake role.";
//            break;
//
//         case Game::Choose_fake_role_failed::Reason::already_chosen:
//            err << _game_log->get_name(*e.player)
//            << " has already been given a fake role.";
//            break;
//      }
//   }
//   catch (const Game::Mafia_kill_failed &e) {
//      err << "^HMafia kill failed!^h";
//
//      switch (e.reason) {
//         case Game::Mafia_kill_failed::Reason::game_ended:
//            err << "The game has already ended.";
//            break;
//         case Game::Mafia_kill_failed::Reason::bad_timing:
//            err << "The mafia can only use their kill during the night.";
//            break;
//         case Game::Mafia_kill_failed::Reason::already_used:
//            err << "Either the mafia have already used their kill this night, or there are no members of the mafia remaining to perform a kill.";
//            break;
//         case Game::Mafia_kill_failed::Reason::caster_is_not_present:
//            err << _game_log->get_name(*e.caster)
//            << " cannot perform the mafia's kill, as they are no longer in the game.";
//            break;
//         case Game::Mafia_kill_failed::Reason::caster_is_not_in_mafia:
//            err << _game_log->get_name(*e.caster)
//            << " cannot perform the mafia's kill, as they are not part of the mafia.";
//            break;
//         case Game::Mafia_kill_failed::Reason::target_is_not_present:
//            err << _game_log->get_name(*e.target)
//            << " cannot be targetted to kill by the mafia, as they are no longer in the game.";
//            break;
//         case Game::Mafia_kill_failed::Reason::caster_is_target:
//            err << _game_log->get_name(*e.caster)
//            << " cannot use the mafia's kill on themself.";
//            break;
//      }
//   }
//   catch (const Game::Kill_failed &e) {
//      err << "^HKill failed!^h";
//
//      switch (e.reason) {
//         case Game::Kill_failed::Reason::game_ended:
//            err << "The game has already ended.";
//            break;
//         case Game::Kill_failed::Reason::caster_cannot_kill:
//            err << _game_log->get_name(*e.caster)
//            << " cannot use a kill ability right now.";
//            break;
//         case Game::Kill_failed::Reason::target_is_not_present:
//            err << _game_log->get_name(*e.caster)
//            << " cannot kill "
//            << _game_log->get_name(*e.target)
//            << ", because "
//            << _game_log->get_name(*e.target)
//            << " is no longer present in the game.";
//            break;
//         case Game::Kill_failed::Reason::caster_is_target:
//            err << _game_log->get_name(*e.caster)
//            << " is not allowed to kill themself.\n(nice try.)";
//            break;
//      }
//   }
//   catch (const Game::Heal_failed &e) {
//      err << "^HHeal failed!^h";
//
//      switch (e.reason) {
//         case Game::Heal_failed::Reason::game_ended:
//            err << "The game has already ended.";
//            break;
//         case Game::Heal_failed::Reason::caster_cannot_heal:
//            err << _game_log->get_name(*e.caster)
//            << " cannot use a heal ability right now.";
//            break;
//         case Game::Heal_failed::Reason::target_is_not_present:
//            err << _game_log->get_name(*e.caster)
//            << " cannot heal "
//            << _game_log->get_name(*e.target)
//            << ", because "
//            << _game_log->get_name(*e.target)
//            << " is no longer present in the game.";
//            break;
//         case Game::Heal_failed::Reason::caster_is_target:
//            err << _game_log->get_name(*e.caster)
//            << " cannot heal themself.";
//            break;
//      }
//   }
//   catch (const Game::Investigate_failed &e) {
//      err << "^HInvestigation failed!^h";
//
//      switch (e.reason) {
//         case Game::Investigate_failed::Reason::game_ended:
//            err << "The game has already ended.";
//            break;
//         case Game::Investigate_failed::Reason::caster_cannot_investigate:
//            err << _game_log->get_name(*e.caster)
//            << " cannot investigate anybody right now.";
//            break;
//         case Game::Investigate_failed::Reason::target_is_not_present:
//            err << _game_log->get_name(*e.caster)
//            << " cannot investigate "
//            << _game_log->get_name(*e.target)
//            << ", because "
//            << _game_log->get_name(*e.target)
//            << " is no longer present in the game.";
//            break;
//         case Game::Investigate_failed::Reason::caster_is_target:
//            err << _game_log->get_name(*e.caster)
//            << " cannot investigate themself.";
//            break;
//      }
//   }
//   catch (const Game::Skip_failed &e) {
//      err << "^HSkip failed!^hThe current ability, if one is showing, cannot be skipped.";
//   }
//   catch (const Game_log::Player_not_found &e) {
//      err << "^HPlayer not found!^hA player named ^c"
//      << e.name
//      << "^h could not be found.";
//   }
   catch (const Event::Bad_commands &e) {
      throw error::unresolved_input("^HUnrecognised input!^hThe text that you entered couldn't be recognised.\n(enter ^chelp^h if you're unsure what to do.)");
   }
   catch (const Question::Bad_commands &e) {
      throw error::unresolved_input("^HInvalid input!^hPlease answer the question being shown before trying to do anything else.");
   }
   catch (const No_game_in_progress &e) {
      throw error::unresolved_input("^HNo game in progress!^hThere is no game in progress at the moment, and so game-related commands cannot be used.\n(enter ^cbegin^h to begin a new game, or ^chelp^h for a list of usable commands.)");
   }
   catch (const Begin_game_failed &e) {
      switch (e.reason) {
         case Begin_game_failed::Reason::game_already_in_progress:
            throw error::unresolved_input("^HGame in progress!^hA new game cannot begin until the current game ends.\n(enter ^cend^h to force the game to end early, or if the game has already ended and you want to return to the game setup screen.)");
            break;
      }
   }
   catch (const error::unresolved_input& ex) {
      _error_message = ex.styled_message();
      return false;
   }
}

bool maf::Console::input(const std::string& input) {
   auto commands = rkt::split_if_and_prune(input, [](char c) {
      return std::isspace(c);
   });
   return do_commands(commands);
}

const maf::Styled_text & maf::Console::output() const {
   return _output;
}

void maf::Console::read_output(std::istream &is) {
   _output = styled_text_from(is);
}

void maf::Console::refresh_output() {
   std::stringstream ss{};

   if (has_help_screen()) {
      _help_screen->write(ss);
   } else if (has_question()) {
      _question->write(ss);
   } else if (has_game()) {
      _game_log->current_event().write_full(ss);
   } else {
      _setup_screen.write(ss);
   }

   read_output(ss);
}

const maf::Styled_text & maf::Console::error_message() const {
   return _error_message;
}

void maf::Console::read_error_message(std::istream& is) {
   _error_message = styled_text_from(is);
}

void maf::Console::clear_error_message() {
   _error_message.clear();
}

const maf::Help_screen * maf::Console::help_screen() const {
   return _help_screen.get();
}

bool maf::Console::has_help_screen() const {
   return static_cast<bool>(_help_screen);
}

void maf::Console::store_help_screen(Help_screen *hs) {
   _help_screen.reset(hs);
}

void maf::Console::clear_help_screen() {
   _help_screen.reset();
}

const maf::Question * maf::Console::question() const {
   return _question.get();
}

bool maf::Console::has_question() const {
   return static_cast<bool>(_question);
}

void maf::Console::store_question(Question *q) {
   _question.reset(q);
}

void maf::Console::clear_question() {
   _question.reset();
}

const maf::Game & maf::Console::game() const {
   if (!has_game()) throw No_game_in_progress();
   return _game_log->game();
}

const maf::Game_log & maf::Console::game_log() const {
   if (!has_game()) throw No_game_in_progress();
   return *_game_log;
}

bool maf::Console::has_game() const {
   return (bool)_game_log;
}

void maf::Console::end_game() {
   /* fix-me: set location where history is saved. */

   if (has_game()) {
      std::time_t t = std::time(nullptr);

      std::ofstream ofs{"/Users/Jack_Copsey/dev/mafia/misc/game_history.txt", std::ofstream::app};
      ofs << "\n====== ";
      ofs << std::put_time(std::localtime(&t), "%F %T");
      ofs << " ======\n\n";
      _game_log->write_transcript(ofs);

      _game_log.reset();
   }
}

const maf::Rulebook & maf::Console::active_rulebook() const {
   if (has_game()) {
      return _game_log->game().rulebook();
   } else {
      return _setup_screen.rulebook();
   }
}

void maf::Console::begin_game(const std::vector<std::string> &pl_names,
                                const std::vector<Role::ID> &r_ids,
                                const std::vector<Wildcard::ID> &w_ids,
                                const Rulebook &rulebook) {
   if (has_game()) throw Begin_game_failed{Begin_game_failed::Reason::game_already_in_progress};
   _game_log.reset(new Game_log{pl_names, r_ids, w_ids, rulebook});
}

void maf::Console::begin_pending_game() {
   if (has_game()) throw Begin_game_failed{Begin_game_failed::Reason::game_already_in_progress};
   _game_log = _setup_screen.new_game_log();
}

void maf::Console::begin_preset(int i) {
   if (i >= 0 && i < num_presets) {
      Game_parameters params = _presets[i];
      begin_game(params.player_names, params.role_ids, params.wildcard_ids, params.rulebook);
   } else {
      throw error::invalid_preset();
   }
}
