#ifndef MAFIA_LOGIC_GAME
#define MAFIA_LOGIC_GAME

#include "player.hpp"
#include "rulebook.hpp"

namespace maf {
   struct Game {
      // Signifies that a game was created with an unequal number of players and
      // cards.
      struct Players_to_cards_mismatch {
         std::size_t num_players;
         std::size_t num_cards;
      };

      // Signifies that no player could be found with the given ID.
      struct Player_not_found {
         Player::ID id;
      };

      // An exception signifying that the given player couldn't be kicked.
      struct Kick_failed {
         enum class Reason {
            game_ended,
            bad_timing,
            already_kicked
         };

         Kick_failed(const Player &player, Reason reason)
         : player{player}, reason{reason} { }

         rkt::ref<const Player> player;
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
            voter_is_not_present,
            target_is_not_present,
            voter_is_target
         };

         Lynch_vote_failed(const Player &voter,
                           const Player *target,
                           Reason reason)
         : voter{voter}, target{target}, reason{reason} { }

         rkt::ref<const Player> voter;
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

      // An exception signifying that the next night cannot begin right now.
      struct Begin_night_failed {
         enum class Reason {
            game_ended,
            already_night,
            lynch_can_occur
         };

         Reason reason;
      };

      struct Choose_fake_role_failed {
         enum class Reason {
            game_ended,
            bad_timing,
            player_is_not_faker,
            already_chosen
         };

         Choose_fake_role_failed(const Player &player, const Role &fake_role,
                                 Reason reason)
         : player{player}, fake_role{fake_role}, reason{reason} { }

         rkt::ref<const Player> player;
         rkt::ref<const Role> fake_role;
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

      struct Kill_failed {
         enum class Reason {
            game_ended,
            caster_cannot_kill,
            target_is_not_present,
            caster_is_target
         };

         Kill_failed(const Player &caster, const Player &target, Reason reason)
         : caster{caster}, target{target}, reason{reason} { }

         rkt::ref<const Player> caster;
         rkt::ref<const Player> target;
         Reason reason;
      };

      // An exception signifying that the caster cannot heal the target.
      struct Heal_failed {
         enum class Reason {
            game_ended,
            caster_cannot_heal,
            target_is_not_present,
            caster_is_target
         };

         Heal_failed(const Player &caster, const Player &target, Reason reason)
         : caster{caster}, target{target}, reason{reason} { }

         rkt::ref<const Player> caster;
         rkt::ref<const Player> target;
         Reason reason;
      };

      struct Investigate_failed {
         enum class Reason {
            game_ended,
            caster_cannot_investigate,
            target_is_not_present,
            caster_is_target
         };

         Investigate_failed(const Player &caster, const Player &target,
                            Reason reason)
         : caster{caster}, target{target}, reason{reason} { }

         rkt::ref<const Player> caster;
         rkt::ref<const Player> target;
         Reason reason;
      };

      struct Peddle_failed {
         enum class Reason {
            game_ended,
            caster_cannot_peddle,
            target_is_not_present,
            caster_is_target
         };

         Peddle_failed(const Player &caster, const Player &target,
                       Reason reason)
         : caster{caster}, target{target}, reason{reason} { }

         rkt::ref<const Player> caster;
         rkt::ref<const Player> target;
         Reason reason;
      };

      /* fix-me: make specific iterations of this exception for each ability. */
      // An exception signifying that an ability cannot be skipped.
      struct Skip_failed { };


      // The result of an investigation during the night.
      struct Investigation {
         Investigation(const Player &caster, const Player &target,
                       bool target_is_suspicious)
         : caster{caster}, target{target},
         target_is_suspicious{target_is_suspicious} { }

         rkt::ref<const Player> caster;
         rkt::ref<const Player> target;
         bool target_is_suspicious;
      };


      // Start a new game with the given parameters, creating a set of players
      // and assigning each player an initial role.
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

      // Whether the time is currently day.
      bool is_day() const;

      // Forcibly remove the given player from the game.
      // A player removed in this way cannot win.
      void kick_player(Player::ID id);

      // The player that would be lynched if the lynch votes were to be
      // processed now, or nullptr if no player would be lynched.
      const Player * next_lynch_victim() const;
      // Whether a lynch can still take place today.
      bool lynch_can_occur() const;
      // Casts a lynch vote by the voter against the target.
      void cast_lynch_vote(Player::ID voter_id, Player::ID target_id);
      // Clears the voter's lynch vote.
      void clear_lynch_vote(Player::ID voter_id);
      // Submits the lynch votes, and lynches the next victim.
      // Returns the player lynched, or nullptr if nobody was lynched.
      const Player * process_lynch_votes();

      // Stages a duel initiated by the caster against the target.
      void stage_duel(Player::ID caster_id, Player::ID target_id);

      // Whether it is currently night.
      bool is_night() const;
      // Proceeds to the next night.
      void begin_night();

      // Choose the given role as a fake role for the given player.
      void choose_fake_role(Player::ID player_id, Role::ID fake_role_id);

      // Whether or not the mafia can cast their nightly kill right now.
      bool mafia_can_use_kill() const;
      // Chooses a caster and target for the mafia's nightly kill, or skips
      // it altogether.
      void cast_mafia_kill(Player::ID caster_id, Player::ID target_id);
      void skip_mafia_kill();

      // Makes the caster kill the target, or skip using the kill.
      void cast_kill(Player::ID caster_id, Player::ID target_id);
      void skip_kill(Player::ID caster_id);

      // Makes the caster heal the target, or skip using the heal.
      void cast_heal(Player::ID caster_id, Player::ID target_id);
      void skip_heal(Player::ID caster_id);

      // Makes the caster investigate the target, or skip performing an
      // investigation.
      void cast_investigate(Player::ID caster_id, Player::ID target_id);
      void skip_investigate(Player::ID caster_id);

      // Makes the caster peddle drugs to the target, or skip peddling any
      // drugs.
      void cast_peddle(Player::ID caster_id, Player::ID target_id);
      void skip_peddle(Player::ID caster_id);

      // The results of the previous night's investigations.
      std::vector<Investigation> investigations() const;

      // Whether or not the game has ended.
      bool has_ended() const;

   private:
      std::vector<Player> _players{};
      Rulebook _rulebook;

      bool _has_ended{false};

      Date _date{0};
      Time _time{Time::day};

      bool _lynch_can_occur{false};

      bool _mafia_can_use_kill{false};
      Player *_mafia_kill_caster{nullptr};
      Player *_mafia_kill_target{nullptr};

      std::vector<std::pair<Player *, Player *>> _pending_kills{};
      std::vector<std::pair<Player *, Player *>> _pending_heals{};
      std::vector<std::pair<Player *, Player *>> _pending_investigations{};
      std::vector<std::pair<Player *, Player *>> _pending_peddles{};

      std::vector<Player *> _pending_haunters{};
      std::vector<std::tuple<Player *, Player *, bool>> _investigations{};

      // Gets the player with the given ID.
      // Throws an exception if no such player could be found.
      Player & find_player(Player::ID id);

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
