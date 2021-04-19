#include <charconv>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "../util/algorithm.hpp"
#include "../util/random.hpp"
#include "../util/string.hpp"

#include "command.hpp"
#include "console.hpp"
#include "names.hpp"

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
	},
	maf::Console::Game_parameters{
		{"Alice", "Bob", "Charlie", "Daisy", "Eduardo", "Fiona"},
		{maf::Role::ID::godfather, maf::Role::ID::actor, maf::Role::ID::serial_killer, maf::Role::ID::village_idiot, maf::Role::ID::village_idiot, maf::Role::ID::village_idiot},
		{},
		maf::Rulebook{}
	}
};

maf::Console::Console() {
	refresh_output();
}

bool maf::Console::do_commands(const std::vector<std::string_view> & commands) {
	std::stringstream err{}; // Write an error here if something goes wrong.
	TextParams err_params = {}; // (include parameters for error message here)

	try {
		if (commands.size() == 0) {
			err << "=Missing input!=\n\nEntering a blank input has no effect.\n(enter @help@ if you're unsure what to do.)";
		}
		else if (commands_match(commands, {"help"})) {
			if (has_game()) {
				store_help_screen(new Event_Help_Screen{_game_log->current_event()});
			} else {
				store_help_screen(new Setup_Help_Screen());
			}
		}
		else if (commands_match(commands, {"help", "r", ""})) {
			RoleRef r_ref = commands[2];

			try {
				auto& role = active_rulebook().look_up(r_ref);
				store_help_screen(new Role_Info_Screen(role));
			} catch (std::out_of_range) {
				throw Rulebook::Missing_role_alias{std::string(commands[2])};
			}
		}
		else if (commands_match(commands, {"list", "r"})) {
			store_help_screen(new List_Roles_Screen(active_rulebook()));
		}
		else if (commands_match(commands, {"list", "r", "v"})) {
			store_help_screen(new List_Roles_Screen(active_rulebook(), Alignment::village));
		}
		else if (commands_match(commands, {"list", "r", "m"})) {
			store_help_screen(new List_Roles_Screen(active_rulebook(), Alignment::mafia));
		}
		else if (commands_match(commands, {"list", "r", "f"})) {
			store_help_screen(new List_Roles_Screen(active_rulebook(), Alignment::freelance));
		}
		else if (commands_match(commands, {"info", ""})) {
			if (!has_game()) {
				err << "=No game in progress!=\n\nThere is no game in progress to display information about.";
			} else {
				auto& approx_name = commands[1];
				auto& player = _game_log->find_player(approx_name);

				store_help_screen(new Player_Info_Screen{player, *_game_log});
			}
		}
		else if (has_help_screen()) {
			if (commands_match(commands, {"ok"})) {
				clear_help_screen();
			} else {
				err << "=Invalid input!=\n\nPlease leave the help screen that is currently being displayed before trying to do anything else.\n(this is done by entering @ok@)";
			}
		}
		else if (has_question()) {
			if (_question->do_commands(commands)) {
				clear_question();
			}
		}
		else if (commands_match(commands, {"end"})) {
			if (!has_game()) {
				err << "=No game in progress!=\n\nThere is no game in progress to end.";
			} else if (dynamic_cast<const Game_ended *>(&_game_log->current_event())) {
				end_game();
			} else {
				store_question(new Confirm_end_game(*this));
			}
		}
		else if (has_game()) {
			_game_log->do_commands(commands);
		}
		else if (commands_match(commands, {"begin"})) {
			begin_pending_game();
		}
		else if (commands_match(commands, {"preset"})) {
			std::uniform_int_distribution<int> uid{0, static_cast<int>(num_presets) - 1};
			begin_preset(uid(util::random_engine));
		}
		else if (commands_match(commands, {"preset", ""})) {
			int i;
			auto& str = commands[1];

			if (auto result = std::from_chars(std::begin(str), std::end(str), i);
				result.ec == std::errc{})
			{
				begin_preset(i);
			} else {
				err_params["str"] = escaped(str);
				
				err << "=Invalid input!=\n\nThe string @{str}@ could not be converted into a preset index. (i.e. a relatively-small integer)";
			}
		}
		else {
			_setup_screen.do_commands(commands);
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
		err_params["alias"] = escaped(e.alias);
		
		err << "=Invalid alias!=\n\nNo role could be found whose alias is @{alias}@.\nNote that aliases are case-sensitive.\n(enter @list r@ to see a list of each role and its alias.)";
	}
	catch (const Rulebook::Missing_wildcard_alias &e) {
		err_params["alias"] = escaped(e.alias);
		
		err << "=Invalid alias!=\n\nNo wildcard could be found whose alias is @{alias}@.\nNote that aliases are case-sensitive.\n(enter @list w@ to see a list of each wildcard and its alias.)";
	}
	catch (const Game::Kick_failed &e) {
		err_params["player"] = escaped(_game_log->get_name(*e.player));
		
		err << "=Kick failed!=\n\n";

		switch (e.reason) {
			case Game::Kick_failed::Reason::game_ended:
				err << "{player} could not be kicked from the game, because the game has already ended.";
				break;

			case Game::Kick_failed::Reason::bad_timing:
				err << "Players can only be kicked from the game during the day.";
				break;

			case Game::Kick_failed::Reason::already_kicked:
				err << "{player} has already been kicked from the game";
				break;
		}
	}
	catch (const Game::Lynch_failed &e) {
		err << "=Lynch failed!=\n\n";

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
		err_params["voter"] = escaped(_game_log->get_name(*e.voter));
		err_params["target"] = escaped(_game_log->get_name(*e.target));
		
		err << "=Lynch vote failed!=\n\n";

		switch (e.reason) {
			case Game::Lynch_vote_failed::Reason::game_ended:
				err << "The game has already ended.";
				break;

			case Game::Lynch_vote_failed::Reason::bad_timing:
				err << "No lynch votes can be cast at this moment in time.";
				break;

			case Game::Lynch_vote_failed::Reason::voter_is_not_present:
				err << "{voter} is unable to cast a lynch vote, as they are no longer present in the game.";
				break;

			case Game::Lynch_vote_failed::Reason::target_is_not_present:
				err << "{voter} cannot cast a lynch vote against {target}, because {target} is no longer present in the game.";
				break;

			case Game::Lynch_vote_failed::Reason::voter_is_target:
				err << "A player cannot cast a lynch vote against themself.";
				break;
		}
	}
	catch (const Game::Duel_failed &e) {
		err_params["caster"] = escaped(_game_log->get_name(*e.caster));
		err_params["target"] = escaped(_game_log->get_name(*e.target));

		err << "=Duel failed!=\n\n";

		switch (e.reason) {
			case Game::Duel_failed::Reason::game_ended:
				err << "The game has already ended.";
				break;

			case Game::Duel_failed::Reason::bad_timing:
				err << "A duel can only take place during the day.";
				break;

			case Game::Duel_failed::Reason::caster_is_not_present:
				err << "{caster} is unable to initiate a duel, as they are no longer present in the game.";
				break;

			case Game::Duel_failed::Reason::target_is_not_present:
				err << "{caster} cannot initiate a duel against {target}, because {target} is no longer present in the game.";
				break;

			case Game::Duel_failed::Reason::caster_is_target:
				err << "A player cannot duel themself.";
				break;

			case Game::Duel_failed::Reason::caster_has_no_duel:
				err << "{caster} has no duel ability to use.";
				break;
			
			case Game::Duel_failed::Reason::bad_probability:
				err << "An error occurred when calculating the probabilities needed to simulate the duel.";
				break;
		}
	}
	catch (const Game::Begin_night_failed &e) {
		err << "=Cannot begin night!=\n\n";

		switch (e.reason) {
			case Game::Begin_night_failed::Reason::game_ended:
				err << "The game has ended, and so cannot be continued.\n(enter @end@ to return to the game setup screen.)";
				break;

			case Game::Begin_night_failed::Reason::already_night:
				err << "It is already nighttime.";

			case Game::Begin_night_failed::Reason::lynch_can_occur:
				err << "The next night cannot begin until a lynch has taken place.\n(enter @lynch@ to submit the current lynch votes.)";
				break;
		}
	}
	catch (const Game::Choose_fake_role_failed &e) {
		err_params["player"] = escaped(_game_log->get_name(*e.player));
		
		err << "=Choose fake role failed!=\n\n";

		switch (e.reason) {
			case Game::Choose_fake_role_failed::Reason::game_ended:
				err << "The game has already ended.";
				break;

			case Game::Choose_fake_role_failed::Reason::bad_timing:
				err << "Wait until night before choosing a fake role.";
				break;

			case Game::Choose_fake_role_failed::Reason::player_is_not_faker:
				err << "{player} doesn't need to be given a fake role.";
				break;

			case Game::Choose_fake_role_failed::Reason::already_chosen:
				err << "{player} has already been given a fake role.";
				break;
		}
	}
	catch (const Game::Mafia_kill_failed &e) {
		err_params["caster"] = escaped(_game_log->get_name(*e.caster));
		err_params["target"] = escaped(_game_log->get_name(*e.target));
		
		err << "=Mafia kill failed!=\n\n";

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
				err << "{caster} cannot perform the mafia's kill, as they are no longer in the game.";
				break;
			case Game::Mafia_kill_failed::Reason::caster_is_not_in_mafia:
				err << "{caster} cannot perform the mafia's kill, as they are not part of the mafia.";
				break;
			case Game::Mafia_kill_failed::Reason::target_is_not_present:
				err << "{target} cannot be targetted to kill by the mafia, as they are no longer in the game.";
				break;
			case Game::Mafia_kill_failed::Reason::caster_is_target:
				err << "{caster} is not allowed to kill themself.\n(nice try.)";
				break;
		}
	}
	catch (const Game::Kill_failed &e) {
		err_params["caster"] = escaped(_game_log->get_name(*e.caster));
		err_params["target"] = escaped(_game_log->get_name(*e.target));
		
		err << "=Kill failed!=\n\n";

		switch (e.reason) {
			case Game::Kill_failed::Reason::game_ended:
				err << "The game has already ended.";
				break;
			case Game::Kill_failed::Reason::caster_cannot_kill:
				err << "{caster} cannot use a kill ability right now.";
				break;
			case Game::Kill_failed::Reason::target_is_not_present:
				err << "{caster} cannot kill {target}, because {target} is no longer present in the game.";
				break;
			case Game::Kill_failed::Reason::caster_is_target:
				err << "{caster} is not allowed to kill themself.\n(nice try.)";
				break;
		}
	}
	catch (const Game::Heal_failed &e) {
		err_params["caster"] = escaped(_game_log->get_name(*e.caster));
		err_params["target"] = escaped(_game_log->get_name(*e.target));
		
		err << "=Heal failed!=\n\n";

		switch (e.reason) {
			case Game::Heal_failed::Reason::game_ended:
				err << "The game has already ended.";
				break;

			case Game::Heal_failed::Reason::caster_cannot_heal:
				err << "{caster} cannot use a heal ability right now.";
				break;

			case Game::Heal_failed::Reason::target_is_not_present:
				err << "{caster} cannot heal {target}, because {target} is no longer present in the game.";
				break;

			case Game::Heal_failed::Reason::caster_is_target:
				err << "{caster} is not allowed to heal themself.";
				break;
		}
	}
	catch (const Game::Investigate_failed &e) {
		err_params["caster"] = escaped(_game_log->get_name(*e.caster));
		err_params["target"] = escaped(_game_log->get_name(*e.target));
		
		err << "=Investigation failed!=\n\n";

		switch (e.reason) {
			case Game::Investigate_failed::Reason::game_ended:
				err << "The game has already ended.";
				break;

			case Game::Investigate_failed::Reason::caster_cannot_investigate:
				err << "{caster} cannot investigate anybody right now.";
				break;

			case Game::Investigate_failed::Reason::target_is_not_present:
				err << "{caster} cannot investigate {target}, because {target} is no longer present in the game.";
				break;

			case Game::Investigate_failed::Reason::caster_is_target:
				err << "{caster} is not allowed to investigate themself.";
				break;
		}
	}
	catch (Game::Peddle_failed const& e) {
		err_params["caster"] = escaped(_game_log->get_name(*e.caster));
		err_params["target"] = escaped(_game_log->get_name(*e.target));
		
		err << "=Peddle failed!=\n\n";

		switch (e.reason) {
			case Game::Peddle_failed::Reason::game_ended:
				err << "The game has already ended.";
				break;

			case Game::Peddle_failed::Reason::caster_cannot_peddle:
				err << "{caster} cannot use this ability right now.";
				break;

			case Game::Peddle_failed::Reason::target_is_not_present:
				err << "{caster} cannot target {target}, because {target} is no longer present in the game.";
				break;

			case Game::Peddle_failed::Reason::caster_is_target:
				err << "{caster} is not allowed to target themself.";
				break;
		}
	}
	catch (const Game::Skip_failed &e) {
		err << "=Skip failed!=\n\nThe current ability, if one is showing, cannot be skipped.";
	}
	catch (const Game_log::Players_to_cards_mismatch &e) {
		err << "=Mismatch!=\n\nA new game cannot begin with an unequal number of players and cards.";
	}
	catch (const Game_log::Player_not_found &e) {
		err_params["player"] = escaped(e.name);
		
		err << "=Player not found!=\n\nA player named @{player}@ could not be found.";
	}
	catch (const Event::Bad_commands &e) {
		err << "=Unrecognised input!=\n\nThe text that you entered couldn't be recognised.\n(enter @help@ if you're unsure what to do.)";
	}
	catch (const Setup_screen::Bad_player_name &e) {
		err << "=Invalid name!=\n\nThe name of a player can only contain letters and numbers.";
	}
	catch (const Setup_screen::Player_already_exists &e) {
		err_params["player"] = escaped(e.name);
		
		err << "=Player already exists!=\n\nA player named @{player}@ has already been selected to play in the next game.\nNote that names are case-insensitive.)";
	}
	catch (const Setup_screen::Player_missing &e) {
		err_params["player"] = escaped(e.name);
		
		err << "=Missing player!=\n\nA player named @{player}@ could not be found.";
	}
	catch (const Setup_screen::Rolecard_unselected &e) {
		err_params["alias"] = escaped(e.role->alias());
		
		err << "=Rolecard not selected!=\n\nNo copies of the rolecard with alias @{alias}@ have been selected.";
	}
	catch (const Setup_screen::Wildcard_unselected &e) {
		err_params["alias"] = escaped(e.wildcard->alias());
		
		err << "=Wildcard not selected!=\n\nNo copies of the wildcard with alias @{alias}@ have been selected.";
	}
	catch (const Setup_screen::Bad_commands &e) {
		err << "=Unrecognised input!=\n\nThe text that you entered couldn't be recognised.\n(enter @help@ if you're unsure what to do.)";
	}
	catch (const Question::Bad_commands &e) {
		err << "=Invalid input!=\n\nPlease answer the question being shown before trying to do anything else.";
	}
	catch (const No_game_in_progress &e) {
		err << "=No game in progress!=\n\nThere is no game in progress at the moment, and so game-related commands cannot be used.\n(enter @begin@ to begin a new game, or @help@ for a list of usable commands.)";
	}
	catch (const Begin_game_failed &e) {
		switch (e.reason) {
			case Begin_game_failed::Reason::game_already_in_progress:
				 err << "=Game in progress!=\n\nA new game cannot begin until the current game ends.\n(enter @end@ to force the game to end early, or if the game has already ended and you want to return to the game setup screen.)";
				break;
		}
	}
	catch (const Missing_preset &e) {
		err_params["index"] = std::to_string(e.index);
		
		err << "=Missing preset!=There is no preset defined for the index {index}.";
	}

	if (err.tellp() == 0) {
		refresh_output();
		clear_error_message();
		return true;
	}
	else {
		read_error_message(err.str(), err_params);
		return false;
	}
}

bool maf::Console::input(std::string_view input) {
	auto v = parse_input(input);
	return do_commands(v);
}

maf::StyledText const& maf::Console::output() const {
	return _output;
}

void maf::Console::read_output(std::string_view contents) {
	try {
		_output = format_text(contents);
	} catch (format_text_error const& error) {
		std::string msg = "\n\nERROR: ";
		error.write(msg);
		msg += " in the following string:\n\n";

		_output.clear();
		_output.emplace_back("Error!", StyledString::title_attributes);
		_output.emplace_back(msg, StyledString::default_attributes);
		_output.emplace_back(std::string{error.input}, StyledString::monospace_attributes);
	}
}

void maf::Console::refresh_output() {
	const Screen *current_screen = nullptr;
	std::string str;

	if (has_help_screen()) {
		current_screen = _help_screen.get();
	} else if (has_question()) {
		current_screen = _question.get();
	} else if (has_game()) {
		current_screen = &(_game_log->current_event());
	}

	if (current_screen) {
		current_screen->write(str);
	} else {
		std::stringstream ss;
		_setup_screen.write(ss);
		str = ss.str();
	}

	read_output(str);
}

maf::StyledText const& maf::Console::error_message() const
{
	return _error_message;
}

void maf::Console::read_error_message(std::string_view raw_err_msg, TextParams const& params)
{
	// TODO: Catch exceptions when preprocessing the text.
	auto preprocessed_err_msg = preprocess_text(raw_err_msg, params);
	
	// TODO: Catch exceptions when formatting the text.
	// Might make sense to show another error message in this case, explaining
	// why the original error message couldn't be parsed?
	_error_message = format_text(preprocessed_err_msg);
}

void maf::Console::clear_error_message()
{
	_error_message.clear();
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
	// TODO: Choose file location where history is saved

	if (has_game()) {
		std::time_t t = std::time(nullptr);

		std::ofstream ofs{"/Users/Jack/Documents/Developer/Projects/mafia/misc/game_history.txt", std::ofstream::app};
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
                              const Rulebook &rulebook)
{
	if (has_game()) throw Begin_game_failed{Begin_game_failed::Reason::game_already_in_progress};
	_game_log.reset(new Game_log{pl_names, r_ids, w_ids, rulebook});
}

void maf::Console::begin_pending_game()
{
	if (has_game()) throw Begin_game_failed{Begin_game_failed::Reason::game_already_in_progress};
	_game_log = _setup_screen.new_game_log();
}

void maf::Console::begin_preset(int i)
{
	if (i >= 0 && i < num_presets) {
		Game_parameters params = _presets[i];
		begin_game(params.player_names, params.role_ids, params.wildcard_ids, params.rulebook);
	} else {
		throw Missing_preset{i};
	}
}
