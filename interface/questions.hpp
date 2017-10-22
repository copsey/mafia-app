#ifndef MAFIA_INTERFACE_QUESTIONS
#define MAFIA_INTERFACE_QUESTIONS

#include "../riketi/ref.hpp"

#include <ostream>

#include "screens.hpp"

namespace maf {
   struct Console;

   
   // FIXME: Place all Questions into separate 'question' child namespace
   // of 'maf'.
    
   struct Question: Base_Screen {
      struct Bad_commands {};

      using Base_Screen::Base_Screen;

      Help_Screen * get_help_screen() const override;
   };


//   struct Confirm_no_lynch_votes: Question {
//      bool do_commands(const std::vector<std::string> &commands) override;
//
//      void write(std::ostream &os) const override;
//   };


//   struct Confirm_mafia_kill_skip: Question {
//      bool do_commands(const std::vector<std::string> &commands) override;
//
//      void write(std::ostream &os) const override;
//   };


   struct Confirm_end_game: Question {
      Confirm_end_game(Console & console)
         : Question(console)
      { }

      bool handle_commands(const std::vector<std::string> & commands) override;
      void write(std::ostream &os) const override;
   };
}

#endif
