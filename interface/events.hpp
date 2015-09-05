#ifndef MAFIA_EVENT_H
#define MAFIA_EVENT_H

#include <ostream>
#include <string>

#include "../logic/logic.hpp"

namespace mafia {
   struct Game_log;

   struct Event {
      // Signifies that an event failed to process a set of commands.
      struct Bad_commands { };

      virtual ~Event() = default;

      // Handles the given commands, acting on the given game log as required.
      // Throws an exception if the commands couldn't be handled.
      virtual void do_commands(const std::vector<std::string> &commands, Game_log &game_log) = 0;

      // Writes a tagged string detailing the event to os.
      virtual void write_full(std::ostream &os) const = 0;
      // Writes a summary of the event to os.
      // (this is a "normal" string, and not a tagged string.)
      // By default, nothing is written at all.
      virtual void write_summary(std::ostream &os) const;
      // Writes a tagged string to os containing help with the event.
      // By default, complains that no help has been written for the event.
      virtual void write_help(std::ostream &os) const;
   };


   struct Player_given_initial_role: Event {
      Player_given_initial_role(const Player &player,
                                const Role &role,
                                const Wildcard *wildcard)
      : _p{&player}, _r{&role}, _w{wildcard} { }

      void do_commands(const std::vector<std::string> &commands, Game_log &game_log) override;

      void write_full(std::ostream &os) const override;
      void write_summary(std::ostream &os) const override;

   private:
      const Player *_p;
      const Role *_r;
      const Wildcard *_w;
      bool _is_private{false};
   };


   struct Time_changed: Event {
      Time_changed(Date d, Time t): date{d}, time{t} { }

      Date date;
      Time time;

      void do_commands(const std::vector<std::string> &commands, Game_log &game_log) override;

      void write_full(std::ostream &os) const override;
      void write_summary(std::ostream &os) const override;
   };


   struct Obituary: Event {
      Obituary(std::vector<rkt::ref<const Player>> deaths)
      : _deaths{deaths} { }

      void do_commands(const std::vector<std::string> &commands, Game_log &game_log) override;

      void write_full(std::ostream &os) const override;
      void write_summary(std::ostream &os) const override;

   private:
      std::vector<rkt::ref<const Player>> _deaths;
      std::ptrdiff_t _deaths_index{-1};
   };


   struct Town_meeting: Event {
      Town_meeting(std::vector<rkt::ref<const Player>> players,
                   Date date,
                   bool lynch_can_occur,
                   const Player *next_lynch_victim,
                   const Player *recent_lynch_vote_caster,
                   const Player *recent_lynch_vote_target)
      : _players{players}, _date{date}, _lynch_can_occur{lynch_can_occur},
      _next_lynch_victim{next_lynch_victim},
      _recent_vote_caster{recent_lynch_vote_caster},
      _recent_vote_target{recent_lynch_vote_target} { }

      void do_commands(const std::vector<std::string> &commands, Game_log &game_log) override;

      void write_full(std::ostream &os) const override;
      void write_summary(std::ostream &os) const override;

   private:
      std::vector<rkt::ref<const Player>> _players;
      Date _date;
      bool _lynch_can_occur;
      const Player *_next_lynch_victim;
      const Player *_recent_vote_caster;
      const Player *_recent_vote_target;
   };


   struct Player_kicked: Event {
      Player_kicked(const Player &player, const Role &player_role)
      : player{player}, player_role{player_role} { }

      void do_commands(const std::vector<std::string> &commands, Game_log &game_log) override;

      void write_full(std::ostream &os) const override;
      void write_summary(std::ostream &os) const override;

      rkt::ref<const Player> player;
      rkt::ref<const Role> player_role;
   };


   struct Lynch_result: Event {
      Lynch_result(const Player *victim, const Role *victim_role)
      : victim{victim}, victim_role{victim_role} { }

      // The player who was lynched, or nullptr if nobody was lynched.
      const Player *victim;
      // The role of the player who was lynched, or nullptr if nobody was
      // lynched / the role could not be determined.
      const Role *victim_role;

      void do_commands(const std::vector<std::string> &commands, Game_log &game_log) override;

      void write_full(std::ostream &os) const override;
      void write_summary(std::ostream &os) const override;
   };


   struct Duel_result: Event {
      Duel_result(const Player &caster, const Player &target): caster{caster}, target{target} { }

      void do_commands(const std::vector<std::string> &commands, Game_log &game_log) override;

      void write_full(std::ostream &os) const override;
      void write_summary(std::ostream &os) const override;

      rkt::ref<const Player> caster;
      rkt::ref<const Player> target;
   };


   struct Choose_fake_role: Event {
      Choose_fake_role(const Player &player)
      : _player{&player} { }

      void do_commands(const std::vector<std::string> &commands, Game_log &game_log) override;

      void write_full(std::ostream &os) const override;
      void write_summary(std::ostream &os) const override;

   private:
      const Player *_player;
      const Role *_fake_role{nullptr};
      bool _go_to_sleep{false};
   };


   struct Mafia_meeting: Event {
      Mafia_meeting(std::vector<rkt::ref<const Player>> mafiosi,
                    bool is_initial_meeting)
      : _mafiosi{mafiosi}, _initial{is_initial_meeting} { }

      void do_commands(const std::vector<std::string> &commands, Game_log &game_log) override;

      void write_full(std::ostream &os) const override;

   private:
      std::vector<rkt::ref<const Player>> _mafiosi;
      bool _initial;
      bool _go_to_sleep{false};
   };


   struct Kill_use: Event {
      Kill_use(const Player &caster): _caster{&caster} { }

      void do_commands(const std::vector<std::string> &commands, Game_log &game_log) override;

      void write_full(std::ostream &os) const override;

   private:
      const Player *_caster;
      bool _go_to_sleep{false};
   };


   struct Heal_use: Event {
      Heal_use(const Player &caster): _caster{&caster} { }

      void do_commands(const std::vector<std::string> &commands, Game_log &game_log) override;

      void write_full(std::ostream &os) const override;

   private:
      const Player *_caster;
      bool _go_to_sleep{false};
   };


   struct Investigate_use: Event {
      Investigate_use(const Player &caster): _caster{&caster} { }

      void do_commands(const std::vector<std::string> &commands, Game_log &game_log) override;

      void write_full(std::ostream &os) const override;

   private:
      const Player *_caster;
      bool _go_to_sleep{false};
   };


   struct Peddle_use: Event {
      Peddle_use(const Player &caster): _caster{&caster} { }

      void do_commands(const std::vector<std::string> &commands, Game_log &game_log) override;

      void write_full(std::ostream &os) const override;

   private:
      const Player *_caster;
      bool _go_to_sleep{false};
   };


   struct Boring_night: Event {
      Boring_night() { }

      void do_commands(const std::vector<std::string> &commands, Game_log &game_log) override;

      void write_full(std::ostream &os) const override;
   };


   struct Investigation_result: Event {
      Investigation_result(Game::Investigation investigation)
      : investigation{investigation} { }

      Game::Investigation investigation;

      void do_commands(const std::vector<std::string> &commands, Game_log &game_log) override;

      void write_full(std::ostream &os) const override;
      void write_summary(std::ostream &os) const override;

   private:
      bool _go_to_sleep{false};
   };


   struct Game_ended: Event {
      Game_ended(const Game &game) : _game_ptr{&game} { }

      void do_commands(const std::vector<std::string> &commands, Game_log &game_log) override;

      void write_full(std::ostream &os) const override;
      void write_summary(std::ostream &os) const override;

   private:
      const Game *_game_ptr;
   };
}

#endif
