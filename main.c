// compilation: gcc -lX11 -lXfixes -lXcursor main.c
// -DDUMP_MOZ_CURSORS -DDUMP_X11_CURSORS if needed

/* Base X window program shamelessly stolen from https://tronche.com/gui/x/xlib-tutorial/2nd-program-anatomy.html
 * (prog-2.cc - Written by Ch. Tronche)
 *
 * cursor.bdf was gotten from https://gitlab.freedesktop.org/xorg/font/cursor-misc/
 * (X.Org pointer cursor bitmaps)
 * 
 * nsGtkCursors was taken from Mozilla Firefox sources
 * (Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.).
 * */


#include <X11/Xlib.h>
// #include <X11/extensions/Xfixes.h>
#include <X11/Xcursor/Xcursor.h> // for Cursor
#include <X11/cursorfont.h> // XC_stuff
#include <X11/Xlibint.h> // CURSORFONT

#include "nsGtkCursors.h" // moz_ _bits

#include <stdio.h>  // for fprintf()
#include <stdlib.h> // for exit()
#include <string.h> // for strlen()
#include <unistd.h> // for sleep()
#include <limits.h> // for CHAR_BIT
#include <iso646.h> // for not

// bitmap byte consists of 8 'pixel' bits, but cursor needs 1 whole byte per that bit
#define Cursor_bitmap_side ( 32 )
#define Cursor_bitmap_width_in_bytes ( Cursor_bitmap_side / CHAR_BIT )

// #define Max_needed_length ( strlen( "STARTCHAR 01234567891014") + 2 )
// 'cause max name in cursor font file ¯\_(ツ)_/¯
// sb_h_double_arrow_mask = 22 symbols
// bottom_right_corner_mask = 24 symbols
	// for \n and null byte
#define Max_needed_length ( strlen( "STARTCHAR bottom_right_corner_mask\0" ) )
// #define Max_name_length ( strlen( "bottom_right_corner_mask " ) )
#define Max_read_length ( Max_needed_length - 1 )

int 
read_bitmap_from_bdf(
		FILE *stream, const char *cursor_name, unsigned char **data,
		int *width, int *height
		) {
	char temp_buf[ Max_needed_length ];
	char font_string_name[ Max_needed_length ];
	strncpy( font_string_name, "STARTCHAR ", strlen( "STARTCHAR+1" ) );
	strncpy( font_string_name + strlen( "STARTCHAR+1" ) - 1 /* \0 */ ,
		cursor_name, strlen( cursor_name ) + 1 );
	// printf( "Debug: font_string_name = %s (%ld)\n", font_string_name, strlen( font_string_name ) );
	*height = 0;
	*width = 0;
	int found = 0;
	int rows = 0;
	// skim for cursor name part
	while( NULL != fgets( temp_buf, Max_read_length, stream ) ) {
		rows++;
		if( 0 == strncmp( temp_buf, font_string_name, strlen( font_string_name ) ) ) {
			// printf( "Debug: found %s", temp_buf );
			found = 1;
			break;
			}
		}
	// printf( "Debug: bdf read rows = %d\n", rows );
	if( not found ) {
		return( ( printf( "\"%s\" character not found\n", cursor_name ) ), -1 );
		}

	// skip "encoding", "swidth", "dwidth"
		// sick shit, yeah, i know
	for( int i = 1; i <= 3 && NULL != fgets( temp_buf, Max_read_length, stream ); ++i ) {
		}
	// scan "BBX 1, 2, 3, 4"
	if( 0 == fscanf( stream, "%*3s %d %d %*d %*d\n", width, height ) ) {
		return( puts( "something bad happened with reading BBX line in .bdf file" ), -3 );
		}
	// printf( "Debug: bdf width = %d\t height = %d\n", *width, *height );
	// <8 equal 1 byte; <16 equal 2 bytes
	int loop_width = ( int )( ( *width + 7 ) / 8 );
	// skip "BITMAP"
	if( NULL == fgets( temp_buf, Max_read_length, stream ) ) {
		return( puts( "something bad happened with reading BITMAP line in .bdf file" ), -4 );
		}

	*data = calloc( Cursor_bitmap_side * Cursor_bitmap_side, sizeof( unsigned char ) );
	int element = 0;
	for( int i = 0; i < *height; ++i ) {
		if( NULL != fgets( temp_buf, Max_read_length, stream ) ) {
			// printf( "debug: bitmap %d = %s", i + 1, temp_buf );
				// as x11 bitmap consists of 32 bits per row...
			for( int j = 0; j < loop_width; j++ ) {
				// max 2 symbols at a time
				unsigned char temp[ 3 ] = { temp_buf[ j * 2 ], temp_buf[ j * 2 + 1 ], '\0' };
				// printf( "%8b ", ( unsigned int )strtoul( temp, NULL, 16 ) ); // not good

				typedef union {
					unsigned char c;
					struct { // for be able to get different bits from char
						unsigned char bit_1: 1;
						unsigned char bit_2: 1;
						unsigned char bit_3: 1;
						unsigned char bit_4: 1;
						unsigned char bit_5: 1;
						unsigned char bit_6: 1;
						unsigned char bit_7: 1;
						unsigned char bit_8: 1;
						};
				} byte_with_bits;

				byte_with_bits to_swap;
				to_swap.c = ( unsigned char )strtoul( temp, NULL, 16 );
				byte_with_bits to_swap_ref = to_swap;

				to_swap.bit_1 = to_swap_ref.bit_8;
				to_swap.bit_2 = to_swap_ref.bit_7;
				to_swap.bit_3 = to_swap_ref.bit_6;
				to_swap.bit_4 = to_swap_ref.bit_5;
				to_swap.bit_5 = to_swap_ref.bit_4;
				to_swap.bit_6 = to_swap_ref.bit_3;
				to_swap.bit_7 = to_swap_ref.bit_2;
				to_swap.bit_8 = to_swap_ref.bit_1;

				// ( *data )[ element++ ] = ( unsigned char )strtoul( temp, NULL, 16 ); // nope, nah
				( *data )[ element++ ] = to_swap.c;
				}
				// ...i need to add padding zero bytes
			for( int temp = loop_width; temp < Cursor_bitmap_width_in_bytes; temp++ ) {
				( *data )[ element++ ] = 0;
				}
				// puts( "" ); // end of debug output
			}
		else {
			puts( "Something really bad happened at reading cursor.bdf file" );
			}
		}
	// printf( "Debug: bitmap elements = %d\n", element );
	return 0;
}

static int bitmap_image_depth = 1;
#define To_nowhere NULL

// There is newline after printing hash.
XImage* 
make_cursor_ximage_from_data(
		Display *display, FILE *into_file, unsigned char *bitmap_data
		) {
	// <X11/X.h>
	// ZPixmap	2	/* depth == drawable depth */
	int offset = 0; // none
	int ximage_width = Cursor_bitmap_side; // hardcoded
	int ximage_height = Cursor_bitmap_side; // hardcoded
	int bitmap_pad = 32; // for ZPixmap
	int bytes_per_line = 0; // hardcoded (libX11/blob/master/src/ImUtil.c)
	// XCreateImage( display, XDefaultVisualOfScreen( XDefaultScreenOfDisplay( display ) ), 1, ZPixmap,
	// XCreateImage( display, XDefaultVisual( display, 0 ), 1, ZPixmap,
	XImage *my_ximage = XCreateImage(
			display, CopyFromParent, bitmap_image_depth, ZPixmap,
			offset, bitmap_data, ximage_width, ximage_height,
			bitmap_pad, bytes_per_line
			);
	// status = 1 if success
	int status = XInitImage( my_ximage );
	if( !status ) {
		return( puts( "Can't init XImage for some reason " ), NULL );
		}

	// <X11/Xcursor/Xcursor.h>
	// #define XCURSOR_BITMAP_HASH_SIZE    16
	unsigned char bitmap_hash[ XCURSOR_BITMAP_HASH_SIZE ] = { 0 };
	XcursorImageHash( my_ximage, bitmap_hash );
	if( into_file != To_nowhere ) {
		// copied from libXcursor/src/xlib.c
		for( int i = 0; i < XCURSOR_BITMAP_HASH_SIZE; i++ ) {
			fprintf( into_file, "%02x", bitmap_hash[ i ] );
			}
			fprintf( into_file, "\n" );
		}

	return my_ximage;
}

void 
print_bitmap(
		FILE *output, const unsigned char *bitmap, int height
		) {
	// a bitmap could occupy less real height lines than 32.
	for( int row = 0; row < height; row++ ) {
		for( int column_byte = 0; column_byte < Cursor_bitmap_width_in_bytes; column_byte++ ) {
			unsigned char c = bitmap[ row * Cursor_bitmap_width_in_bytes + column_byte ];
			for( int bit = 0; bit < CHAR_BIT; bit++ ) {
				( c >> bit & 1 ) ? fputc( '*', output ) : fputc( ' ', output );
				}
			}
			fputc( '\n', output ); // next row
		}
}

static const char *moz_cursors_names[] = {
	"moz_question_arrow_bits", "moz_hand_grab_bits", "moz_hand_grabbing_bits",
	"moz_copy_bits", "moz_alias_bits", "moz_menu_bits", "moz_spinning_bits",
	"moz_zoom_in_bits", "moz_zoom_out_bits", "moz_not_allowed_bits",
	"moz_col_resize_bits", "moz_row_resize_bits", "moz_vertical_text_bits",
	"moz_nesw_resize_bits", "moz_nwse_resize_bits", "moz_none_bits"
	};

// = 16
static unsigned long int size_of_moz_cursors_names = sizeof( moz_cursors_names )
	                 / //----------------------------------------------------
		                 sizeof( moz_cursors_names[ 0 ] );


static const char *X_cursors_names[] = {
	"X_cursor", "arrow", "based_arrow_down", "based_arrow_up", "boat", "bogosity", 
	"bottom_left_corner", "bottom_right_corner", "bottom_side", "bottom_tee", 
	"box_spiral", "center_ptr", "circle", "clock", "coffee_mug", "cross", 
	"cross_reverse", "crosshair", "diamond_cross", "dot", "dotbox", "double_arrow", 
	"draft_large", "draft_small", "draped_box", "exchange", "fleur", "gobbler", 
	"gumby", "hand1", "hand2", "heart", "icon", "iron_cross", "left_ptr", 
	"left_side", "left_tee", "leftbutton", "ll_angle", "lr_angle", "man", 
	"middlebutton", "mouse", "pencil", "pirate", "plus", "question_arrow", 
	"right_ptr", "right_side", "right_tee", "rightbutton", "rtl_logo", "sailboat", 
	"sb_down_arrow", "sb_h_double_arrow", "sb_left_arrow", "sb_right_arrow", 
	"sb_up_arrow", "sb_v_double_arrow", "shuttle", "sizing", "spider", "spraycan", 
	"star", "target", "tcross", "top_left_arrow", "top_left_corner", 
	"top_right_corner", "top_side", "top_tee", "trek", "ul_angle", "umbrella", 
	"ur_angle", "watch", "xterm"
	};

// XC_num_glyphs / 2
static unsigned long int size_of_X_cursors_names = sizeof( X_cursors_names )
		             / //---------------------------------------------------
			             sizeof( X_cursors_names[ 0 ] );


int main( void ) {
	// https://gitlab.freedesktop.org/xorg/lib/libxcursor/-/blob/master/src/xlib.c
	// setenv( "XCURSOR_DISCOVER", "1", 1 );
	Display *display = XOpenDisplay( NULL );
	if( display == NULL ) exit( EXIT_FAILURE );
	XcursorSetTheme( display, XCURSOR_CORE_THEME );
	// XcursorSetThemeCore( display, ( XcursorBool )1 );

	int black = BlackPixel( display, DefaultScreen( display ) );
	int white = WhitePixel( display, DefaultScreen( display ) );

	Window window = XCreateSimpleWindow( display, DefaultRootWindow( display ), 0, 0, 
					// 200, 200, 4, black, white );
					200, 200, 4, white, black );
	XSelectInput( display, window, StructureNotifyMask );
	XMapWindow( display, window );
	GC graphics_context = XCreateGC( display, window, 0, NULL );
	// XSetBackground( display, graphics_context, black ); // not working
	XSetForeground( display, graphics_context, white );
	// XSetForeground( display, graphics_context, black );
	// XSetFillStyle( display, graphics_context, FillSolid );
	for( ;; ) {
		XEvent event;
		XNextEvent( display, &event );
		if( event.type == MapNotify )
			break;
		}
	// XDrawLine( display, window, graphics_context, 10, 60, 180, 20 );

#define DATA( c ) { 0UL, c, c, c, 0, 0 }
	static XColor  xcolor_black = DATA( 0 );
	static XColor  xcolor_white = DATA( 65535 );
#undef DATA


	// now real stuff
	Cursor cursor;


	FILE *file_for_bitmap_output;
	// file_for_bitmap_output = stdout; // for debug
	unsigned char *bitmap_data = NULL;
	int width = 0;
	int height = 0;

#ifdef DUMP_MOZ_CURSORS
	file_for_bitmap_output = fopen( "moz_cursors.asc", "w" ); // only fully rewrite file here
	if( !file_for_bitmap_output ) {
		printf( "Can't open moz_cursors.asc file here.\n" );
		exit( EXIT_FAILURE );
		}
	for( int name = 0; name < size_of_moz_cursors_names; name++ ) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
		bitmap_data = GtkCursors[ name ].bits;
#pragma GCC diagnostic pop
		fprintf( file_for_bitmap_output, "%s ", moz_cursors_names[ name ] );
		fprintf( file_for_bitmap_output, "<=> " );
		XImage *my_ximage_temp = make_cursor_ximage_from_data(
			display, file_for_bitmap_output, bitmap_data );
// moz_bits will always be like 32x32 for hash purposes
//  but only some lines of height in reality. All other data is just zeroes.
//  "row_resize" 18
//  "not_allowed" 19
#define Moz_bits_output_height 19
		print_bitmap( file_for_bitmap_output, bitmap_data, Moz_bits_output_height );
		}
	bitmap_data = NULL; // no need for free it
	fclose( file_for_bitmap_output );
#endif /* DUMP_MOZ_CURSORS */

 

#ifdef DUMP_X11_CURSORS
	FILE *bdf = fopen( "cursor.bdf", "r" );
	if( bdf == NULL ) {
		puts( "Can't open cursor.bdf" );
		exit( EXIT_FAILURE );
		}
	file_for_bitmap_output = fopen( "x_core_cursors.asc", "w" ); // only fully rewrite file here
	if( !file_for_bitmap_output ) {
		printf( "Can't open x_core_cursors.asc file here.\n" );
		exit( EXIT_FAILURE );
		}
	char *cursor_name_to_find;
	// cursor_name_to_find = "arrow"; // for debug

	for( int name = 0; name < size_of_X_cursors_names; name++ ) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
		cursor_name_to_find = X_cursors_names[ name ];
#pragma GCC diagnostic pop
		if( !read_bitmap_from_bdf( bdf, cursor_name_to_find, &bitmap_data, &width, &height ) ) {
			// printf( "width = %d\t height = %d\n", width, height );
			// printf( "cursor -> %s\n", cursor_name_to_find );
			fprintf( file_for_bitmap_output, "%s ", cursor_name_to_find );
			fprintf( file_for_bitmap_output, "<=> " );
			XImage *my_ximage_temp = make_cursor_ximage_from_data( display, file_for_bitmap_output, bitmap_data );
			// fprintf( file_for_bitmap_output, "\n" ); // not needed after hash
			// 16 is max for cursor.bdf
#define X11_bitmap_output_height 16
			print_bitmap( file_for_bitmap_output, bitmap_data, X11_bitmap_output_height );
			} // if read_bitmap_from_bdf
		} // for name

	if( file_for_bitmap_output ) {
		fclose( file_for_bitmap_output );
		}
	fclose( bdf );
#endif /* DUMP_X11_CURSORS */


	// from /usr/include/X11/cursorfont.h
	int cursor_shape = XC_hand1;
	 
	// cursor = XCreateFontCursor( display, cursor_shape ); // yeah, now we can get default ones

	// or such way
	// cursor = XcursorShapeLoadCursor( display, cursor_shape );
 
	// this one works too
	// Font cur_font = XLoadFont( display, CURSORFONT );
	// cursor = XCreateGlyphCursor( display, cur_font, cur_font, cursor_shape, cursor_shape + 1, &xcolor_black, &xcolor_white );
	// XUnloadFont( display, cur_font );

	// cursor = XcursorLibraryLoadCursor( display, "left_ptr_watch" ); // from your Theme

	// if( cursor == None ) {
	// 	puts( "Something is totally wrong, cursor couldn't be find..." );
	// 	exit( EXIT_FAILURE );
	// 	}
	 
	// XSynchronize( display, 1 ); // turn on failed requests debug

	// now we use ximage for setting cursor
	/*
	bitmap_data = GtkCursors[ 9 ].bits; // forbidden bitmap
	XImage *my_ximage = make_cursor_ximage_from_data( display, To_nowhere, bitmap_data );
	GC ximage_gc;
	Pixmap forbidden = XCreatePixmap( display, window, 32, 32, bitmap_image_depth );
	ximage_gc = XCreateGC( display, forbidden, 0, NULL );
	XPutImage( display, forbidden, ximage_gc, my_ximage, 0, 0, 0, 0, 32, 32 );
	XFreeGC( display, ximage_gc );

	bitmap_data = GtkCursors[ 9 ].mask_bits; // mask for it
	Pixmap forbidden_mask = XCreatePixmap( display, window, 32, 32, bitmap_image_depth );
	my_ximage = make_cursor_ximage_from_data( display, To_nowhere, bitmap_data );
	ximage_gc = XCreateGC( display, forbidden_mask, 0, NULL );
	XPutImage( display, forbidden_mask, ximage_gc, my_ximage, 0, 0, 0, 0, 32, 32 );
	XFreeGC( display, ximage_gc );
	*/

	/* The pixels set to 1 in the mask define which source pixels are displayed,
	 * and the pixels set to 0 define which pixels are ignored.
	 */
	// cursor = XCreatePixmapCursor( display, forbidden, 0,
			// &xcolor_white, &xcolor_black, 2, 2 );
			// &xcolor_black, &xcolor_white, 2, 2 ); // nope
	/*cursor = XCreatePixmapCursor( display, forbidden, forbidden_mask,
			// &xcolor_white, &xcolor_black, 2, 2 );
			&xcolor_black, &xcolor_white, 2, 2 );
	*/

	// XDefineCursor( display, window, cursor ); // for displaying our cursor
	XFlush( display );

	sleep( 3 ); // enough for moving cursor and staring at it
	// XFreePixmap( display, forbidden );
	// XFreePixmap( display, forbidden_mask );
	// XFreeCursor( display, cursor );
	// unsetenv( "XCURSOR_DISCOVER" );
	exit( EXIT_SUCCESS );
}

