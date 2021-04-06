//
//  InterfaceGlue.h
//  Mafia
//
//  Created by Jack Copsey on 02/05/2015.
//  Copyright (c) 2015-2021. All rights reserved.
//

#include "console.hpp"

#import <Cocoa/Cocoa.h>

#import "AppDelegate.h"

@interface InterfaceGlue : NSObject {
   maf::Console _console;
}

@property (weak) IBOutlet AppDelegate *delegate;
@property (unsafe_unretained) IBOutlet NSTextView *output;
@property (weak) IBOutlet NSTextField *input;

- (void)awakeFromNib;

- (IBAction)readInput:(id)sender;
- (void)clearInput;

- (void)showOutput;
- (void)showErrorMessage;

+ (NSDictionary *)defaultAttributes;
+ (NSDictionary *)italicAttributes;
+ (NSDictionary *)helpTextAttributes;
+ (NSDictionary *)monospaceAttributes;

@end
