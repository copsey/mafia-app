//
//  InterfaceGlue.h
//  Mafia
//
//  Created by Jack Copsey on 02/05/2015.
//  Copyright (c) 2015 Ponderous Programs. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#include "console.hpp"

@interface InterfaceGlue : NSObject {
   mafia::Console _console;
}

@property (unsafe_unretained) IBOutlet NSTextView *output;
@property (weak) IBOutlet NSTextField *input;

- (void)awakeFromNib;

- (IBAction)readInput:(id)sender;

- (void)showOutput;
- (void)showErrorMessage;

+ (NSDictionary *)gameStyleAttributes;
+ (NSDictionary *)gameItalicStyleAttributes;
+ (NSDictionary *)helpStyleAttributes;
+ (NSDictionary *)commandStyleAttributes;

@end
