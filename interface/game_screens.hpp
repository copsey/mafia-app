#ifndef MAFIA_INTERFACE_GAME_SCREENS
#define MAFIA_INTERFACE_GAME_SCREENS

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
   }
}

#endif
