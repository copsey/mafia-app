#ifndef MAFIA_INTERFACE_SCREEN
#define MAFIA_INTERFACE_SCREEN

#include <ostream>
#include <string>
#include <vector>

#include "../riketi/ref.hpp"

#include "console.hpp"
#include "help_screens.hpp"

namespace maf {
   namespace screen {
      struct base_screen {
         // Make a new screen, with its owner set to `con`.
         base_screen(Console & con)
            : _console_ref{con}
         { }

         // Get the owning console for this screen.
         Console & console() { return *_console_ref; }
         const Console & console() const { return *_console_ref; }

         // Handle the commands, acting as appropriate on the owning console.
         //
         // Should be overriden by derived classes to provide custom commands,
         // calling `base_screen::handle_commands` to process the default set of
         // commands shared by all screens.
         //
         // @returns `true` if the commands were successfully processed, `false`
         // otherwise.
         //
         // @throws `error::missing_commands` if `commands` is an empty vector.
         virtual bool handle_commands(const std::vector<std::string> & commands);

         // Write a tagged string to `os` containing the content of this screen.
         virtual void write(std::ostream & os) const = 0;

         // Generate a help screen corresponding to this screen.
         //
         // @returns `nullptr` if this screen does not have a corresponding help
         // screen (e.g. if this screen is already a help screen).
         virtual Help_Screen * get_help_screen() const = 0;

      private:
         rkt::ref<Console> _console_ref;
      };
   }
}

#endif
