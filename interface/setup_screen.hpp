#ifndef MAFIA_INTERFACE_SETUP_SCREEN
#define MAFIA_INTERFACE_SETUP_SCREEN

#include <ostream>
#include <set>

#include "../logic/logic.hpp"

#include "game_log.hpp"
#include "names.hpp"
#include "screens.hpp"

namespace maf {
   // FIXME: Place Setup_screen into separate 'screen' child namespace
   // of 'maf'.
    
   struct Setup_Screen: Base_Screen {
      // Signifies that the given name is invalid for a player.
      struct Bad_player_name {
         std::string name;
      };

      // Signifies that a player with the given name already exists.
      struct Player_already_exists {
         std::string name;
      };

      // Signifies that no player with the given name exists.
      struct Player_missing {
         std::string name;
      };

      // Signifies that no copies of the given rolecard have been chosen.
      struct Rolecard_unselected {
         Rolecard_unselected(const Role &r): role{r} { }

         rkt::ref<const Role> role;
      };

      // Signifies that no copies of the given wildcard have been chosen.
      struct Wildcard_unselected {
         Wildcard_unselected(const Wildcard &w): wildcard{w} { }

         rkt::ref<const Wildcard> wildcard;
      };

      // Signifies that no preset is defined with index i.
      struct Missing_preset {
         int index;
      };

      // Signifies that the string `str` could not be converted into a preset
      // index.
      struct Bad_preset_string {
         std::string str;
      };

      // Signifies that a set of commands couldn't be interpreted.
      struct Bad_commands { };

      // Make a blank setup screen.
      Setup_Screen(Console & con)
         : Base_Screen{con}
      { }

      // The rulebook to be used in the pending game.
      const Rulebook & rulebook() const;
      // The names of the currently selected players.
      std::vector<std::string> player_names() const;
      // The IDs of the currently selected rolecards.
      std::vector<Role::ID> rolecard_ids() const;
      // The IDs of the currently selected wildcards.
      std::vector<Wildcard::ID> wildcard_ids() const;

      // Checks if a player with the given name already exists.
      bool has_player(const std::string &name) const;
      // Checks if at least one rolecard with the given alias has been chosen.
      bool has_rolecard(const std::string &alias) const;
      // Checks if at least one wildcard with the given alias has been chosen.
      bool has_wildcard(const std::string &alias) const;

      // The total number of players that have been chosen.
      std::size_t num_players() const;
      // The total number of rolecards that have been chosen.
      std::size_t num_rolecards() const;
      // The total number of wildcards that have been chosen.
      std::size_t num_wildcards() const;
      // The total number of cards that have been chosen.
      std::size_t num_cards() const;

      // Adds a new player with the given name.
      void add_player(const std::string &name);
      // Adds a single copy of the rolecard with the given alias.
      void add_rolecard(const std::string &alias);
      // Adds a single copy of the wildcard with the given alias.
      void add_wildcard(const std::string &alias);

      // Removes the player with the given name.
      void remove_player(const std::string &name);
      // Removes a single copy of the rolecard with the given alias.
      void remove_rolecard(const std::string &alias);
      // Removes a single copy of the wildcard with the given alias.
      void remove_wildcard(const std::string &alias);

      // Removes all of the players that have been chosen.
      void clear_all_players();
      // Removes all copies of the rolecard with the given alias.
      void clear_rolecards(const std::string &alias);
      // Removes all of the rolecards that have been chosen.
      void clear_all_rolecards();
      // Removes all copies of the wildcard with the given alias.
      void clear_wildcards(const std::string &alias);
      // Removes all of the wildcards that have been chosen.
      void clear_all_wildcards();
      // Removes all of the cards that have been chosen.
      void clear_all_cards();
      // Removes all of the players and cards that have been chosen.
      void clear_all();

      // Begin the pending game.
      void begin_pending_game();
      // Begin the preset with index `i`.
      void begin_preset(int i);
      // Begin a random preset.
      void begin_random_preset();

      // Handles the given commands, making alterations to the setup screen as
      // appropriate.
      //
      // @throws `error::bad_commands` if the commands couldn't be interpreted.
      bool handle_commands(const std::vector<std::string> & commands) override;

      // Write the setup screen to `os`.
      void write(std::ostream & os) const override;
      // Write a list of the selected players to `os`.
      void write_players_list(std::ostream &os) const;
      // Write a list of the selected cards to `os`.
      void write_cards_list(std::ostream &os) const;

      // Generate a help screen for setting up a new game.
      Help_Screen * get_help_screen() const override;

   private:
      struct Game_parameters {
         std::vector<std::string> player_names;
         std::vector<Role::ID> role_ids;
         std::vector<Wildcard::ID> wildcard_ids;
         Rulebook rulebook;
      };

      static constexpr std::size_t num_presets{2};
      static const std::array<Game_parameters, num_presets> _presets;

      Rulebook _rulebook{};
      std::set<std::string> _player_names{};
      std::map<Role::ID, std::size_t, Role_ID_full_name_compare> _role_ids{};
      std::map<Wildcard::ID, std::size_t> _wildcard_ids{};
   };
}

#endif
