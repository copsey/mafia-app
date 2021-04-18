//
//  InterfaceGlue.m
//  Mafia
//
//  Created by Jack Copsey on 02/05/2015.
//  Copyright (c) 2015-2021. All rights reserved.
//

#import "InterfaceGlue.h"

@implementation InterfaceGlue

- (void)awakeFromNib {
	[self showOutput];
}

- (IBAction)readInput:(id)sender {
	std::string_view str = {self.input.stringValue.UTF8String};
	
	if (_console.input(str)) {
		/* FIXME: make more reliable, e.g., "preset i" doesn't get caught by this method */
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
	bool clearWhitespaceFromFront = false;
	
	for (auto & styled_str : _console.output()) {
		NSMutableDictionary *attributes = [NSMutableDictionary dictionaryWithDictionary:[InterfaceGlue defaultAttributes]];
		NSString *string = [NSString stringWithUTF8String:styled_str.string.c_str()];
		
		// Strip whitespace from front if last style was "title".
		if (clearWhitespaceFromFront) {
			NSCharacterSet *characterSet = [NSCharacterSet whitespaceAndNewlineCharacterSet];
			NSRange rangeOfLastWantedCharacter = [string rangeOfCharacterFromSet:[characterSet invertedSet]];
			if (rangeOfLastWantedCharacter.location == NSNotFound) {
				string = @"";
			} else {
				string = [string substringFromIndex:rangeOfLastWantedCharacter.location];
			}
			clearWhitespaceFromFront = false;
		}
		
		auto style = styled_str.style;
		
		if ((style & maf::StyledString::title_mask).any()) {
			_output.window.title = string;
			clearWhitespaceFromFront = true;
			continue;
		}
		
		if ((style & maf::StyledString::italic_mask).any()) {
			[attributes addEntriesFromDictionary:[InterfaceGlue italicAttributes]];
		}

		if ((style & maf::StyledString::bold_mask).any()) {
			[attributes addEntriesFromDictionary:[InterfaceGlue boldAttributes]];
		}
		
		if ((style & maf::StyledString::help_text_mask).any()) {
			[attributes addEntriesFromDictionary:[InterfaceGlue helpTextAttributes]];
		}
		
		if ((style & maf::StyledString::monospace_mask).any()) {
			[attributes addEntriesFromDictionary:[InterfaceGlue monospaceAttributes]];
		}
		
		NSAttributedString *newString = [[NSAttributedString alloc] initWithString:string attributes:attributes];
		[attributedString appendAttributedString:newString];
	}
	
	self.output.textStorage.attributedString = attributedString;
	
	// Play some music.
	if (_console.has_game()) {
		const maf::Event *event = &_console.game_log().current_event();
		
		if (auto e = dynamic_cast<const maf::Time_changed *>(event); e) {
			if (e->time == maf::Time::day) {
				[_delegate playMusic:MafiaPlaylistItem_Daytime];
			} else {
				switch (arc4random_uniform(4)) {
					case 0:
						[_delegate playMusic:MafiaPlaylistItem_Nighttime1];
						break;
						
					case 1:
						[_delegate playMusic:MafiaPlaylistItem_Nighttime2];
						break;
						
					case 2:
						[_delegate playMusic:MafiaPlaylistItem_Nighttime3];
						break;
				}
			}
		}
		else if (auto e = dynamic_cast<const maf::Lynch_result *>(event); e) {
			if (e->victim && e->victim_role->is_troll()) {
				[_delegate playMusic:MafiaPlaylistItem_TrollLynch];
			}
		}
		else if (auto e = dynamic_cast<const maf::Duel_result *>(event); e) {
			[_delegate playMusic:MafiaPlaylistItem_Duel];
		}
		else if (auto e = dynamic_cast<const maf::Game_ended *>(event); e) {
			[_delegate playMusic:MafiaPlaylistItem_GameEnded];
		}
	}
}

- (void)showErrorMessage {
	NSAlert *alert = [[NSAlert alloc] init];
	NSMutableString *informativeText = [NSMutableString string];
	bool clearWhitespaceFromFront = false;
	
	for (auto & styled_str : _console.error_message()) {
		NSString *string = [NSString stringWithUTF8String:styled_str.string.c_str()];
		auto style = styled_str.style;
		
		// Strip whitespace from front if last style was "title".
		if (clearWhitespaceFromFront) {
			NSCharacterSet *characterSet = [NSCharacterSet whitespaceAndNewlineCharacterSet];
			NSRange rangeOfLastWantedCharacter = [string rangeOfCharacterFromSet:[characterSet invertedSet]];
			if (rangeOfLastWantedCharacter.location == NSNotFound) {
				string = @"";
			} else {
				string = [string substringFromIndex:rangeOfLastWantedCharacter.location];
			}
			clearWhitespaceFromFront = false;
		}
		
		if ((style & maf::StyledString::title_mask).any()) {
			alert.messageText = string;
			clearWhitespaceFromFront = true;
		} else if ((style & maf::StyledString::monospace_mask).any()) {
			[informativeText appendString:@"\""];
			[informativeText appendString:string];
			[informativeText appendString:@"\""];
		} else {
			[informativeText appendString:string];
		}
	}
	
	alert.informativeText = informativeText;
	[alert runModal];
}

+ (NSDictionary *)defaultAttributes {
	return @{NSFontAttributeName: [NSFont fontWithName:@"Helvetica" size:14]};
}

+ (NSDictionary *)italicAttributes {
	NSMutableParagraphStyle *paragraphStyle = [[NSMutableParagraphStyle alloc] init];
	paragraphStyle.alignment = NSTextAlignmentCenter;
	return @{NSFontAttributeName: [NSFont fontWithName:@"Times New Roman Italic" size:14],
			 NSParagraphStyleAttributeName: paragraphStyle};
}

+ (NSDictionary *)boldAttributes {
	return @{NSFontAttributeName: [NSFont fontWithName:@"Helvetica Bold" size:14]};
}

+ (NSDictionary *)helpTextAttributes {
	return @{NSForegroundColorAttributeName: [NSColor colorWithRed:0 green:0.42 blue:0.65 alpha:1]};
}

+ (NSDictionary *)monospaceAttributes {
	return @{NSFontAttributeName: [NSFont fontWithName:@"Andale Mono" size:14],
			 NSForegroundColorAttributeName: [NSColor colorWithWhite:0.2 alpha:1],
			 NSBackgroundColorAttributeName: [NSColor colorWithWhite:0.93 alpha:1]};
}

@end
