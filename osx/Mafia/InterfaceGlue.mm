//
//  InterfaceGlue.m
//  Mafia
//
//  Created by Jack Copsey on 02/05/2015.
//  Copyright (c) 2015 Ponderous Programs. All rights reserved.
//

#import "InterfaceGlue.h"

@implementation InterfaceGlue

- (void)awakeFromNib {
   [self showOutput];
}

- (IBAction)readInput:(id)sender {
   std::string str{[self.input.stringValue cStringUsingEncoding:NSUTF8StringEncoding]};

   if (_console.input(str)) {
      /* fix-me: make more reliable, i.e. "preset i" doesn't work with this method. */
      if (str == "begin" || str == "preset") {
         [_delegate playMusic:MafiaPlaylistItem_Beginning];
      } else if (str == "end") {
         [_delegate playMusic:MafiaPlaylistItem_None];
      }

      [self showOutput];
   } else {
      [self showErrorMessage];
   }
}

- (void)clearInput {
   self.input.stringValue = @"";
}

- (void)showOutput {
   NSMutableAttributedString *attributedString = [[NSMutableAttributedString alloc] init];

   for (const mafia::Styled_string &styled_str : _console.output()) {
      NSString *string = [NSString stringWithUTF8String:styled_str.string.c_str()];

      switch (styled_str.style) {
         case mafia::Styled_string::Style::game_title:
         case mafia::Styled_string::Style::help_title:
            _output.window.title = string;
            break;

         case mafia::Styled_string::Style::game: {
            [attributedString appendAttributedString:[[NSAttributedString alloc] initWithString:string
                                                                                     attributes:[InterfaceGlue gameStyleAttributes]]];
            break;
         }

         case mafia::Styled_string::Style::game_italic: {
            [attributedString appendAttributedString:[[NSAttributedString alloc] initWithString:string
                                                                                     attributes:[InterfaceGlue gameItalicStyleAttributes]]];
            break;
         }

         case mafia::Styled_string::Style::help: {
            [attributedString appendAttributedString:[[NSAttributedString alloc] initWithString:string
                                                                                     attributes:[InterfaceGlue helpStyleAttributes]]];
            break;
         }

         case mafia::Styled_string::Style::command: {
            [attributedString appendAttributedString:[[NSAttributedString alloc] initWithString:string
                                                                                     attributes:[InterfaceGlue commandStyleAttributes]]];
            break;
         }
      }
   }

   self.output.textStorage.attributedString = attributedString;

   // Play some music.
   if (_console.has_game()) {
      const mafia::Event *event = &_console.game_log().current_event();

      if (auto e = dynamic_cast<const mafia::Time_changed *>(event)) {
         if (e->time == mafia::Time::day) {
            [_delegate playMusic:MafiaPlaylistItem_Daytime];
         } else {
            switch (e->date % 2) {
               case 0:
                  [_delegate playMusic:MafiaPlaylistItem_Nighttime1];
                  break;

               case 1:
                  [_delegate playMusic:MafiaPlaylistItem_Nighttime2];
                  break;
            }
         }
      }
      else if (auto e = dynamic_cast<const mafia::Lynch_result *>(event)) {
         if (e->victim && e->victim_role->is_troll) {
            [_delegate playMusic:MafiaPlaylistItem_TrollLynch];
         }
      }
      else if (auto e = dynamic_cast<const mafia::Game_ended *>(event)) {
         // Hide compiler warning that e is unused
         e = nullptr;

         [_delegate playMusic:MafiaPlaylistItem_GameEnded];
      }
   }
}

- (void)showErrorMessage {
   NSAlert *alert = [[NSAlert alloc] init];
   NSMutableString *informativeText = [NSMutableString string];

   for (const mafia::Styled_string::Styled_string &styled_str : _console.error_message()) {
      NSString *string = [NSString stringWithUTF8String:styled_str.string.c_str()];

      switch (styled_str.style) {
         case mafia::Styled_string::Style::help_title:
            alert.messageText = string;
            break;

         case mafia::Styled_string::Style::command:
            [informativeText appendString:@"\""];
            [informativeText appendString:string];
            [informativeText appendString:@"\""];
            break;

         default:
            [informativeText appendString:string];
            break;
      }
   }

   alert.informativeText = informativeText;
   [alert runModal];
}

+ (NSDictionary *)gameStyleAttributes {
   return @{NSFontAttributeName: [NSFont fontWithName:@"Helvetica" size:14]};
}

+ (NSDictionary *)gameItalicStyleAttributes {
   NSMutableParagraphStyle *paragraphStyle = [[NSMutableParagraphStyle alloc] init];
   paragraphStyle.alignment = NSCenterTextAlignment;
   return @{NSFontAttributeName: [NSFont fontWithName:@"Times New Roman Italic" size:14],
            NSParagraphStyleAttributeName: paragraphStyle};
}

+ (NSDictionary *)helpStyleAttributes {
   return @{NSFontAttributeName: [NSFont fontWithName:@"Helvetica" size:13]};
}

+ (NSDictionary *)commandStyleAttributes {
   return @{NSFontAttributeName: [NSFont fontWithName:@"Andale Mono" size:13],
            NSForegroundColorAttributeName: [NSColor colorWithWhite:0.2 alpha:1],
            NSBackgroundColorAttributeName: [NSColor colorWithWhite:0.93 alpha:1]};
}

@end
