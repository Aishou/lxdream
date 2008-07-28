/**
 * $Id$
 *
 * Core Cocoa-based user interface
 *
 * Copyright (c) 2008 Nathan Keynes.
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
 */

#include <AppKit/AppKit.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "lxdream.h"
#include "dream.h"
#include "dreamcast.h"
#include "config.h"
#include "display.h"
#include "gui.h"
#include "cocoaui/cocoaui.h"

void cocoa_gui_update( void );
void cocoa_gui_start( void );
void cocoa_gui_stop( void );
void cocoa_gui_run_later( void );
uint32_t cocoa_gui_run_slice( uint32_t nanosecs );

struct dreamcast_module cocoa_gui_module = { "gui", NULL,
        cocoa_gui_update, 
        cocoa_gui_start, 
        cocoa_gui_run_slice, 
        cocoa_gui_stop, 
        NULL, NULL };

/**
 * Count of running nanoseconds - used to cut back on the GUI runtime
 */
static uint32_t cocoa_gui_nanos = 0;
static uint32_t cocoa_gui_ticks = 0;
static struct timeval cocoa_gui_lasttv;
static BOOL cocoa_gui_autorun = NO;
static BOOL cocoa_gui_is_running = NO;

@interface NSApplication (PrivateAdditions)
- (void) setAppleMenu:(NSMenu *)aMenu;
@end

/**
 * Produces the menu title by looking the text up in gettext, removing any
 * underscores, and returning the result as an NSString.
 */
static NSString *NSMENU_( const char *text )
{
    return [[NSString stringWithUTF8String: gettext(text)] stringByReplacingOccurrencesOfString: @"_" withString: @""];
}

static void cocoa_gui_create_menu(void)
{
    NSMenu *appleMenu, *services;
    NSMenuItem *menuItem;
    NSString *title;
    NSString *appName;

    appName = @"Lxdream";
    appleMenu = [[NSMenu alloc] initWithTitle:@""];

    /* Add menu items */
    title = [@"About " stringByAppendingString:appName];
    [appleMenu addItemWithTitle:title action:@selector(about_action:) keyEquivalent:@""];
    
    [appleMenu addItem:[NSMenuItem separatorItem]];
    [appleMenu addItemWithTitle: NSMENU_("_Preferences...") action:@selector(preferences_action:) keyEquivalent:@","];

    // Services Menu
    [appleMenu addItem:[NSMenuItem separatorItem]];
    services = [[[NSMenu alloc] init] autorelease];
    [appleMenu addItemWithTitle: NS_("Services") action:nil keyEquivalent:@""];
    [appleMenu setSubmenu: services forItem: [appleMenu itemWithTitle: @"Services"]];

    // Hide AppName
    title = [@"Hide " stringByAppendingString:appName];
    [appleMenu addItemWithTitle:title action:@selector(hide:) keyEquivalent:@"h"];

    // Hide Others
    menuItem = (NSMenuItem *)[appleMenu addItemWithTitle:@"Hide Others" 
                              action:@selector(hideOtherApplications:) 
                              keyEquivalent:@"h"];
    [menuItem setKeyEquivalentModifierMask:(NSAlternateKeyMask|NSCommandKeyMask)];

    // Show All
    [appleMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];
    [appleMenu addItem:[NSMenuItem separatorItem]];

    // Quit AppName
    title = [@"Quit " stringByAppendingString:appName];
    [appleMenu addItemWithTitle:title action:@selector(terminate:) keyEquivalent:@"q"];

    /* Put menu into the menubar */
    menuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil  keyEquivalent:@""];
    [menuItem setSubmenu: appleMenu];
    NSMenu *menu = [NSMenu new];
    [menu addItem: menuItem];

    NSMenu *gdromMenu = cocoa_gdrom_menu_new();

    NSMenu *fileMenu = [[NSMenu alloc] initWithTitle: NSMENU_("_File")];
    [fileMenu addItemWithTitle: NSMENU_("Load _Binary...") action: @selector(load_binary_action:) keyEquivalent: @"b"];
    [[fileMenu addItemWithTitle: NSMENU_("_GD-Rom") action: nil keyEquivalent: @""]
      setSubmenu: gdromMenu];
    [fileMenu addItem: [NSMenuItem separatorItem]];
    [[fileMenu addItemWithTitle: NSMENU_("_Reset") action: @selector(reset_action:) keyEquivalent: @"r"]
      setKeyEquivalentModifierMask:(NSAlternateKeyMask|NSCommandKeyMask)];
    [fileMenu addItemWithTitle: NSMENU_("_Pause") action: @selector(pause_action:) keyEquivalent: @"p"];
    [fileMenu addItemWithTitle: NS_("Resume") action: @selector(run_action:) keyEquivalent: @"r"];
    [fileMenu addItem: [NSMenuItem separatorItem]];
    [fileMenu addItemWithTitle: NSMENU_("_Load State...") action: @selector(load_action:) keyEquivalent: @"o"];
    [fileMenu addItemWithTitle: NSMENU_("_Save State...") action: @selector(save_action:) keyEquivalent: @"s"];

    menuItem = [[NSMenuItem alloc] initWithTitle:NSMENU_("_File") action: nil keyEquivalent: @""];
    [menuItem setSubmenu: fileMenu];
    [menu addItem: menuItem];

    /* Tell the application object that this is now the application menu */
    [NSApp setMainMenu: menu];
    [NSApp setAppleMenu: appleMenu];
    [NSApp setServicesMenu: services];

    /* Finally give up our references to the objects */
    [appleMenu release];
    [menuItem release];
    [menu release];
}

@interface LxdreamDelegate : NSObject
@end

@implementation LxdreamDelegate
- (void)windowWillClose: (NSNotification *)notice
{
    dreamcast_shutdown();
    exit(0);
}
- (void)windowDidBecomeMain: (NSNotification *)notice
{
    if( cocoa_gui_autorun ) {
        cocoa_gui_autorun = NO;
        cocoa_gui_run_later();
    }
}
- (void)windowDidBecomeKey: (NSNotification *)notice
{
    display_set_focused( TRUE );
}
- (void)windowDidResignKey: (NSNotification *)notice
{
    display_set_focused( FALSE );
    [((LxdreamMainWindow *)[NSApp mainWindow]) setIsGrabbed: NO];
}
- (void) about_action: (id)sender
{
    NSArray *keys = [NSArray arrayWithObjects: @"Version", @"Copyright", nil];
    NSArray *values = [NSArray arrayWithObjects: NS_(lxdream_full_version), NS_(lxdream_copyright),  nil];

    NSDictionary *options= [NSDictionary dictionaryWithObjects: values forKeys: keys];

    [NSApp orderFrontStandardAboutPanelWithOptions: options];
}
- (void) preferences_action: (id)sender
{
    cocoa_gui_show_preferences();
}
- (void) load_action: (id)sender
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    const gchar *dir = lxdream_get_config_value(CONFIG_SAVE_PATH);
    NSString *path = (dir == NULL ? NSHomeDirectory() : [NSString stringWithCString: dir]);
    NSArray *fileTypes = [NSArray arrayWithObject: @"dst"];
    int result = [panel runModalForDirectory: path file: nil types: fileTypes];
    if( result == NSOKButton && [[panel filenames] count] > 0 ) {
        NSString *filename = [[panel filenames] objectAtIndex: 0];
        dreamcast_load_state( [filename UTF8String] );
    }
}
- (void) save_action: (id)sender
{
    NSSavePanel *panel = [NSSavePanel savePanel];
    const gchar *dir = lxdream_get_config_value(CONFIG_SAVE_PATH);
    NSString *path = (dir == NULL ? NSHomeDirectory() : [NSString stringWithCString: dir]);
    [panel setRequiredFileType: @"dst"];
    int result = [panel runModalForDirectory: path file:@""];
    if( result == NSOKButton ) {
        NSString *filename = [panel filename];
        dreamcast_save_state( [filename UTF8String] );
    }
}
- (void) load_binary_action: (id)sender
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    const gchar *dir = lxdream_get_config_value(CONFIG_DEFAULT_PATH);
    NSString *path = (dir == NULL ? NSHomeDirectory() : [NSString stringWithCString: dir]);
    int result = [panel runModalForDirectory: path file: nil types: nil];
    if( result == NSOKButton && [[panel filenames] count] > 0 ) {
        NSString *filename = [[panel filenames] objectAtIndex: 0];
        file_load_magic( [filename UTF8String] );
    }
}
- (void) mount_action: (id)sender
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    const gchar *dir = lxdream_get_config_value(CONFIG_DEFAULT_PATH);
    NSString *path = (dir == NULL ? NSHomeDirectory() : [NSString stringWithCString: dir]);
    int result = [panel runModalForDirectory: path file: nil types: nil];
    if( result == NSOKButton && [[panel filenames] count] > 0 ) {
        NSString *filename = [[panel filenames] objectAtIndex: 0];
        gdrom_mount_image( [filename UTF8String] );
    }
}
- (void) pause_action: (id)sender
{
    dreamcast_stop();
}

- (void) reset_action: (id)sender
{
    dreamcast_reset();
}
- (void) run_action: (id)sender
{
    cocoa_gui_run_later();
}
- (void) run_immediate
{
    dreamcast_run();
}
- (void) gdrom_list_action: (id)sender
{
    gdrom_list_set_selection( [sender tag] );
}
@end


gboolean gui_parse_cmdline( int *argc, char **argv[] )
{
    /* If started from the finder, the first (and only) arg will look something like 
     * -psn_0_... - we want to remove this so that lxdream doesn't try to process it 
     * normally
     */
    if( *argc == 2 && strncmp((*argv)[1], "-psn_", 5) == 0 ) {
        *argc = 1;
    }
    return TRUE;
}

gboolean gui_init( gboolean withDebug )
{
    dreamcast_register_module( &cocoa_gui_module );

    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];

    LxdreamDelegate *delegate = [[LxdreamDelegate alloc] init];
    [NSApp setDelegate: delegate];
    NSString *iconFile = [[NSBundle mainBundle] pathForResource:@"dcemu" ofType:@"gif"];
    NSImage *iconImage = [[NSImage alloc] initWithContentsOfFile: iconFile];
    [iconImage setName: @"NSApplicationIcon"];
    [NSApp setApplicationIconImage: iconImage];   
    cocoa_gui_create_menu();
    NSWindow *window = cocoa_gui_create_main_window();
    [window makeKeyAndOrderFront: nil];
    [NSApp activateIgnoringOtherApps: YES];   

    [pool release];
}

void gui_main_loop( gboolean run )
{
    if( run ) {
        cocoa_gui_autorun = YES;
    }
    cocoa_gui_is_running = YES;
    [NSApp run];
    cocoa_gui_is_running = NO;
}

void gui_update_state(void)
{
    cocoa_gui_update();
}

gboolean gui_error_dialog( const char *msg, ... )
{
    if( cocoa_gui_is_running ) {
        NSString *error_string;

        va_list args;
        va_start(args, msg);
        error_string = [[NSString alloc] initWithFormat: [NSString stringWithCString: msg] arguments: args];
        NSRunAlertPanel(NS_("Error in Lxdream"), error_string, nil, nil, nil);
        va_end(args);
        return TRUE;
    } else {
        return FALSE;
    }
}

void gui_update_io_activity( io_activity_type io, gboolean active )
{

}


uint32_t cocoa_gui_run_slice( uint32_t nanosecs )
{
    NSEvent *event;
    NSAutoreleasePool *pool;

    cocoa_gui_nanos += nanosecs;
    if( cocoa_gui_nanos > GUI_TICK_PERIOD ) { /* 10 ms */
        cocoa_gui_nanos -= GUI_TICK_PERIOD;
        cocoa_gui_ticks ++;
        uint32_t current_period = cocoa_gui_ticks * GUI_TICK_PERIOD;

        // Run the event loop
        pool = [NSAutoreleasePool new];
        while( (event = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: nil 
                         inMode: NSDefaultRunLoopMode dequeue: YES]) != nil ) {
            [NSApp sendEvent: event];
        }
        [pool release];

        struct timeval tv;
        gettimeofday(&tv,NULL);
        uint32_t ns = ((tv.tv_sec - cocoa_gui_lasttv.tv_sec) * 1000000000) + 
        (tv.tv_usec - cocoa_gui_lasttv.tv_usec)*1000;
        if( (ns * 1.05) < current_period ) {
            // We've gotten ahead - sleep for a little bit
            struct timespec tv;
            tv.tv_sec = 0;
            tv.tv_nsec = current_period - ns;
            nanosleep(&tv, &tv);
        }

        /* Update the display every 10 ticks (ie 10 times a second) and 
         * save the current tv value */
        if( cocoa_gui_ticks > 10 ) {
            gchar buf[32];
            cocoa_gui_ticks -= 10;

            double speed = (float)( (double)current_period * 100.0 / ns );
            cocoa_gui_lasttv.tv_sec = tv.tv_sec;
            cocoa_gui_lasttv.tv_usec = tv.tv_usec;
            snprintf( buf, 32, _("Running (%2.4f%%)"), speed );
            [((LxdreamMainWindow *)[NSApp mainWindow]) setStatusText: buf];

        }
    }
    return nanosecs;
}

void cocoa_gui_update( void )
{

}

void cocoa_gui_start( void )
{
    LxdreamMainWindow *win = (LxdreamMainWindow *)[NSApp mainWindow];
    [win setRunning: YES];
    cocoa_gui_nanos = 0;
    gettimeofday(&cocoa_gui_lasttv,NULL);
}

void cocoa_gui_stop( void )
{
    LxdreamMainWindow *win = (LxdreamMainWindow *)[NSApp mainWindow];
    [win setRunning: NO];
}

/**
 * Queue a dreamcast_run() to execute after the currently event(s)
 */
void cocoa_gui_run_later( void )
{
    [[NSRunLoop currentRunLoop] performSelector: @selector(run_immediate) 
     target: [NSApp delegate] argument: nil order: 1 
     modes: [NSArray arrayWithObject: NSDefaultRunLoopMode] ];
}

/*************************** Convenience methods ***************************/

NSImage *NSImage_new_from_framebuffer( frame_buffer_t buffer )
{
    NSBitmapImageRep *rep = 
        [[NSBitmapImageRep alloc] initWithBitmapDataPlanes: &buffer->data
         pixelsWide: buffer->width  pixelsHigh: buffer->height
         bitsPerSample: 8 samplesPerPixel: 3
         hasAlpha: NO isPlanar: NO
         colorSpaceName: NSDeviceRGBColorSpace  bitmapFormat: 0
         bytesPerRow: buffer->rowstride  bitsPerPixel: 24];

    NSImage *image = [[NSImage alloc] initWithSize: NSMakeSize(0.0,0.0)];
    [image addRepresentation: rep];
    return image;
}


NSTextField *cocoa_gui_add_label( NSView *parent, NSString *text, NSRect frame )
{
    NSTextField *label = [[NSTextField alloc] initWithFrame: frame];
    [label setStringValue: text];
    [label setBordered: NO];
    [label setDrawsBackground: NO];
    [label setEditable: NO];
    [label setAutoresizingMask: (NSViewMinYMargin|NSViewMaxXMargin)];
    if( parent != NULL ) {
        [parent addSubview: label];
    }
    return label;
}
