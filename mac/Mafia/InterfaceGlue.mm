//
//  InterfaceGlue.m
//  Mafia
//
//  Created by Jack Copsey on 02/05/2015.
//  Copyright (c) 2015-2021. All rights reserved.
//

#import "InterfaceGlue.h"
#include "console.hpp"

@interface InterfaceGlue ()

+ (NSFont *)fontFor:(maf::StyledString::attributes_t)attributes;
+ (NSDictionary *)attributesFor:(maf::StyledString::attributes_t)attributes;

@end

using style_option = maf::StyledString::attributes_t::style_option;
using weight_option = maf::StyledString::attributes_t::weight_option;
using typeface_option = maf::StyledString::attributes_t::typeface_option;
using semantics_option = maf::StyledString::attributes_t::semantics_option;

@implementation InterfaceGlue {
	maf::Console _console;
	bool _game_in_progress;
}

- (void)awakeFromNib {
	_game_in_progress = false;
	[self showOutput];
}

- (IBAction)readInput:(id)sender {
	maf::string_view str = {self.input.stringValue.UTF8String};

	if (_console.input(str)) {
		[self showOutput];
	} else {
		[self showErrorMessage];
	}

	// Check if a game just started or ended. If so, change the music that's
	// playing.

	if (!_game_in_progress && _console.has_game()) {
		[_delegate playMusic:MafiaPlaylistItem_Beginning];
	} else if (_game_in_progress && !_console.has_game()) {
		[_delegate playMusic:MafiaPlaylistItem_None];
	}

	_game_in_progress = _console.has_game();
}

- (void)clearInput {
	self.input.stringValue = @"";
}

- (void)showOutput {
	NSMutableAttributedString *attributedString = [[NSMutableAttributedString alloc] init];
	bool clearWhitespaceFromFront = false;

	for (auto & styled_str : _console.output()) {
		NSString *string = [NSString stringWithUTF8String:styled_str.str.c_str()];

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

		if (styled_str.attributes.semantics == maf::StyledString::attributes_t::semantics_option::title) {
			_output.window.title = string;
			clearWhitespaceFromFront = true;
		} else {
			NSDictionary *attributes = [InterfaceGlue attributesFor:styled_str.attributes];
			NSAttributedString *newString = [[NSAttributedString alloc] initWithString:string attributes:attributes];
			[attributedString appendAttributedString:newString];
		}
	}

	self.output.textStorage.attributedString = attributedString;

	// Play some music.
	if (_console.has_game()) {
		const maf::Game_screen & screen = _console.game_log().active_screen();

		if (screen.id() == "time-changed") {
			auto & specific_screen = static_cast<const maf::Time_changed &>(screen);

			if (specific_screen.time == maf::core::Time::day) {
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
		} else if (screen.id() == "lynch-result") {
			auto & specific_screen = static_cast<const maf::Lynch_result &>(screen);

			if (specific_screen.victim && specific_screen.victim_role->is_troll()) {
				[_delegate playMusic:MafiaPlaylistItem_TrollLynch];
			}
		} else if (screen.id() == "duel-result") {
			[_delegate playMusic:MafiaPlaylistItem_Duel];
		} else if (screen.id() == "game-ended") {
			[_delegate playMusic:MafiaPlaylistItem_GameEnded];
		}
	}
}

- (void)showErrorMessage {
	NSAlert *alert = [[NSAlert alloc] init];
	NSMutableString *informativeText = [NSMutableString string];
	bool clearWhitespaceFromFront = false;

	for (auto & styled_str : _console.error_message()) {
		NSString *string = [NSString stringWithUTF8String:styled_str.str.c_str()];
		auto attributes = styled_str.attributes;

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

		if (attributes.semantics == maf::StyledString::attributes_t::semantics_option::title) {
			alert.messageText = string;
			clearWhitespaceFromFront = true;
		} else if (attributes.typeface == maf::StyledString::attributes_t::typeface_option::monospace) {
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

+ (NSFont *)fontFor:(maf::StyledString::attributes_t)attributes {
	if (attributes.semantics == semantics_option::flavour_text) {
		attributes.style = style_option::italic;
		attributes.typeface = typeface_option::serif;
	}

	NSString *fontName;

	switch (attributes.typeface) {
	case typeface_option::sans_serif:
		if (attributes.style == style_option::normal
			&& attributes.weight == weight_option::normal)
		{
			fontName = @"Helvetica";
		}
		else if (attributes.style == style_option::italic
				 && attributes.weight == weight_option::normal)
		{
			fontName = @"Helvetica Oblique";
		}
		else if (attributes.style == style_option::normal
				 && attributes.weight == weight_option::bold)
		{
			fontName = @"Helvetica Bold";
		}
		else /* if (attributes.style == style_option::italic
				 && attributes.weight == weight_option::bold) */
		{
			fontName = @"Helvetica Bold Oblique";
		}
		break;

	case typeface_option::serif:
		if (attributes.style == style_option::normal
			&& attributes.weight == weight_option::normal)
		{
			fontName = @"Times New Roman";
		}
		else if (attributes.style == style_option::italic
				 && attributes.weight == weight_option::normal)
		{
			fontName = @"Times New Roman Italic";
		}
		else if (attributes.style == style_option::normal
				 && attributes.weight == weight_option::bold)
		{
			fontName = @"Times New Roman Bold";
		}
		else /* if (attributes.style == style_option::italic
				 && attributes.weight == weight_option::bold) */
		{
			fontName = @"Times New Roman Bold Italic";
		}
		break;

	case typeface_option::monospace:
		fontName = @"Andale Mono";
		break;
	}

	return [NSFont fontWithName:fontName size:14];
}

+ (NSDictionary *)attributesFor:(maf::StyledString::attributes_t)attributes {
	NSFont *font = [InterfaceGlue fontFor:attributes];

	if (attributes.typeface == maf::StyledString::attributes_t::typeface_option::monospace) {
		return @{NSFontAttributeName: font,
				 NSForegroundColorAttributeName: [NSColor colorWithWhite:0.2 alpha:1],
				 NSBackgroundColorAttributeName: [NSColor colorWithWhite:0.93 alpha:1]};
	} else if (attributes.semantics == maf::StyledString::attributes_t::semantics_option::help_text) {
		return @{NSFontAttributeName: font,
				 NSForegroundColorAttributeName: [NSColor colorWithRed:0 green:0.42 blue:0.65 alpha:1]};
	} else if (attributes.semantics == maf::StyledString::attributes_t::semantics_option::flavour_text) {
		NSMutableParagraphStyle *paragraphStyle = [[NSMutableParagraphStyle alloc] init];
		paragraphStyle.alignment = NSTextAlignmentCenter;
		return @{NSFontAttributeName: font,
				 NSParagraphStyleAttributeName: paragraphStyle};
	} else {
		return @{NSFontAttributeName: font};
	}
}

@end
