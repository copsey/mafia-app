#ifndef MAFIA_INTERFACE_GAME_SCREENS
#define MAFIA_INTERFACE_GAME_SCREENS

#include <vector>

#include "../riketi/ref.hpp"

#include "../logic/logic.hpp"

#include "screens.hpp"

namespace maf {
   class Game_log;


   struct Game_Screen: Base_Screen {
      using Base_Screen::Base_Screen;

      bool handle_commands(const std::vector<std::string> & commands) override;

   protected:
      // Get a non-const reference to the `Game_log` of the owning console.
      Game_log & game_log() const;
   };


   namespace screen {
      struct Player_Given_Initial_Role: Game_Screen {
         Player_Given_Initial_Role(Console & con,
                                   const Player & player,
                                   const Role & role,
                                   const Wildcard * wildcard_ptr)
            : Game_Screen{con}, _p_ref{player}, _r_ref{role}, _w_ptr{wildcard_ptr}
         { }

         bool handle_commands(const std::vector<std::string> & commands) override;
         void write(std::ostream & os) const override;
         Help_Screen * get_help_screen() const override;

      private:
         rkt::ref<const Player> _p_ref;
         rkt::ref<const Role> _r_ref;
         const Wildcard *_w_ptr;
         int _page{0};
      };


      struct Time_Changed: Game_Screen {
         Time_Changed(Console & con, Date d, Time t)
            : Game_Screen{con}, date{d}, time{t}
         { }

         bool handle_commands(const std::vector<std::string> & commands) override;
         void write(std::ostream & os) const override;
         Help_Screen * get_help_screen() const override;

      private:
         Date date;
         Time time;
      };


      struct Obituary: Game_Screen {
         Obituary(Console & con, std::vector<rkt::ref<const Player>> deaths)
            : Game_Screen{con}, _deaths{deaths}
         { }

         bool handle_commands(const std::vector<std::string> & commands) override;
         void write(std::ostream & os) const override;
         Help_Screen * get_help_screen() const override;

      private:
         std::vector<rkt::ref<const Player>> _deaths;
         std::ptrdiff_t _deaths_index{-1};
      };


      struct Town_Meeting: Game_Screen {
         Town_Meeting(Console & con,
                      std::vector<rkt::ref<const Player>> players,
                      Date date,
                      bool lynch_can_occur,
                      const Player * next_lynch_victim,
                      const Player * recent_lynch_vote_caster,
                      const Player * recent_lynch_vote_target)
            : Game_Screen{con}, _players{players}, _date{date}, _lynch_can_occur{lynch_can_occur},
              _next_lynch_victim{next_lynch_victim}, _recent_vote_caster{recent_lynch_vote_caster},
              _recent_vote_target{recent_lynch_vote_target}
         { }

         bool handle_commands(const std::vector<std::string> & commands) override;
         void write(std::ostream & os) const override;
         Help_Screen * get_help_screen() const override;

      private:
         std::vector<rkt::ref<const Player>> _players;
         Date _date;
         bool _lynch_can_occur;
         const Player *_next_lynch_victim;
         const Player *_recent_vote_caster;
         const Player *_recent_vote_target;
      };


      struct Player_Kicked: Game_Screen {
         Player_Kicked(Console & con, const Player & player, const Role & player_role)
            : Game_Screen{con}, _pl_ref{player}, _pl_role_ref{player_role}
         { }

         bool handle_commands(const std::vector<std::string> & commands) override;
         void write(std::ostream & os) const override;
         Help_Screen * get_help_screen() const override;

      private:
         rkt::ref<const Player> _pl_ref;
         rkt::ref<const Role> _pl_role_ref;
      };


      struct Lynch_Result: Game_Screen {
         // Tag indicating that nobody was lynched.
         struct no_lynch_t {};
         static constexpr auto no_lynch = no_lynch_t{};

         Lynch_Result(Console & con, no_lynch_t)
            : Game_Screen{con}
         { }

         Lynch_Result(Console & con, const Player & victim, const Role & victim_role)
            : Game_Screen{con}, _victim_ptr{&victim}, _victim_role_ptr{&victim_role}
         { }

         bool handle_commands(const std::vector<std::string> & commands) override;
         void write(std::ostream & os) const override;
         Help_Screen * get_help_screen() const override;

      private:
         const Player * _victim_ptr{nullptr};
         const Role * _victim_role_ptr{nullptr};
      };


      struct Investigation_Result: Game_Screen {
         Investigation_Result(Console & con, Investigation investigation)
            : Game_Screen{con}, _inv{investigation}
         { }

         bool handle_commands(const std::vector<std::string> & commands) override;
         void write(std::ostream & os) const override;
         Help_Screen * get_help_screen() const override;

      private:
         Investigation _inv;
         int _page{0};
      };


      struct Game_Ended: Game_Screen {
         using Game_Screen::Game_Screen;

         bool handle_commands(const std::vector<std::string> & commands) override;
         void write(std::ostream & os) const override;
         Help_Screen * get_help_screen() const override;
      };
   }
}

#endif
