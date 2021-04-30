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

@interface InterfaceGlue : NSObject

@property (weak) IBOutlet AppDelegate *delegate;
@property (unsafe_unretained) IBOutlet NSTextView *output;
@property (weak) IBOutlet NSTextField *input;

- (void)awakeFromNib;

- (IBAction)readInput:(id)sender;
- (void)clearInput;

- (void)showOutput;
- (void)showErrorMessage;

+ (NSFont *)fontFor:(maf::StyledString::attributes_t)attributes;
+ (NSDictionary *)attributesFor:(maf::StyledString::attributes_t)attributes;

@end
