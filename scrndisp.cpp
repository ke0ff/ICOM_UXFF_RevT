/********************************************************************
 *
 *  File name: scrndisp.c
 *
 *  Module:    Screen Display
 *
 *  Summary:   Contains generic console display functions.
 *
 *******************************************************************/

/********************************************************************
 *
 * Revision History:
 *    08-16-08 dms: creation date
 *
 *******************************************************************/


#include <windows.h>
#include <conio.h>
#include <ctype.h>

#include "scrndisp.h"




/***************************
 *   Private Defines
 ***************************/

/* none */

/***************************
 *   Private Variables
 ***************************/

static HANDLE hStdout;
static CONSOLE_SCREEN_BUFFER_INFO con;
static WORD NormAttributes;

/***************************
 *   Private Prototypes
 ***************************/

/* none */


/********************************************************************
 *
 *  Function:   init_screen
 *
 *  Requirements:  Initialize variables to allow display to the console.
 *      Get a handle for the standard output device and save the current
 *      display attributes
 *
 *  Return value:  none
 *
 *  Called by:  utility function may be called from anywhere
 *
 *  Functions called:  GetStdHandle
 *                     GetConsoleScreenBufferInfo
 *
 *  Input data:    none
 *
 *  Data modified:  hStdout
 *                  con
 *                  NormAttributes
 *
 *  Limitations:    none
 *
 *  Requirements Test Coverage: Requirements coverage achieved with 1 call.
 *
 *  Structural Test Coverage: Statement, decision and coupling achieved by
 *               requirements based test cases.
 *
 *  Notes:
 *
 *
 *  Revision History:
 *     08-17-08 dms:  creation date
 *     07-30-10 dms:  modified to allow screen size to be defined
 *
 *******************************************************************/
void init_screen(
        WORD screen_width,         // number of characters across in console window
        WORD screen_lines          // number of lines in console window
)
{
  COORD  dwSize;

  hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
  dwSize.X = screen_width;
  dwSize.Y = screen_lines;
  SetConsoleScreenBufferSize(hStdout, dwSize);
  GetConsoleScreenBufferInfo(hStdout, &con);
  con.srWindow.Right = con.srWindow.Left + dwSize.X - 1;
  con.srWindow.Bottom = con.srWindow.Top + dwSize.Y - 1;
  SetConsoleWindowInfo(hStdout, true, &con.srWindow);

  NormAttributes = con.wAttributes;
}


/********************************************************************
 *
 *  Function:   restore_screen
 *
 *  Requirements:  Restore the console screen to the state it was in
 *      when the program started.
 *
 *  Return value:  none
 *
 *  Called by:  utility function may be called from anywhere
 *
 *  Functions called:  SetConsoleTextAttribute
 *
 *  Input data:    none
 *
 *  Data modified:  none
 *
 *  Limitations:    none
 *
 *  Requirements Test Coverage: Requirements coverage achieved with 1 call.
 *
 *  Structural Test Coverage: Statement, decision and coupling achieved by
 *               requirements based test cases.
 *
 *  Notes:
 *
 *
 *  Revision History:
 *     08-17-08 dms:  creation date
 *
 *******************************************************************/
void restore_screen(void)
{
  SetConsoleTextAttribute(hStdout, NormAttributes);
}


/********************************************************************
 *
 *  Function:   gopos
 *
 *  Requirements:  Move the cursor to the specified row and column.
 *
 *  Return value:  none
 *
 *  Called by:  utility function may be called from anywhere
 *
 *  Functions called:  SetConsoleCursorPosition
 *
 *  Input data:    none
 *
 *  Data modified:  none
 *
 *  Limitations:    none
 *
 *  Requirements Test Coverage: Requirements coverage achieved with 1 call.
 *
 *  Structural Test Coverage: Statement, decision and coupling achieved by
 *               requirements based test cases.
 *
 *  Notes:
 *
 *
 *  Revision History:
 *     08-17-08 dms:  creation date
 *
 *******************************************************************/
void gopos(
        unsigned char row,         /* row to move cursor to */
        unsigned char col          /* column to move cursor to */
)
{
  COORD cp = {(unsigned int)(col-1), (unsigned int)(row-1)};

  SetConsoleCursorPosition(hStdout, cp);
}


/********************************************************************
 *
 *  Function:   clr_screen
 *
 *  Requirements:  Clear the screen.
 *
 *  Return value:  none
 *
 *  Called by:  utility function may be called from anywhere
 *
 *  Functions called:  GetConsoleScreenBufferInfo
 *                     FillConsoleOutputCharacter
 *                     GetConsoleScreenBufferInfo
 *                     FillConsoleOutputAttribute
 *                     SetConsoleCursorPosition
 *
 *  Input data:    hStdout
 *
 *  Data modified:  none
 *
 *  Limitations:    none
 *
 *  Requirements Test Coverage: Requirements coverage achieved with 1 call.
 *
 *  Structural Test Coverage: Statement, decision and coupling achieved by
 *               requirements based test cases.
 *
 *  Notes:
 *
 *
 *  Revision History:
 *     08-17-08 dms:  creation date
 *
 *******************************************************************/
void clr_screen(void)
{
  COORD coordScreen = { 0, 0 };    // home for the cursor
  DWORD cCharsWritten;
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  DWORD dwConSize;

  // Get the number of character cells in the current buffer.
  GetConsoleScreenBufferInfo(hStdout, &csbi);
  dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

  // Fill the entire screen with blanks.
  FillConsoleOutputCharacter(hStdout, ' ', dwConSize, coordScreen, &cCharsWritten);

  // Get the current text attribute.
  GetConsoleScreenBufferInfo(hStdout, &csbi);

  // Set the buffer's attributes accordingly.
  FillConsoleOutputAttribute(hStdout, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);

  // Put the cursor at its home coordinates.
  SetConsoleCursorPosition(hStdout, coordScreen);
}


/********************************************************************
 *
 *  Function:   clr_to_eol
 *
 *  Requirements:  Clear from cursor position to the end of the line.
 *
 *  Return value:  none
 *
 *  Called by:  utility function may be called from anywhere
 *
 *  Functions called:  GetConsoleScreenBufferInfo
 *                     FillConsoleOutputCharacter
 *                     FillConsoleOutputAttribute
 *                     SetConsoleCursorPosition
 *
 *  Input data:    hStdout
 *
 *  Data modified:  none
 *
 *  Limitations:    none
 *
 *  Requirements Test Coverage: Requirements coverage achieved with 1 call.
 *
 *  Structural Test Coverage: Statement, decision and coupling achieved by
 *               requirements based test cases.
 *
 *  Notes:
 *
 *
 *  Revision History:
 *     08-17-08 dms:  creation date
 *
 *******************************************************************/
void clr_to_eol(void)
{
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  DWORD cCharsWritten;
  DWORD dwConWidth;

  GetConsoleScreenBufferInfo(hStdout, &csbi);

  dwConWidth = csbi.dwSize.X - csbi.dwCursorPosition.X;

  FillConsoleOutputCharacter(hStdout, ' ', dwConWidth,
      csbi.dwCursorPosition, &cCharsWritten);

  FillConsoleOutputAttribute(hStdout, csbi.wAttributes,
      dwConWidth, csbi.dwCursorPosition, &cCharsWritten);

  SetConsoleCursorPosition(hStdout, csbi.dwCursorPosition);
}


/********************************************************************
 *
 *  Function:   set_text_color
 *
 *  Requirements:  Set the foreground and background colors for the display.
 *      set_text_color shifts the background color into the upper 4 bits and
 *      the foreground color into the lower 4 bits of a word and calls
 *      Windows function SetConsoleTextAttribute() to change the colors.
 *
 *  Return value:  none
 *
 *  Called by:  utility function may be called from anywhere
 *
 *  Functions called:  SetConsoleTextAttribute
 *
 *  Input data:    hStdout
 *
 *  Data modified:  none
 *
 *  Limitations:    none
 *
 *  Requirements Test Coverage: Requirements coverage achieved with 1 call.
 *
 *  Structural Test Coverage: Statement, decision and coupling achieved by
 *               requirements based test cases.
 *
 *  Notes:
 *
 *
 *  Revision History:
 *     08-17-08 dms:  creation date
 *
 *******************************************************************/
void set_text_color(
        unsigned int foreground_color,
        unsigned int background_color
)
{
  WORD attr;

  attr = (WORD)((background_color) << 4 | (foreground_color));
  SetConsoleTextAttribute(hStdout, attr);
}


/********************************************************************
 *
 *  Function:   shift_scroll_area
 *
 *  Requirements:  Move the specified area up one row.
 *
 *  Return value:  none
 *
 *  Called by:  utility function may be called from anywhere
 *
 *  Functions called:  TBD
 *
 *  Input data:    hStdout
 *
 *  Data modified:  none
 *
 *  Limitations:    none
 *
 *  Requirements Test Coverage: Requirements coverage achieved with 1 call.
 *
 *  Structural Test Coverage: Statement, decision and coupling achieved by
 *               requirements based test cases.
 *
 *  Notes:
 *
 *
 *  Revision History:
 *     11-02-11 dms:  creation date
 *
 *******************************************************************/
void shift_scroll_area(
        unsigned char top_row,
        unsigned char bottom_row,
        unsigned char window_width
)
{
  COORD      destination;          // upper left corner of scroll area
  CHAR_INFO  fill_char;            // characteristics of characters to fill moved area
  SMALL_RECT area_to_move;         // rectangle to be moved

  area_to_move.Left = 0;
  area_to_move.Top = top_row+1;
  area_to_move.Right = window_width;
  area_to_move.Bottom = bottom_row;

  destination.X = 0;
  destination.Y = top_row;

  fill_char.Char.AsciiChar = ' ';
  fill_char.Attributes = 0;

  ScrollConsoleScreenBuffer(hStdout, &area_to_move, NULL, destination, &fill_char);
}

