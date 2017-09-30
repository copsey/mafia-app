#ifndef MAFIA_LOGIC_GAME
#define MAFIA_LOGIC_GAME

#include "../riketi/ref.hpp"

#include "player.hpp"
#include "rulebook.hpp"

namespace maf {
   /// The result of an investigation that `caster` performed on `target`.
   ///
   /// Note that only references to `caster` and `target` are stored, and so
   /// the lifetime of an investigation result should not exceed the lifetime
   /// of the players concerned.
   struct Investigation {
      Investigation(const Player & caster, const Player & target, Date date, bool result)
       : _caster_ref{caster}, _target_ref{target}, _date{date}, _result{result} { }

      /// The player that performed the investigation.
      const Player& caster() const {
         return *_caster_ref;
      }

      /// The target of the investigation.
      const Player& target() const {
         return *_target_ref;
      }

      /// The date on which the investigation occurred.
      Date date() const {
         return _date;
      }

      /// The result of the investigation.
      ///
      /// @returns `true` if the target appeared as suspicious,
      /// `false` otherwise.
      bool result() const {
         return _result;
      }

   private:
      rkt::ref<const Player> _caster_ref;
      rkt::ref<const Player> _target_ref;
      Date _date;
      bool _result;
   };


   struct Game {
      // Start a new game with the given parameters, creating a set of players
      // and assigning each player an initial role.
      // Note that this could lead to the game immediately ending.
      Game(const std::vector<Role::ID> &role_ids,
           const std::vector<Wildcard::ID> &wildcard_ids,
           const Rulebook &rulebook = Rulebook{});
      
      // The rulebook being used to run the game.
      const Rulebook & rulebook() const;

      // The participating players, both present and not present.
      const std::vector<Player> & players() const;
      // A vector containing every player remaining.
      std::vector<rkt::ref<const Player>> remaining_players() const;
      // A vector containing every player remaining with the given alignment.
      std::vector<rkt::ref<const Player>> remaining_players(Alignment alignment) const;
      // The number of players remaining.
      std::size_t num_players_left() const;
      // The number of players remaining with the given alignment.
      std::size_t num_players_left(Alignment alignment) const;

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

      /// The results of all of the investigations which have occurred so
      /// far in the course of the game.
      ///
      /// The results are stored in chronological order, with the most
      /// recent investigations at the end of the vector.
      const std::vector<Investigation> & investigations() const {
         return _investigations;
      }

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
      std::vector<Investigation> _investigations{};

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
