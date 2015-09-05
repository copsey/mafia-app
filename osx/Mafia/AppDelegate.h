//
//  AppDelegate.h
//  Mafia
//
//  Created by Jack Copsey on 01/05/2015.
//  Copyright (c) 2015 Ponderous Programs. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "MafiaPlaylist.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property (nonatomic, getter = shouldPlayMusic) BOOL playsMusic;
@property (assign) IBOutlet NSButton *togglePlaysMusicButton;

- (IBAction)togglePlaysMusic:(id)sender;
- (void)playMusic:(MafiaPlaylistItem)aMafiaPlaylistItem;
- (void)fadeOutMusic;

@end

