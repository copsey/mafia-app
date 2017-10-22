#ifndef MAFIA_INTERFACE_ERROR
#define MAFIA_INTERFACE_ERROR

#include <istream>
#include <string>

#include "styled_string.hpp"

namespace maf {
   namespace error {
      struct bad_commands {};
      struct cards_mismatch {};
      struct duplicate_player {};
      struct invalid_name {};
      struct invalid_preset {};
      struct unselected_card {};

      // An exception containing an error message in the form of styled text.
      struct styled_exception {
         // Create a styled exception using the tagged string contained in `is`.
         styled_exception(std::istream& is)
            : _msg(styled_text_from(is))
         { }

         // Create a styled exception using the raw tagged string in `raw_msg`.
         styled_exception(const std::string& raw_msg)
            : _msg(styled_text_from(raw_msg))
         { }

         // Get the styled text describing this exception.
         Styled_text styled_message() const {
            return _msg;
         }

      private:
         Styled_text _msg;
      };

      // An exception signifying that not all commands inputted by the user
      // could be resolved.
      //
      // This could be for a variety of reasons; further information is
      // contained in the error message.
      struct unresolved_input: styled_exception {
         using styled_exception::styled_exception;
      };
   }
}

#endif
