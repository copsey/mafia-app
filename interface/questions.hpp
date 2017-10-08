#ifndef MAFIA_INTERFACE_QUESTIONS
#define MAFIA_INTERFACE_QUESTIONS

#include "../riketi/ref.hpp"

#include <ostream>

namespace maf {
   struct Console;

   
   struct Question {
      // Signifies that a question failed to process a set of commands.
      struct Bad_commands { };

      // Handles the given commands, taking action as appropriate.
      //
      // Returns true if the question has been fully answered, and false otherwise.
      // If false, then the tagged string outputted by write may have changed.
      //
      // Throws an exception if the commands couldn't be handled.
      virtual bool do_commands(const std::vector<std::string> & commands) = 0;

      // Writes a tagged string containing the question to os.
      virtual void write(std::ostream &os) const = 0;
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
         : _console_ref(console)
      { }

      bool do_commands(const std::vector<std::string> & commands) override;
      
      void write(std::ostream &os) const override;

   private:
      rkt::ref<Console> _console_ref;
   };
}

#endif
