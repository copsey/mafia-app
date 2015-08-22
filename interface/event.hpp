#ifndef MAFIA_EVENT_H
#define MAFIA_EVENT_H

#include <string>

#include "../logic/logic.hpp"

namespace mafia {
   struct Event {
      virtual ~Event() = default;

      // Try to advance the event to its next state.
      // Returns true if this is possible, and false otherwise.
      virtual bool advance() { return false; }

      // Get a string containing a summary of the event.
      // An empty string means that the event has no summary, and should be
      // ignored when generating a transcript.
      virtual std::string summary() const { return ""; }

      // Get a tagged string describing the event.
      // (see Styled_text for more information on tagged strings.)
      virtual std::string description() const = 0;
   };


   struct Player_given_initial_role: Event {
      Player_given_initial_role(const Player &player,
                                const Role &role,
                                const Joker *joker_ptr)
         : _player_ptr{&player}, _role_ptr{&role}, _joker_ptr{joker_ptr} {
      }

      std::string summary() const override;
      std::string description() const override;

      bool advance() override;

   private:
      const Player *_player_ptr;
      const Role *_role_ptr;
      const Joker *_joker_ptr;
      bool _is_private{false};
   };


   struct Time_changed: Event {
      Time_changed(Date d, Time t) : _date{d}, _time{t} { }

      std::string summary() const override;
      std::string description() const override;

   private:
      Date _date;
      Time _time;
   };


   struct Town_meeting: Event {
      Town_meeting(const Game &game) : _game{&game} { }

      std::string description() const override;

   private:
      const Game *_game;
   };


   struct Lynch_vote_cast: Event {
      Lynch_vote_cast(const Player &caster, const Player *target)
      : _caster{&caster}, _target{target} { }

      std::string summary() const override;
      std::string description() const override;

   private:
      const Player *_caster;
      const Player *_target;
   };


   struct Player_lynched: Event {
      Player_lynched(const Player *victim, const Role *victim_role)
      : _victim{victim}, _victim_role{victim_role} { }

      std::string summary() const override;
      std::string description() const override;

   private:
      const Player *_victim;
      const Role *_victim_role;
   };


   struct Mafia_meeting: Event {
      Mafia_meeting(std::vector<const Player *> mafiosi,
                    bool is_initial_meeting)
      : _mafiosi{mafiosi}, _initial{is_initial_meeting} { }

      std::string description() const override;

   private:
      std::vector<const Player *> _mafiosi;
      bool _initial;
   };


   struct Boring_night: Event {
      std::string description() const override;
   };


   struct Game_ended: Event {
      Game_ended(const Game &game) : _game_ptr{&game} { }

      /* fix-me: std::string summary() const override; */
      std::string description() const override;

   private:
      const Game *_game_ptr;
   };
}

#endif
