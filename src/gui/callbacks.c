/**
 * $Id: callbacks.c,v 1.17 2006-12-15 10:17:08 nkeynes Exp $
 *
 * All GTK callbacks go here (stubs are autogenerated by Glade)
 *
 * Copyright (c) 2005 Nathan Keynes.
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include "gui/callbacks.h"
#include "gui/interface.h"
#include "gui/gui.h"
#include "gdrom/gdrom.h"
#include "mem.h"
#include "mmio.h"
#include "dreamcast.h"
#include "loader.h"

int selected_pc = -1;
int selected_row = -1;

void
on_new_file1_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_open1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    const gchar *dir = dreamcast_get_config_value(CONFIG_DEFAULT_PATH);
    open_file_dialog( "Open...", file_load_magic, NULL, NULL, dir );
}


void
on_save1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_save_as1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_exit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gtk_main_quit();
}


void
on_preferences1_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    GtkWidget *about = create_about_win();
    gtk_widget_show(about);
}


void
on_load_btn_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
    const gchar *dir = dreamcast_get_config_value(CONFIG_DEFAULT_PATH);
    open_file_dialog( "Open...", gdrom_mount_image, NULL, NULL, dir );
}


void
on_reset_btn_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
    dreamcast_reset();
}


void
on_stop_btn_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
    dreamcast_stop();
}


void
on_step_btn_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
    debug_info_t data = get_debug_info(GTK_WIDGET(button));
    debug_win_single_step(data);
}


void
on_run_btn_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
    dreamcast_run();
}


void
on_runto_btn_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
    if( selected_pc == -1 )
        WARN( "No address selected, so can't run to it", NULL );
    else {
	debug_info_t data = get_debug_info(GTK_WIDGET(button));
	debug_win_set_oneshot_breakpoint( data, selected_row );
	dreamcast_run();
    }
}


void
on_break_btn_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
    debug_info_t data = get_debug_info(GTK_WIDGET(button));
    debug_win_toggle_breakpoint( data, selected_row );
}

void on_trace_button_toggled           (GtkToggleButton *button,
					gpointer user_data)
{
    struct mmio_region *io_rgn = (struct mmio_region *)user_data;
    gboolean isActive = gtk_toggle_button_get_active(button);
    if( io_rgn != NULL ) {
	io_rgn->trace_flag = isActive ? 1 : 0;
    }
}


gboolean
on_debug_win_delete_event              (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    gtk_main_quit();
    return FALSE;
}


void
on_disasm_list_select_row              (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    debug_info_t data = get_debug_info(GTK_WIDGET(clist));
    selected_pc = row_to_address(data, row);
    selected_row = row;
}


void
on_disasm_list_unselect_row            (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    debug_info_t data = get_debug_info(GTK_WIDGET(clist));
    int pc = row_to_address(data,row);
    if( selected_pc == pc ) selected_pc = -1;
}


void
on_mem_mapped_regs1_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    mmr_open_win();
}


gboolean
on_mmr_win_delete_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    mmr_close_win();
    return TRUE;
}


void
on_mmr_close_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
    mmr_close_win();
}


void
on_mode_field_changed                  (GtkEditable     *editable,
                                        gpointer         user_data)
{
    const gchar *text = gtk_entry_get_text( GTK_ENTRY(editable) );
    debug_info_t data = get_debug_info( GTK_WIDGET(editable) );
    set_disassembly_cpu( data, text );
}


void
on_page_locked_btn_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

}


gboolean
on_page_field_key_press_event          (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{
    if( event->keyval == GDK_Return || event->keyval == GDK_Linefeed ) {
	debug_info_t data = get_debug_info(widget);
        const gchar *text = gtk_entry_get_text( GTK_ENTRY(widget) );
        gchar *endptr;
        unsigned int val = strtoul( text, &endptr, 16 );
        if( text == endptr ) { /* invalid input */
            char buf[10];
            sprintf( buf, "%08X", row_to_address(data,0) );
            gtk_entry_set_text( GTK_ENTRY(widget), buf );
        } else {
            set_disassembly_region(data, val);
        }
    }
    return FALSE;
}


void
on_output_list_select_row              (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
    if( event->type == GDK_2BUTTON_PRESS && event->button.button == 1 ) {
        char *val;
        gtk_clist_get_text( clist, row, 2, &val );
        if( val[0] != '\0' ) {
            int addr = strtoul( val, NULL, 16 );
	    debug_info_t data = get_debug_info( GTK_WIDGET(clist) );
            jump_to_disassembly( data, addr, TRUE );
        }
    }
}


void
on_jump_pc_btn_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
    debug_info_t data = get_debug_info( GTK_WIDGET(button) );
    jump_to_pc( data, TRUE );
}


void
on_button_add_watch_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_clear_all_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_close_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_view_memory_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    dump_window_new();
}


void
on_loadstate_button_clicked            (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
    const gchar *dir = dreamcast_get_config_value(CONFIG_SAVE_PATH);
    open_file_dialog( "Load state...", dreamcast_load_state, "*.dst", "lxDream Save State (*.dst)", dir );
}


void
on_savestate_button_clicked            (GtkToolButton   *toolbutton,
                                        gpointer         user_data)
{
    const gchar *dir = dreamcast_get_config_value(CONFIG_SAVE_PATH);
    save_file_dialog( "Save state...", dreamcast_save_state, "*.dst", "lxDream Save State (*.dst)", dir );
}

