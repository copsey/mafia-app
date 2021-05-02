#include <charconv>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "../util/algorithm.hpp"
#include "../util/fstream.hpp"
#include "../util/random.hpp"
#include "../util/string.hpp"

#include "command.hpp"
#include "console.hpp"
#include "names.hpp"


namespace maf {
	Console::Console() : _setup_screen{*this} {
		refresh_output();
	}

	bool Console::do_commands(const CmdSequence & commands) {
		std::stringstream err{}; // Write an error here if something goes wrong.
		TextParams err_params = {}; // (include parameters for error message here)

		try {
			active_screen().do_commands(commands);

			/* FIXME: add  "list w", "list w v", "list w m", "list w f". */

			/* FIXME: "add p A B C" should result in players A, B, C all being chosen. */

			/* FIXME: enter "auto" to automatically choose enough random cards for the currently-selected players to start a new game. */

			/* FIXME: list p random, a utility command to generate a list of the players in a game, in a random order.
			(for example, when asking people to choose their lynch votes, without the option to change.)
			list p should be context-aware, i.e. it should show pending players if no game is in progress, and actual players if a game is in progress. */

			/* FIXME: enter "skip" to skip a player's ability use at night and the mafia's kill. This should result in a yes/no screen to be safe. */
		}
		catch (const core::Rulebook::Missing_role_alias &e) {
			err_params["alias"] = escaped(e.alias);

			err << "=Invalid alias!=\n\nNo role could be found whose alias is @{alias}@.\nNote that aliases are case-sensitive.\n(enter @list r@ to see a list of each role and its alias.)";
		}
		catch (const core::Rulebook::Missing_wildcard_alias &e) {
			err_params["alias"] = escaped(e.alias);

			err << "=Invalid alias!=\n\nNo wildcard could be found whose alias is @{alias}@.\nNote that aliases are case-sensitive.\n(enter @list w@ to see a list of each wildcard and its alias.)";
		}
		catch (const core::Game::Kick_failed &e) {
			err_params["player"] = escaped(_game_log->get_name(e.player));

			err << "=Kick failed!=\n\n";

			switch (e.reason) {
			using Reason = core::Game::Kick_failed::Reason;
			case Reason::game_ended:
				err << "{player} could not be kicked from the game, because the game has already ended.";
				break;
			case Reason::bad_timing:
				err << "Players can only be kicked from the game during the day.";
				break;
			case Reason::already_kicked:
				err << "{player} has already been kicked from the game";
				break;
			}
		}
		catch (const core::Game::Lynch_failed &e) {
			err << "=Lynch failed!=\n\n";

			switch (e.reason) {
			using Reason = core::Game::Lynch_failed::Reason;
			case Reason::game_ended:
				err << "The game has already ended.";
				break;
			case Reason::bad_timing:
				err << "A lynch cannot occur at this moment in time.";
				break;
			}
		}
		catch (const core::Game::Lynch_vote_failed &e) {
			err_params["voter"] = escaped(_game_log->get_name(e.voter));
			if (e.target) {
				err_params["target"] = escaped(_game_log->get_name(*(e.target)));
			}

			err << "=Lynch vote failed!=\n\n";

			switch (e.reason) {
			using Reason = core::Game::Lynch_vote_failed::Reason;
			case Reason::game_ended:
				err << "The game has already ended.";
				break;
			case Reason::bad_timing:
				err << "No lynch votes can be cast at this moment in time.";
				break;
			case Reason::voter_is_not_present:
				err << "{voter} is unable to cast a lynch vote, as they are no longer present in the game.";
				break;
			case Reason::target_is_not_present:
				err << "{voter} cannot cast a lynch vote against {target}, because {target} is no longer present in the game.";
				break;
			case Reason::voter_is_target:
				err << "A player cannot cast a lynch vote against themself.";
				break;
			}
		}
		catch (const core::Game::Duel_failed &e) {
			err_params["caster"] = escaped(_game_log->get_name(e.caster));
			err_params["target"] = escaped(_game_log->get_name(e.target));

			err << "=Duel failed!=\n\n";

			switch (e.reason) {
			using Reason = core::Game::Duel_failed::Reason;
			case Reason::game_ended:
				err << "The game has already ended.";
				break;
			case Reason::bad_timing:
				err << "A duel can only take place during the day.";
				break;
			case Reason::caster_is_not_present:
				err << "{caster} is unable to initiate a duel, as they are no longer present in the game.";
				break;
			case Reason::target_is_not_present:
				err << "{caster} cannot initiate a duel against {target}, because {target} is no longer present in the game.";
				break;
			case Reason::caster_is_target:
				err << "A player cannot duel themself.";
				break;
			case Reason::caster_has_no_duel:
				err << "{caster} has no duel ability to use.";
				break;
			case Reason::bad_probability:
				err << "An error occurred when calculating the probabilities needed to simulate the duel.";
				break;
			}
		}
		catch (const core::Game::Begin_night_failed &e) {
			err << "=Cannot begin night!=\n\n";

			switch (e.reason) {
			using Reason = core::Game::Begin_night_failed::Reason;
			case Reason::game_ended:
				err << "The game has ended, and so cannot be continued.\n(enter @end@ to return to the game setup screen.)";
				break;
			case Reason::already_night:
				err << "It is already nighttime.";
			case Reason::lynch_can_occur:
				err << "The next night cannot begin until a lynch has taken place.\n(enter @lynch@ to submit the current lynch votes.)";
				break;
			}
		}
		catch (const core::Game::Choose_fake_role_failed &e) {
			err_params["player"] = escaped(_game_log->get_name(e.player));

			err << "=Choose fake role failed!=\n\n";

			switch (e.reason) {
			using Reason = core::Game::Choose_fake_role_failed::Reason;
			case Reason::game_ended:
				err << "The game has already ended.";
				break;
			case Reason::bad_timing:
				err << "Wait until night before choosing a fake role.";
				break;
			case Reason::player_is_not_faker:
				err << "{player} doesn't need to be given a fake role.";
				break;
			case Reason::already_chosen:
				err << "{player} has already been given a fake role.";
				break;
			}
		}
		catch (const core::Game::Mafia_kill_failed &e) {
			err_params["caster"] = escaped(_game_log->get_name(e.caster));
			err_params["target"] = escaped(_game_log->get_name(e.target));

			err << "=Mafia kill failed!=\n\n";

			switch (e.reason) {
			using Reason = core::Game::Mafia_kill_failed::Reason;
			case Reason::game_ended:
				err << "The game has already ended.";
				break;
			case Reason::bad_timing:
				err << "The mafia can only use their kill during the night.";
				break;
			case Reason::already_used:
				err << "Either the mafia have already used their kill this night, or there are no members of the mafia remaining to perform a kill.";
				break;
			case Reason::caster_is_not_present:
				err << "{caster} cannot perform the mafia's kill, as they are no longer in the game.";
				break;
			case Reason::caster_is_not_in_mafia:
				err << "{caster} cannot perform the mafia's kill, as they are not part of the mafia.";
				break;
			case Reason::target_is_not_present:
				err << "{target} cannot be targetted to kill by the mafia, as they are no longer in the game.";
				break;
			case Reason::caster_is_target:
				err << "{caster} is not allowed to kill themself.\n(nice try.)";
				break;
			}
		}
		catch (const core::Game::Kill_failed &e) {
			err_params["caster"] = escaped(_game_log->get_name(e.caster));
			err_params["target"] = escaped(_game_log->get_name(e.target));

			err << "=Kill failed!=\n\n";

			switch (e.reason) {
			using Reason = core::Game::Kill_failed::Reason;
			case Reason::game_ended:
				err << "The game has already ended.";
				break;
			case Reason::caster_cannot_kill:
				err << "{caster} cannot use a kill ability right now.";
				break;
			case Reason::target_is_not_present:
				err << "{caster} cannot kill {target}, because {target} is no longer present in the game.";
				break;
			case Reason::caster_is_target:
				err << "{caster} is not allowed to kill themself.\n(nice try.)";
				break;
		}
		}
		catch (const core::Game::Heal_failed &e) {
			err_params["caster"] = escaped(_game_log->get_name(e.caster));
			err_params["target"] = escaped(_game_log->get_name(e.target));

			err << "=Heal failed!=\n\n";

			switch (e.reason) {
			using Reason = core::Game::Heal_failed::Reason;
			case Reason::game_ended:
				err << "The game has already ended.";
				break;
			case Reason::caster_cannot_heal:
				err << "{caster} cannot use a heal ability right now.";
				break;
			case Reason::target_is_not_present:
				err << "{caster} cannot heal {target}, because {target} is no longer present in the game.";
				break;
			case Reason::caster_is_target:
				err << "{caster} is not allowed to heal themself.";
				break;
			}
		}
		catch (const core::Game::Investigate_failed &e) {
			err_params["caster"] = escaped(_game_log->get_name(e.caster));
			err_params["target"] = escaped(_game_log->get_name(e.target));

			err << "=Investigation failed!=\n\n";

			switch (e.reason) {
			using Reason = core::Game::Investigate_failed::Reason;
			case Reason::game_ended:
				err << "The game has already ended.";
				break;
			case Reason::caster_cannot_investigate:
				err << "{caster} cannot investigate anybody right now.";
				break;
			case Reason::target_is_not_present:
				err << "{caster} cannot investigate {target}, because {target} is no longer present in the game.";
				break;
			case Reason::caster_is_target:
				err << "{caster} is not allowed to investigate themself.";
				break;
			}
		}
		catch (const core::Game::Peddle_failed & e) {
			err_params["caster"] = escaped(_game_log->get_name(e.caster));
			err_params["target"] = escaped(_game_log->get_name(e.target));

			err << "=Peddle failed!=\n\n";

			switch (e.reason) {
			using Reason = core::Game::Peddle_failed::Reason;
			case Reason::game_ended:
				err << "The game has already ended.";
				break;
			case Reason::caster_cannot_peddle:
				err << "{caster} cannot use this ability right now.";
				break;
			case Reason::target_is_not_present:
				err << "{caster} cannot target {target}, because {target} is no longer present in the game.";
				break;
			case Reason::caster_is_target:
				err << "{caster} is not allowed to target themself.";
				break;
			}
		}
		catch (const core::Game::Skip_failed &e) {
			err << "=Skip failed!=\n\nThe current ability, if one is showing, cannot be skipped.";
		}
		catch (const Game_log::Players_to_cards_mismatch &e) {
			err << "=Mismatch!=\n\nA new game cannot begin with an unequal number of players and cards.";
		}
		catch (const Game_log::Player_not_found &e) {
			err_params["player"] = escaped(e.name);

			err << "=Player not found!=\n\nA player named @{player}@ could not be found.";
		}
		catch (const Screen::Bad_commands &e) {
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
			err_params["alias"] = escaped(e.role.alias());

			err << "=Rolecard not selected!=\n\nNo copies of the rolecard with alias @{alias}@ have been selected.";
		}
		catch (const Setup_screen::Wildcard_unselected &e) {
			err_params["alias"] = escaped(e.wildcard.alias());

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
			err_params["index"] = e.index;

			err << "=Error!=\n\nThere is no preset defined for the index {index}.";
		}
		catch (const Generic_error & error) {
			err_params = error.params;
			err << error.msg;
		}

		if (err.tellp() == 0) {
			refresh_output();
			clear_error_message();
			return true;
		} else {
			read_error_message(err.str(), err_params);
			return false;
		}
	}

	bool Console::input(string_view input) {
		auto v = parse_input(input);
		return do_commands(v);
	}

	const StyledText & Console::output() const {
		return _output;
	}

	void Console::read_output(string_view contents) {
		try {
			_output = format_text(contents);
		} catch (format_text_error const& error) {
			string msg = "\n\nERROR: ";
			msg += error.message();
			msg += " in the following string:\n\n";

			_output.clear();
			_output.emplace_back("Error!", StyledString::title_attributes);
			_output.emplace_back(msg, StyledString::default_attributes);
			_output.emplace_back(string{error.input}, StyledString::monospace_attributes);
		}
	}

	void Console::refresh_output() {
		string str;
		active_screen().write(str);
		auto substr = util::drop_whitespace_from_end(str);
		read_output(substr);
	}

	const StyledText & Console::error_message() const {
		return _error_message;
	}

	void Console::read_error_message(string_view raw_err_msg, const TextParams & params) {
		// TODO: Catch exceptions when preprocessing the text.
		auto preprocessed_err_msg = preprocess_text(raw_err_msg, params);

		// TODO: Catch exceptions when formatting the text.
		// Might make sense to show another error message in this case, explaining
		// why the original error message couldn't be parsed?
		_error_message = format_text(preprocessed_err_msg);
	}

	void Console::clear_error_message() {
		_error_message.clear();
	}

	Screen & Console::active_screen() {
		const Console * const_this = this;
		return const_cast<Screen &>(const_this->active_screen());
	}

	const Screen & Console::active_screen() const {
		if (_help_screen) {
			return *_help_screen;
		} else if (_question) {
			return *_question;
		} else if (_game_log) {
			return _game_log->active_screen();
		} else {
			return _setup_screen;
		}
	}

	void Console::end_game() {
		// TODO: Choose file location where history is saved
		string_view path{"/Users/Jack/Documents/Developer/Projects/mafia/misc/game_history.txt"};

		if (has_game()) {
			std::time_t t = std::time(nullptr);

			ofstream ofs{path, ofstream::app};
			ofs << "\n====== ";
			ofs << std::put_time(std::localtime(&t), "%F %T");
			ofs << " ======\n\n";

			string transcript;
			_game_log->write_transcript(transcript);
			ofs << transcript;

			_game_log.reset();
		}
	}

	const core::Rulebook & Console::active_rulebook() const {
		if (has_game()) {
			return _game_log->game().rulebook();
		} else {
			return _setup_screen.rulebook();
		}
	}
}
