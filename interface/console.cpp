#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "../riketi/algorithm.hpp"
#include "../riketi/random.hpp"
#include "../riketi/string.hpp"

#include "command.hpp"
#include "console.hpp"
#include "errors.hpp"
#include "game_screens.hpp"
#include "names.hpp"

maf::Console::Console()
   : _setup_screen{*this}, _screen_stack{rkt::ref<Base_Screen>(_setup_screen)}
{
   refresh_output();
}

bool maf::Console::do_commands(const std::vector<std::string_view> & commands) {
   std::stringstream err{}; // Write an error here if something goes wrong.

   try {
      if (commands.size() == 0) {
         err << "^TMissing input!^hEntering a blank input has no effect.\n(enter ^chelp^h if you're unsure what to do.)";
      }
      else if (commands_match(commands, {"help"})) {
         if (has_game()) {
            store_help_screen(new Event_Help_Screen{*this, _game_log->current_event()});
         } else {
            store_help_screen(new Setup_Help_Screen{*this});
         }
      }
      else if (commands_match(commands, {"help", "r", ""})) {
         const Role & r = active_rulebook().get_role(commands[2]);
         store_help_screen(new Role_Info_Screen{*this, r});
      }
      else if (commands_match(commands, {"list", "r"})) {
         store_help_screen(new List_Roles_Screen{*this});
      }
      else if (commands_match(commands, {"list", "r", "v"})) {
         store_help_screen(new List_Roles_Screen{*this, List_Roles_Screen::Filter_Alignment::village});
      }
      else if (commands_match(commands, {"list", "r", "m"})) {
         store_help_screen(new List_Roles_Screen{*this, List_Roles_Screen::Filter_Alignment::mafia});
      }
      else if (commands_match(commands, {"list", "r", "f"})) {
         store_help_screen(new List_Roles_Screen{*this, List_Roles_Screen::Filter_Alignment::freelance});
      }
      else if (has_help_screen()) {
         bool success = _help_screen->handle_commands(commands);

         if (!success) {
            err << "^h^TInvalid input!^/Please leave the help screen that is currently being displayed before trying to do anything else.\n(this is done by entering ^cok^/)";
         }
      }
      else if (has_question()) {
         bool success = _question->handle_commands(commands);

         if (success) {
            clear_question();
         } else {
            err << "^h^TInvalid input!^/Please answer the question being shown before trying to do anything else.";
         }
      }
      else if (has_game()) {
         _game_log->do_commands(commands);
      }
      else {
         bool success = _setup_screen.handle_commands(commands);

         if (!success) {
            err << "^h^TUnrecognised input!^/The text that you entered couldn't be recognised.\n(enter ^chelp^/ if you're unsure what to do.)";
         }
      }

      /* FIXME: add  "list w", "list w v", "list w m", "list w f". */

      /* FIXME: "add p A B C" should result in players A, B, C all being chosen. */

      /* FIXME: enter "auto" to automatically choose enough random cards for the currently-selected players to start a new game. */

      /* FIXME: list p random, a utility command to generate a list of the players in a game, in a random order.
       (for example, when asking people to choose their lynch votes, without the option to change.)
       list p should be context-aware, i.e. it should show pending players if no game is in progress, and actual players if a game is in progress. */

      /* FIXME: enter "skip" to skip a player's ability use at night and the mafia's kill. This should result in a yes/no screen to be safe. */
   }
   catch (const Rulebook::Missing_role_alias &e) {
      err << "^TInvalid alias!^hNo role could be found whose alias is ^c"
      << e.alias
      << "^h.\nNote that aliases are case-sensitive.\n(enter ^clist r^h to see a list of each role and its alias.)";
   }
   catch (const Rulebook::Missing_wildcard_alias &e) {
      err << "^TInvalid alias!^hNo wildcard could be found whose alias is ^c"
      << e.alias
      << "^h.\nNote that aliases are case-sensitive.\n(enter ^clist w^h to see a list of each wildcard and its alias.)";
   }
   catch (const Game::Kick_failed &e) {
      err << "^TKick failed!^h";

      switch (e.reason) {
         case Game::Kick_failed::Reason::game_ended:
            err << _game_log->get_name(*e.player)
                << " could not be kicked from the game, because the game has already ended.";
            break;

         case Game::Kick_failed::Reason::bad_timing:
            err << "Players can only be kicked from the game during the day.";
            break;

         case Game::Kick_failed::Reason::already_kicked:
            err << _game_log->get_name(*e.player)
                << " has already been kicked from the game";
            break;
      }
   }
   catch (const Game::Lynch_failed &e) {
      err << "^TLynch failed!^h";

      switch (e.reason) {
         case Game::Lynch_failed::Reason::game_ended:
            err << "The game has already ended.";
            break;

         case Game::Lynch_failed::Reason::bad_timing:
            err << "A lynch cannot occur at this moment in time.";
            break;
      }
   }
   catch (const Game::Lynch_vote_failed &e) {
      err << "^TLynch vote failed!^h";

      switch (e.reason) {
         case Game::Lynch_vote_failed::Reason::game_ended:
            err << "The game has already ended.";
            break;

         case Game::Lynch_vote_failed::Reason::bad_timing:
            err << "No lynch votes can be cast at this moment in time.";
            break;

         case Game::Lynch_vote_failed::Reason::voter_is_not_present:
            err << _game_log->get_name(*e.voter)
            << " is unable to cast a lynch vote, as they are no longer present in the game.";
            break;

         case Game::Lynch_vote_failed::Reason::target_is_not_present:
            err << _game_log->get_name(*e.voter)
            << " cannot cast a lynch vote against "
            << _game_log->get_name(*e.target)
            << ", because "
            << _game_log->get_name(*e.target)
            << " is no longer present in the game.";
            break;

         case Game::Lynch_vote_failed::Reason::voter_is_target:
            err << "A player cannot cast a lynch vote against themself.";
            break;
      }
   }
   catch (const Game::Duel_failed &e) {
      err << "^TDuel failed!^h";

      switch (e.reason) {
         case Game::Duel_failed::Reason::game_ended:
            err << "The game has already ended.";
            break;

         case Game::Duel_failed::Reason::bad_timing:
            err << "A duel can only take place during the day.";
            break;

         case Game::Duel_failed::Reason::caster_is_not_present:
            err << _game_log->get_name(*e.caster)
            << " is unable to initiate a duel, as they are no longer present in the game.";
            break;

         case Game::Duel_failed::Reason::target_is_not_present:
            err << _game_log->get_name(*e.caster)
            << " cannot initiate a duel against "
            << _game_log->get_name(*e.target)
            << ", because "
            << _game_log->get_name(*e.target)
            << " is no longer present in the game.";
            break;

         case Game::Duel_failed::Reason::caster_is_target:
            err << "A player cannot duel themself.";
            break;

         case Game::Duel_failed::Reason::caster_has_no_duel:
            err << _game_log->get_name(*e.caster)
            << " has no duel ability to use.";
            break;
      }
   }
   catch (const Game::Begin_night_failed &e) {
      err << "^TCannot begin night!^h";

      switch (e.reason) {
         case Game::Begin_night_failed::Reason::game_ended:
            err << "The game has ended, and so cannot be continued.\n(enter ^cend^h to return to the game setup screen.)";
            break;

         case Game::Begin_night_failed::Reason::already_night:
            err << "It is already nighttime.";

         case Game::Begin_night_failed::Reason::lynch_can_occur:
            err << "The next night cannot begin until a lynch has taken place.\n(enter ^clynch^h to submit the current lynch votes.)";
            break;
      }
   }
   catch (const Game::Choose_fake_role_failed &e) {
      err << "^TChoose fake role failed!^h";

      switch (e.reason) {
         case Game::Choose_fake_role_failed::Reason::game_ended:
            err << "The game has already ended.";
            break;

         case Game::Choose_fake_role_failed::Reason::bad_timing:
            err << "Wait until night before choosing a fake role.";
            break;

         case Game::Choose_fake_role_failed::Reason::player_is_not_faker:
            err << _game_log->get_name(*e.player)
            << " doesn't need to be given a fake role.";
            break;

         case Game::Choose_fake_role_failed::Reason::already_chosen:
            err << _game_log->get_name(*e.player)
            << " has already been given a fake role.";
            break;
      }
   }
   catch (const Game::Mafia_kill_failed &e) {
      err << "^TMafia kill failed!^h";

      switch (e.reason) {
         case Game::Mafia_kill_failed::Reason::game_ended:
            err << "The game has already ended.";
            break;
         case Game::Mafia_kill_failed::Reason::bad_timing:
            err << "The mafia can only use their kill during the night.";
            break;
         case Game::Mafia_kill_failed::Reason::already_used:
            err << "Either the mafia have already used their kill this night, or there are no members of the mafia remaining to perform a kill.";
            break;
         case Game::Mafia_kill_failed::Reason::caster_is_not_present:
            err << _game_log->get_name(*e.caster)
            << " cannot perform the mafia's kill, as they are no longer in the game.";
            break;
         case Game::Mafia_kill_failed::Reason::caster_is_not_in_mafia:
            err << _game_log->get_name(*e.caster)
            << " cannot perform the mafia's kill, as they are not part of the mafia.";
            break;
         case Game::Mafia_kill_failed::Reason::target_is_not_present:
            err << _game_log->get_name(*e.target)
            << " cannot be targetted to kill by the mafia, as they are no longer in the game.";
            break;
         case Game::Mafia_kill_failed::Reason::caster_is_target:
            err << _game_log->get_name(*e.caster)
            << " cannot use the mafia's kill on themself.";
            break;
      }
   }
   catch (const Game::Kill_failed &e) {
      err << "^TKill failed!^h";

      switch (e.reason) {
         case Game::Kill_failed::Reason::game_ended:
            err << "The game has already ended.";
            break;
         case Game::Kill_failed::Reason::caster_cannot_kill:
            err << _game_log->get_name(*e.caster)
            << " cannot use a kill ability right now.";
            break;
         case Game::Kill_failed::Reason::target_is_not_present:
            err << _game_log->get_name(*e.caster)
            << " cannot kill "
            << _game_log->get_name(*e.target)
            << ", because "
            << _game_log->get_name(*e.target)
            << " is no longer present in the game.";
            break;
         case Game::Kill_failed::Reason::caster_is_target:
            err << _game_log->get_name(*e.caster)
            << " is not allowed to kill themself.\n(nice try.)";
            break;
      }
   }
   catch (const Game::Heal_failed &e) {
      err << "^THeal failed!^h";

      switch (e.reason) {
         case Game::Heal_failed::Reason::game_ended:
            err << "The game has already ended.";
            break;
         case Game::Heal_failed::Reason::caster_cannot_heal:
            err << _game_log->get_name(*e.caster)
            << " cannot use a heal ability right now.";
            break;
         case Game::Heal_failed::Reason::target_is_not_present:
            err << _game_log->get_name(*e.caster)
            << " cannot heal "
            << _game_log->get_name(*e.target)
            << ", because "
            << _game_log->get_name(*e.target)
            << " is no longer present in the game.";
            break;
         case Game::Heal_failed::Reason::caster_is_target:
            err << _game_log->get_name(*e.caster)
            << " cannot heal themself.";
            break;
      }
   }
   catch (const Game::Investigate_failed &e) {
      err << "^TInvestigation failed!^h";

      switch (e.reason) {
         case Game::Investigate_failed::Reason::game_ended:
            err << "The game has already ended.";
            break;
         case Game::Investigate_failed::Reason::caster_cannot_investigate:
            err << _game_log->get_name(*e.caster)
            << " cannot investigate anybody right now.";
            break;
         case Game::Investigate_failed::Reason::target_is_not_present:
            err << _game_log->get_name(*e.caster)
            << " cannot investigate "
            << _game_log->get_name(*e.target)
            << ", because "
            << _game_log->get_name(*e.target)
            << " is no longer present in the game.";
            break;
         case Game::Investigate_failed::Reason::caster_is_target:
            err << _game_log->get_name(*e.caster)
            << " cannot investigate themself.";
            break;
      }
   }
   catch (const Game::Skip_failed &e) {
      err << "^TSkip failed!^hThe current ability, if one is showing, cannot be skipped.";
   }
   catch (const Game_log::Players_to_cards_mismatch &e) {
      err << "^TMismatch!^hA new game cannot begin with an unequal number of players and cards.";
   }
   catch (const Game_log::Player_not_found &e) {
      err << "^TPlayer not found!^hA player named ^c"
      << e.name
      << "^h could not be found.";
   }
   catch (const Event::Bad_commands &e) {
      err << "^TUnrecognised input!^hThe text that you entered couldn't be recognised.\n(enter ^chelp^h if you're unsure what to do.)";
   }
   catch (const screen::Setup::Bad_player_name &e) {
      err << "^TInvalid name!^hThe name of a player can only contain letters and numbers.";
   }
   catch (const screen::Setup::Player_already_exists &e) {
      err << "^TPlayer already exists!^hA player named ^c"
          << e.name
          << "^h has already been selected to play in the next game.\nNote that names are case-insensitive.)";
   }
   catch (const screen::Setup::Player_missing &e) {
      err << "^TMissing player!^hA player named ^c"
          << e.name
          << "^h could not be found.";
   }
   catch (const screen::Setup::Rolecard_unselected &e) {
      err << "^TRolecard not selected!^hNo copies of the rolecard with alias ^c"
          << e.role->alias()
          << "^h have been selected.";
   }
   catch (const screen::Setup::Wildcard_unselected &e) {
      err << "^TWildcard not selected!^hNo copies of the wildcard with alias ^c"
      << e.wildcard->alias()
      << "^h have been selected.";
   }
   catch (const screen::Setup::Missing_preset &e) {
      err << "^h^TMissing preset!^/There is no preset defined for the index ";
      err << e.index;
      err << ".";
   }
   catch (const screen::Setup::Bad_preset_string &e) {
       err << "^h^TInvalid input!^/The string ^c";
       err << e.str;
       err << "^/ could not be converted into a preset index. (i.e. a relatively-small integer)";
   }
   catch (const No_game_in_progress &e) {
      err << "^TNo game in progress!^hThere is no game in progress at the moment, and so game-related commands cannot be used.\n(enter ^cbegin^h to begin a new game, or ^chelp^h for a list of usable commands.)";
   }
   catch (const Begin_game_failed &e) {
      switch (e.reason) {
         case Begin_game_failed::Reason::game_already_in_progress:
             err << "^TGame in progress!^hA new game cannot begin until the current game ends.\n(enter ^cend^h to force the game to end early, or if the game has already ended and you want to return to the game setup screen.)";
            break;
      }
   }

   if (err.tellp() == 0) {
      refresh_output();
      clear_error_message();
      return true;
   } else {
      read_error_message(err.str());
      return false;
   }
}

bool maf::Console::input(std::string_view input) {
   auto v = parse_input(input);
   return do_commands(v);
}

const maf::Styled_text & maf::Console::output() const {
   return _output;
}

void maf::Console::read_output(std::string_view str) {
   _output = styled_text_from(str);
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

   read_output(ss.str());
}

const maf::Styled_text & maf::Console::error_message() const {
   return _error_message;
}

void maf::Console::read_error_message(std::string_view str) {
   _error_message = styled_text_from(str);
}

void maf::Console::clear_error_message() {
   _error_message.clear();
}

const maf::Base_Screen & maf::Console::current_screen() const {
   return *_screen_stack.back();
}

void maf::Console::push_screen(Base_Screen & screen) {
   _screen_stack.emplace_back(screen);
}

void maf::Console::pop_screen() {
   _screen_stack.pop_back();
}

const maf::Help_Screen * maf::Console::help_screen() const {
   return _help_screen.get();
}

bool maf::Console::has_help_screen() const {
   return static_cast<bool>(_help_screen);
}

void maf::Console::store_help_screen(Help_Screen *hs) {
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
   /* FIXME: set location where history is saved. */

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
