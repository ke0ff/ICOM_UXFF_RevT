/********************************************************************
 *
 *  File name: scrndisp.h
 *
 *  Module:    Screen Display
 *
 *  Summary:   This header contains the variable and function
 *             definitions for module scrndisp.cpp
 *
 *******************************************************************/

/********************************************************************
 *  File scope declarations revision history:

 *    08-17-08 dms:  creation date
 *
 *******************************************************************/


/****    Defines   ****/

enum COLORS
{
    BLACK,      BLUE,         GREEN,      CYAN,          /* dark colors */
    RED,        MAGENTA,      BROWN,      LIGHTGRAY,
    DARKGRAY,   LIGHTBLUE,    LIGHTGREEN, LIGHTCYAN,     /* light colors */
    LIGHTRED,   LIGHTMAGENTA, YELLOW,     WHITE
};



/****    Variable Declarations   ****/
#ifndef  ALLOCATE
#define  ALLOCATE    extern
#endif

/* none */

#undef   ALLOCATE


/****    Function Prototypes   ****/

void init_screen(WORD screen_width, WORD screen_lines);
void restore_screen(void);
void gopos(unsigned char row, unsigned char col);
void clr_screen(void);
void clr_to_eol(void);
void set_text_color(unsigned int foreground_color, unsigned int background_color);
void shift_scroll_area(unsigned char top_row, unsigned char bottom_row, unsigned char window_width);

#define SCRNDISP_H
