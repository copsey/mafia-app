#ifndef MAFIA_INTERFACE_GAME_SCREENS
#define MAFIA_INTERFACE_GAME_SCREENS

#include "screens.hpp"

namespace maf {
   struct Game_Screen: Base_Screen {
      using Base_Screen::Base_Screen;

      bool handle_commands(const std::vector<std::string> & commands) override;
   };
}

#endif
