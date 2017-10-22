#ifndef MAFIA_STYLED_STRING_H
#define MAFIA_STYLED_STRING_H

#include <istream>
#include <string>
#include <vector>

namespace maf {
   // Escape all of the style control characters '^' in `str`.
   //
   // Useful when converting a string entered by the user into styled text.
   void escape_style_codes(std::string& str);

   // Create a copy of `str` with all of the style control characters '^'
   // escaped.
   //
   // Useful when converting a string entered by the user into styled text.
   std::string copy_with_escaped_style_codes(const std::string& str);

   // A string coupled with a suggested style.
   // A style is intended to entail such formatting properties as typeface,
   // font size, colour, etc.
   struct Styled_string {
      enum class Style {
         game,    // Default style during game.
         help,    // Game set-up and general help.
         italic,  // Italic modifier on current style. Used for descriptive text.
         command, // Commands that can be inputted into the console.
         title    // Title of section. A maximum of one title can occur
                  // per message, as the first item.
      };

      std::string string{};
      Style style{Style::game};

      Styled_string() = default;

      Styled_string(std::string string, Style style)
         : string{string}, style{style}
      { }
   };

   // A vector of styled strings, used to form a block of text.
   using Styled_text = std::vector<Styled_string>;

   // Often, styled text is created from what is called a tagged string: this is
   // simply a std::string containing formatting codes of the form ^x.
   // The following tags are possible:
   //    ^g = game
   //    ^h = help
   //    ^i = italic
   //    ^c = command
   //    ^T = title
   // ^/ closes the current tag, so that what follows uses the previous tag.
   // (Note that the maximum-supported tag depth is 9, including the default.)
   // ^^ prints a single '^' character. The appearance of any other two-
   // character substring of the form ^x results in an exception.
   // Note that the default string style is Style::game, and hence game-styled
   // strings need not be prepended with ^g.

   // Convert a tagged string into some styled text.
   Styled_text styled_text_from(const std::string &tagged_s);

   // Convert the tagged string obtained from `is` into some styled text.
   // is.tellg() is assumed to be 0.
   Styled_text styled_text_from(std::istream &is);
}

#endif
