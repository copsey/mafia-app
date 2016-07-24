#ifndef MAFIA_STYLED_STRING_H
#define MAFIA_STYLED_STRING_H

#include <istream>
#include <string>
#include <vector>

namespace maf {
   // A string coupled with a suggested style.
   // A style is intended to entail such formatting properties as typeface,
   // font size, colour, etc.
   struct Styled_string {
      enum class Style {
         game,        // Default style during game.
         game_title,  // Title during game. A maximum of one title can occur
                      // per message, as the first item.
         game_italic, // Italic style during game. Used for descriptive text.
         help,        // Game set-up and general help.
         help_title,  // Title for game set-up and general help. A maximum of
                      // one title can occur per message, as the first item.
         command      // Commands that can be inputted into the console.
      };

      std::string string{};
      Style style{Style::game};

      Styled_string() = default;

      Styled_string(std::string string, Style style)
         : string{string}, style{style} { }
   };

   // A vector of styled strings, used to form a block of text.
   using Styled_text = std::vector<Styled_string>;

   // Often, styled text is created from what is called a tagged string: this is
   // simply a std::string containing formatting codes of the form ^x.
   // The following tags are possible:
   //    ^g = game,
   //    ^G = game_title,
   //    ^i = game_italic,
   //    ^h = help,
   //    ^H = help_title,
   //    ^c = command.
   // ^^ prints a single '^' character. The appearance of any other two-
   // character substring of the form ^x results in an exception.
   // Note that the default string style is Style::game, and hence game-styled
   // strings need not be prepended with ^g.

   // Convert a tagged string into some styled text.
   Styled_text styled_text_from(const std::string &tagged_s);

   // Convert the tagged string obtained from is into some styled text.
   // is.tellg() is assumed to be 0.
   Styled_text styled_text_from(std::istream &is);
}

#endif
