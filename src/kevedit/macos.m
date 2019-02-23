/* macos.m         -- macOS specific functions
 * Copyright (C) 2019 Kev Vance <kvance@kvance.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place Suite 330; Boston, MA 02111-1307, USA.
 */

#import <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>

#include <stdio.h>
#include "macos.h"
#include "SDL_syswm.h"

static NSString *const BarIdentifier = @"com.kvance.kevedit.bar";
static NSString *const ItemIdentifier = @"com.kvance.kevedit.bar.item";
static NSString *const CreatureIdentifier = @"com.kvance.kevedit.bar.creature";
static NSString *const TerrianIdentifier = @"com.kvance.kevedit.bar.terrain";
static NSString *const TextIdentifier = @"com.kvance.kevedit.bar.text";

@interface TouchBarResponder: NSResponder<NSTouchBarDelegate>
@property (nonatomic, assign) BOOL touchBarEnabled;
@property (nonatomic, assign) BOOL textMode;
- (void)itemClicked:(NSButton *)sender;
- (void)creatureClicked:(NSButton *)sender;
- (void)terrainClicked:(NSButton *)sender;
- (void)textClicked:(NSButton *)sender;
- (void)injectScancode:(SDL_Scancode)scancode keycode:(SDL_Keycode)keycode;
@end


static TouchBarResponder *currentTouchBarResponder = nil;


void installTouchBar(SDL_Window *sdl_window) {
    // Get the NSWindow from SDL.
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    if(SDL_GetWindowWMInfo(sdl_window, &wminfo) == SDL_FALSE) {
        NSLog(@"Failed to install touchbar: failed to get WM info from SDL: %s", SDL_GetError());
        return;
    }
    if(wminfo.subsystem != SDL_SYSWM_COCOA) {
        NSLog(@"Failed to install touchbar: unexpected window subsystem %d", wminfo.subsystem);
        return;
    }
    NSWindow *window = wminfo.info.cocoa.window;

    // Add a TouchBarResponder to the window's responder chain.
    currentTouchBarResponder = [[TouchBarResponder alloc] init];
    currentTouchBarResponder.nextResponder = window.nextResponder;
    window.nextResponder = currentTouchBarResponder;
}

void enableTouchBarWithTextMode(bool textMode) {
    if(currentTouchBarResponder) {
        currentTouchBarResponder.touchBarEnabled = YES;
        currentTouchBarResponder.textMode = textMode;
    }
}

void disableTouchBar() {
    if(currentTouchBarResponder) {
        currentTouchBarResponder.touchBarEnabled = NO;
    }
}

@implementation TouchBarResponder
- (id)init {
    self = [super init];
    if(self) {
        self->_touchBarEnabled = YES;
    }
    return self;
}

- (NSTouchBar *)makeTouchBar {
    NSTouchBar *bar = [[NSTouchBar alloc] init];
    bar.delegate = self;
    bar.customizationIdentifier = BarIdentifier;
    bar.defaultItemIdentifiers = @[
        ItemIdentifier,
        CreatureIdentifier,
        TerrianIdentifier,
        TextIdentifier,
    ];
    return bar;
}

- (NSTouchBarItem *)touchBar:(NSTouchBar *)touchBar makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier {
    NSCustomTouchBarItem *item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
    if([identifier isEqualToString:ItemIdentifier]) {
        item.view = [NSButton buttonWithTitle:@"Item"
                                       target:self
                                       action:@selector(itemClicked:)];
    } else if([identifier isEqualToString:CreatureIdentifier]) {
        item.view = [NSButton buttonWithTitle:@"Creature"
                                       target:self
                                       action:@selector(creatureClicked:)];
    } else if([identifier isEqualToString:TerrianIdentifier]) {
        item.view = [NSButton buttonWithTitle:@"Terrian"
                                       target:self
                                       action:@selector(terrainClicked:)];
    } else if([identifier isEqualToString:TextIdentifier]) {
        NSButton *button = [NSButton buttonWithTitle:@"Text"
                                              target:self
                                              action:@selector(textClicked:)];
        button.buttonType = NSPushOnPushOffButton;
        [button highlight:self.textMode];
        item.view = button;
    }
    return item;
}

- (void)setTouchBarEnabled:(BOOL)enabled {
    if(enabled == self->_touchBarEnabled) {
        return;
    }
    self->_touchBarEnabled = enabled;
    for(NSTouchBarItemIdentifier itemId in self.touchBar.defaultItemIdentifiers) {
        NSTouchBarItem *item = [self.touchBar itemForIdentifier:itemId];
        if([item.view isKindOfClass:[NSButton class]]) {
            ((NSButton *)item.view).enabled = enabled;
        }
    }
}

- (void)setTextMode:(BOOL)textMode {
    if(textMode == self->_textMode) {
        return;
    }
    self->_textMode = textMode;
    NSTouchBarItem *item = [self.touchBar itemForIdentifier:TextIdentifier];
    if([item.view isKindOfClass:[NSButton class]]) {
        [((NSButton *)item.view) highlight:textMode];
    }
}


- (void)itemClicked:(NSButton *)sender {
    [self injectScancode:SDL_SCANCODE_F1 keycode:SDLK_F1];
}

- (void)creatureClicked:(NSButton *)sender {
   [self injectScancode:SDL_SCANCODE_F2 keycode:SDLK_F2];
}

- (void)terrainClicked:(NSButton *)sender {
  [self injectScancode:SDL_SCANCODE_F3 keycode:SDLK_F3];
}

- (void)textClicked:(NSButton *)sender {
    [self injectScancode:SDL_SCANCODE_F4 keycode:SDLK_F4];
}

- (void)injectScancode:(SDL_Scancode)scancode keycode:(SDL_Keycode)keycode {
    SDL_Keysym key = {
        .scancode = scancode,
        .sym = keycode,
        .mod = 0
    };

    // Key down
    SDL_Event event = {
        .key = {
            .type = SDL_KEYDOWN,
            .state = SDL_PRESSED,
            .repeat = 0,
            .keysym = key,
        }
    };
    SDL_PushEvent(&event);

    // Key up
    event.key.type = SDL_KEYUP;
    event.key.state = SDL_RELEASED;
    event.key.keysym = key;
    SDL_PushEvent(&event);
}

@end
