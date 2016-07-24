#ifndef MAFIA_QUESTIONS_H
#define MAFIA_QUESTIONS_H

#include <ostream>

namespace maf {
   struct Console;

   
   struct Question {
      // Signifies that a question failed to process a set of commands.
      struct Bad_commands { };

      // Handles some commands, acting upon the console as appropriate.
      // Returns true if fully answered, and false otherwise. If false, then the
      // tagged string outputted by write may have changed.
      // Throws an exception if the commands couldn't be handled.
      virtual bool do_commands(const std::vector<std::string> &commands, Console &console) = 0;

      // Writes a tagged string containing the question to os.
      virtual void write(std::ostream &os) const = 0;
   };


//   struct Confirm_no_lynch_votes: Question {
//      bool do_commands(const std::vector<std::string> &commands, Console &console) override;
//
//      void write(std::ostream &os) const override;
//   };


//   struct Confirm_mafia_kill_skip: Question {
//      bool do_commands(const std::vector<std::string> &commands, Console &console) override;
//
//      void write(std::ostream &os) const override;
//   };


   struct Confirm_end_game: Question {
      bool do_commands(const std::vector<std::string> &commands, Console &console) override;
      
      void write(std::ostream &os) const override;
   };
}

#endif