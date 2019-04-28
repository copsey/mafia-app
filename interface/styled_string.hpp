#ifndef MAFIA_STYLED_STRING_H
#define MAFIA_STYLED_STRING_H

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace maf {
   // Create a new string, identical to `str` but with each tag character
   // '^' replaced with '^^'.
   std::string escape_tags(std::string_view str);
   
   // Check if `param` is an allowed name for a styled-text parameter.
   // This is true if it matches the regex /^[a-zA-Z0-9_]+$/.
   bool is_param_name(std::string_view param);

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
   
   // A map from strings to strings, used as substitutions (e.g. "{exampleStr}")
   // when creating a block of text.
   using TextParams = std::map<std::string, std::string>;

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
   // 
   // This method also supports an optional dictionary of strings, called "params".
   // Each param uses the syntax "{exampleParam}", and is replaced by the value
   // from `params` when the styled text is generated.
   // 
   // For example, if `params == { {"word1", "TAGGED"}, {"word2", "STRING"} }`, then
   //     "This is my {word1} {word2}."
   // will be transformed to:
   //     "This is my TAGGED STRING."

   // Convert a tagged string into styled text, using `params` as a dictionary of
   // text replacements.
   Styled_text styled_text_from(std::string_view tagged_str, TextParams const& params);
}

#endif
