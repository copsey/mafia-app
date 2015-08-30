#ifndef MAFIA_LOGIC_GAME
#define MAFIA_LOGIC_GAME

#include "player.hpp"
#include "rulebook.hpp"

namespace mafia {
   struct Game {
      // Signifies that a game was created with an unequal number of players and
      // cards.
      struct Players_to_cards_mismatch {
         std::size_t num_players;
         std::size_t num_cards;
      };

      // Signifies that the game cannot continue right now.
      struct Cannot_continue {
         enum class Reason {
            game_ended,
            lynch_can_occur,
            mafia_can_use_kill
         };

         Reason reason;
      };

      // Signifies that a lynch cannot occur at the moment.
      struct Lynch_failed {
         enum class Reason {
            game_ended,
            bad_timing
         };

         Reason reason;
      };

      // Signifies that the caster cannot cast a lynch vote against the target.
      struct Lynch_vote_failed {
         enum class Reason {
            game_ended,
            bad_timing,
            caster_is_not_present,
            target_is_not_present,
            caster_is_target
         };

         Lynch_vote_failed(const Player &caster,
                           const Player *target,
                           Reason reason)
         : caster{caster}, target{target}, reason{reason} { }

         rkt::ref<const Player> caster;
         const Player *target;
         Reason reason;
      };

      // Signifies that a duel cannot occur between the caster and the target.
      struct Duel_failed {
         enum class Reason {
            game_ended,
            bad_timing,
            caster_is_not_present,
            target_is_not_present,
            caster_is_target,
            caster_has_no_duel
         };

         Duel_failed(const Player &caster,
                     const Player &target,
                     Reason reason)
         : caster{caster}, target{target}, reason{reason} { }

         rkt::ref<const Player> caster;
         rkt::ref<const Player> target;
         Reason reason;
      };

      // Signifies that the caster cannot perform the mafia's kill on the target.
      struct Mafia_kill_failed {
         enum class Reason {
            game_ended,
            bad_timing,
            already_used,
            caster_is_not_present,
            caster_is_not_in_mafia,
            target_is_not_present,
            caster_is_target
         };

         Mafia_kill_failed(const Player &caster,
                           const Player &target,
                           Reason reason)
         : caster{caster}, target{target}, reason{reason} { }

         rkt::ref<const Player> caster;
         rkt::ref<const Player> target;
         Reason reason;
      };

      // Signifies that an ability cannot be skipped right now.
      struct Skip_failed { };

      // Start a new game with the given parameters, assigning each player an
      // initial role.
      // Note that this could lead to the game immediately ending.
      Game(const std::vector<std::string> &player_names,
           const std::vector<Role::ID> &role_ids,
           const std::vector<Wildcard::ID> &wildcard_ids,
           const Rulebook &rulebook = Rulebook{});

      // The rulebook being used to run the game.
      const Rulebook & rulebook() const;

      // The participating players, both present and not present.
      const std::vector<Player> & players() const;
      // A vector containing every player remaining.
      std::vector<rkt::ref<const Player>> remaining_players() const;
      // A vector containing every player remaining with the given alignment.
      std::vector<rkt::ref<const Player>> remaining_players(Role::Alignment alignment) const;
      // The number of players remaining.
      std::size_t num_players_left() const;
      // The number of players remaining with the given alignment.
      std::size_t num_players_left(Role::Alignment alignment) const;

      // The current in-game date.
      Date date() const;
      // The current in-game time.
      Time time() const;

      // Proceeds to the next day.
      void begin_day();

      // Casts a lynch vote by the caster against the target.
      void cast_lynch_vote(const Player &caster, const Player &target);
      // Clears the caster's lynch vote.
      void clear_lynch_vote(const Player &caster);
      // The player that would be lynched if the lynch votes were to be
      // processed now, or nullptr if no player would be lynched.
      const Player * next_lynch_victim() const;
      // Submits the lynch votes, and lynches the next victim.
      // Returns the player lynched if applicable, or nullptr if no player was
      // lynched.
      const Player * process_lynch_votes();
      // Whether a lynching can still take place today.
      bool lynch_can_occur() const;
      // Whether the lynch votes have already been processed today.
      bool lynch_has_occurred() const;

      // Stage a duel initiated by the caster against the target.
      void stage_duel(const Player &caster, const Player &target);

      // Proceeds to the next night.
      void begin_night();

      // Chooses a caster and target for the mafia's nightly kill.
      void cast_mafia_kill(const Player &caster, const Player &target);
      // Skips the mafia's nightly kill.
      void skip_mafia_kill();
      // Whether or not the mafia can choose a caster and target for their
      // nightly kill.
      bool mafia_can_use_kill() const;

      // Whether or not the game has ended.
      bool has_ended() const;

   private:
      std::vector<Player> _players;
      Rulebook _rulebook;

      bool _has_ended{false};

      Date _date{0};
      Time _time{Time::night};

      bool _lynch_can_occur{false};

      bool _mafia_can_use_kill{false};
      Player *_mafia_kill_caster;
      Player *_mafia_kill_target;

      // Tries to end the current night, continuing to the next day
      // Returns whether this succeeded.
      bool try_to_end_night();

      // Check if the game has ended - that is, if every player still present
      // in the game has had their peace condition resolved.
      // Returns true if the game has ended, in which case the winning players
      // are determined. Returns false if the game has not ended.
      bool try_to_end();
   };
}

#endif
