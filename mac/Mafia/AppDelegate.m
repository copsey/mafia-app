//
//  AppDelegate.m
//  Mafia
//
//  Created by Jack Copsey on 01/05/2015.
//  Copyright (c) 2015 Ponderous Programs. All rights reserved.
//

#import <ScriptingBridge/ScriptingBridge.h>
#import "iTunes.h"

#import "AppDelegate.h"

@interface AppDelegate () {
   NSInteger _iTunesVolumeOriginal;
   MafiaPlaylistItem _pendingMusic;
   NSTimer *_fadeOutMusicTimer;
}

- (iTunesApplication *)getITunes;
- (void)fadeOutMusicTimerFired:(id)sender;

@property (weak) IBOutlet NSWindow *window;
@end

@implementation AppDelegate

- (id)init {
   self = [super init];

   if (self) {
      // The instance variables are initialised.
      _playsMusic = NO;
      _fadeOutMusicTimer = nil;
      _pendingMusic = MafiaPlaylistItem_None;

      // Default values for the app's preferences are registered.
      NSUserDefaults *preferences = [NSUserDefaults standardUserDefaults];
      [preferences registerDefaults:@{@"playsMusic": @YES}];

      // The preferences are now loaded.
      self.playsMusic = [preferences boolForKey:@"playsMusic"];
   }

   return self;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
   // The states of buttons in the preferences window are initially set.
   NSUserDefaults *preferences = [NSUserDefaults standardUserDefaults];

   // Whether or not music should be played is determined.
   self.togglePlaysMusicButton.state = ([preferences boolForKey:@"playsMusic"]) ? NSControlStateValueOn : NSControlStateValueOff;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
   // Any music which is playing in iTunes is stopped.
   {
      // The fade out music timer is invalidated, if required.
      if (_fadeOutMusicTimer) {
         [_fadeOutMusicTimer invalidate];
      }

      // If music is being played, then all music is stopped, and the original volume level is restored.
      iTunesApplication *iTunes = [SBApplication applicationWithBundleIdentifier:@"com.apple.iTunes"];
      if (_playsMusic && iTunes.isRunning) {
         [iTunes stop];
         iTunes.soundVolume = _iTunesVolumeOriginal;
      }
   }

   // The app's preferences are synchronised.
   [[NSUserDefaults standardUserDefaults] synchronize];
}

- (void)setPlaysMusic:(BOOL)playsMusic {
   if (_playsMusic == playsMusic) return;

   iTunesApplication *iTunes = [self getITunes];

   if (playsMusic) {
      _iTunesVolumeOriginal = iTunes.soundVolume;
   } else {
      iTunes.soundVolume = _iTunesVolumeOriginal;
      [iTunes stop];

      [_fadeOutMusicTimer invalidate];
      _pendingMusic = MafiaPlaylistItem_None;
   }

   _playsMusic = playsMusic;
}

- (void)togglePlaysMusic:(id)sender {
   BOOL playsMusic = ([sender state] == NSControlStateValueOn);
   [[NSUserDefaults standardUserDefaults] setBool:playsMusic forKey:@"playsMusic"];
   [self setPlaysMusic:playsMusic];
}

- (iTunesApplication *)getITunes {
   iTunesApplication *iTunes = [SBApplication applicationWithBundleIdentifier:@"com.apple.iTunes"];
   if (iTunes.isRunning == NO) [iTunes run];
   return iTunes;
}

- (void)playMusic:(MafiaPlaylistItem)aMafiaPlaylistItem {
   // The track is only played if music is enabled.
   if (_playsMusic) {
      // A reference to iTunes is obtained.
      iTunesApplication *iTunes = [self getITunes];

      // If a track is currently playing, then it is faded out, and the new track is stored.
      if (iTunes.playerState == iTunesEPlSPlaying) {
         _pendingMusic = aMafiaPlaylistItem;

         if (_fadeOutMusicTimer.isValid == NO) {
            _fadeOutMusicTimer = [NSTimer scheduledTimerWithTimeInterval:0.03 target:self selector:@selector(fadeOutMusicTimerFired:) userInfo:nil repeats:YES];
         }
      }

      // Otherwise, the new track starts playing.
      else {
         // The Mafia playlist is obtained.
         iTunesPlaylist *playlist = [[[[iTunes sources] objectAtIndex:0] playlists] objectWithName:@"Mafia"];

         // The volume level of iTunes is restored.
         iTunes.soundVolume = _iTunesVolumeOriginal;

         // If the required track exists in the playlist, then it is played.
         if (aMafiaPlaylistItem >= 0 && playlist.tracks.count > aMafiaPlaylistItem) {
            iTunesTrack *track = [[playlist tracks] objectAtIndex:aMafiaPlaylistItem];

            if (track) {
               [iTunes stop];
               [track playOnce:YES];
            }
         }
      }
   }
}

- (void)fadeOutMusic {
   [self playMusic:MafiaPlaylistItem_None];
}

- (void)fadeOutMusicTimerFired:(id)sender {
   // A reference to iTunes is obtained.
   iTunesApplication *iTunes = [self getITunes];

   // If the volume level is still audible, then it is decreased.
   if (iTunes.soundVolume > 0) {
      iTunes.soundVolume -= _iTunesVolumeOriginal/32;
   }

   // Otherwise, the pending track is played, and the timer stops.
   else {
      [sender invalidate];
      [iTunes stop];
      [self playMusic:_pendingMusic];
   }
}

@end
