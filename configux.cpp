/********************************************************************
 *
 *  File name: configux.cpp
 *
 *  Summary:   This program configures the PLL, VOL, SQU, and LOHI power
 *              options for the UXFFront module.
 *
 *             The serial port is configured for 9600 baud, no parity.
 *
 *  Called via:   configux <comm port>
 *
 *                <comm port> number, default is 4
 *
 *   If no com port is specified at the command line, the user is prompted
 *      for one (ENTER selects the default).
 *
 *******************************************************************/

/********************************************************************
 *
 * Revision History:
 *     Version 0.0 : copied project from SerialTest with thanks to DianaS.
 *
 *******************************************************************/

#include "stdafx.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/timeb.h>

#include "pcserial.h"
#include "comm.h"
#include "scrndisp.h"
#include "scrnattr.h"
#include "uxpll.h"


#define VERSION     "0.1"

#define CR          0x0d                 /* ASCII carriage return */
#define ESC         0x1b                 /* ASCII ESCape character */

unsigned char lastScrollRowUsed;
unsigned char scrollRow;
unsigned char scrollCol;

FILE* logFile;


char setpll(long freqKK, long* pllarray, char is_tx);
char get_bandid(long freqMM);
char get_hex(char* s);
char upcase(char c);
void get_serial(char ipls);
void wait_ms(short int delay);
bool timer_ms(short int delay, bool set);
bool alive_timer_sec(char delay, bool set);

static void redraw_screen(int comm_port_num, bool autol);
static void help_screen(void);
void file_cmd(long* p_array);


/********************************************************************
 *
 * Function: main
 *
 * Requirements Summary:  Initialize the PC serial port as required
 *      by the program's input parameters and print received bytes
 *      on the screen in hex format.
 *
 * Return value: normal DOS exit
 *
 * Limitations:
 *
 * Notes:
 * Communitcates with operator and target radio to compute and exchange
 *   config data.  Detects radio reset and auto-copies current config
 *   (need a switch to disable that feature).  Updates radio in real-time
 *   and provides rssi and COS/PTT status.  User initiates FLASH Write
 *   and ErAse commands.
 *
 * Revision History:
 *    01-22-21 jmh:  Basic features complete.  Need to do system testing
 *                  and feature creep.
 *    01-20-21 jmh:  creation date - copied from SerialTest, modified for UXFF
 *
 *******************************************************************/
int _tmain(int argc, _TCHAR* argv[])
{
    bool  autoload = false;         /* autoload flag */
    bool  ipl;                      /* initial start flag */
    bool  iplt;                     /* initial time start flag */
    bool  abort;                    /* true to exit program */
    bool  dead_flag = false;        /* true if no device comms */
    long  baud_rate;                /* comm port baud rate, no default */
    long  freq;                     /* UI freq, no default */
    double ffreq;                   /* float freq */
    long  tt;                       /* temps */
    long  ii;
    short int test_count = 0;
    int   com_port;                 /* comm port id (defaults to COM1) */
    int   parity;                   /* comm port parity (defaults to NO_PARITY) */
    char  i;                        /* temp */
    char  j;                        /* temp */
    char  k;                        /* temp */
    char  ch;                       /* byte read from com port */
    char  key;                      /* key pressed on the PC keyboard */
    char  input_str[20];
//    __timeb64 curr_time_sec;        /* current time in seconds */
    __timeb64 tx_time_sec;          /* transmit time in seconds */
    long pll_array[11];             /* target radio data array */
    char obuf[40];                  /* serial tx buffer */
#define IBUF_LEN 15
    char ibuf[255];                 /* serial rx buffer */
    char iptr;                      /* serial RX index */

    init_screen(WINDOW_WIDTH_IN_CHARS, WINDOW_LENGTH_IN_LINES);
    fflush(stdin);
    printf("UXFFront Configurator version %s\n", VERSION);

    for (i = 0; i < 11; i++) {
        pll_array[i] = 0xffffffff;
    }

    if (argc < 2) {
        // If no comport selected at command line, ask user to select one here...
        printf("Usage:  configux <comm port>\n\n");
        printf("        where:   <comm port> defaults to 4\n");
        printf("Enter COM port (ENTER to accept default): ");
        clr_to_eol();
        set_text_color(DATA_COLOR, BACK_COLOR);
        gets_s(input_str, sizeof(input_str));
        com_port = atoi(input_str);
        if (!com_port) {
            com_port = 4;                           // no or "0" entry, select default
        }
    }
    baud_rate = 9600L; 
    iptr = 0;                                       // init serial RX array
    get_serial(1);                                  // init serial display
    parity = NOPARITY;
    if (argc > 1) {
        com_port = atoi(argv[1]);
    }

    if (configCommPort(com_port, baud_rate, parity, 8, ONESTOPBIT) ) {
        redraw_screen(com_port, autoload);

        _ftime64_s(&tx_time_sec);
        lastScrollRowUsed = SCROLL_ROW;
        scrollRow = SCROLL_ROW;
        scrollCol = 1;
        abort = false;
        ipl = true;
        iplt = 1;

        while ( !abort ) {
            if (serialByteIn(&ch)) {
                if((ch == 't') || (ch == 'R') || (ch == '#')) {
                    ibuf[iptr] = ch;
                    iptr += 1;
                }else {
                    if (ibuf[0]) {
                        ibuf[iptr] = ch;
                        iptr += 1;
                    }
                }
                ibuf[iptr] = '\0';                      // place tail null
                if (iptr > IBUF_LEN) {
                    iptr = 0;                           // buffer overflow error, reset buffer
                    ibuf[0] = 0;
                }else {
                        if (iptr == 4) {
                            // process status
                            if (ibuf[0] == 'R') {       // RX status
                                i = ibuf[1] - '0';
                                j = get_hex(&ibuf[2]);
                                gopos(PROG_ID_ROW, TR_STATUS_COL);
                                set_text_color(WARN_COLOR, BACK_COLOR);
                                if ((i < 2) && (j <= 255)) {
                                    printf("rx %u %03u  ", i, j);
                                    alive_timer_sec(ALIVE_TIME, SET_TIMER);
                                    dead_flag = false;
                                }
                            }
                            if (ibuf[0] == 't') {       // TX status
                                j = get_hex(&ibuf[2]);
                                gopos(PROG_ID_ROW, TR_STATUS_COL);
                                set_text_color(WARN_COLOR, BACK_COLOR);
                                if (j >= 0) {
                                    printf("TX - %03u  ", j);
                                    alive_timer_sec(ALIVE_TIME, SET_TIMER);
                                    dead_flag = false;
                                }
                            }
                            if ((ibuf[0] == '#') && autoload) {       // radio IPL detect
                                if ((ibuf[1] == 'U') && (ibuf[2] == 'X') && (ibuf[3] == 'F')) {
                                    get_serial(0);                          // clean serial buffer
                                    get_serial(0);
                                    get_serial(0);
                                    sprintf_s(obuf, "F\n");                 // issue Flash copy command
                                    putss(obuf);
                                    get_serial(0);                          // capture response msg
                                    for (i = 0; i < 11; i++) {              // gather data
                                        getss(ibuf);
                                        j = sscanf_s(ibuf, "%08x", &tt);
                                        pll_array[i] = tt;
                                    }
                                    gopos(PROG_ID_ROW, TR_STATUS_COL);
                                    set_text_color(WARN_COLOR, BACK_COLOR);
                                    printf("Radio INIT         \n");        // post operator alert
                                    wait_ms(400);
                                    wait_ms(400);
                                    wait_ms(400);
                                }
                                alive_timer_sec(ALIVE_TIME, SET_TIMER);
                                dead_flag = false;
                            }
                            iptr = 0;                   // reset buffer
                            ibuf[0] = 0;
                        }
                }
            }
            if (!dead_flag && (!alive_timer_sec(ALIVE_TIME, READ_TIMER))) {
                dead_flag = true;
                ipl = true;
            }
            if (ipl) {
                gopos(PROG_ID_ROW, TR_STATUS_COL);
                set_text_color(WARN_COLOR, BACK_COLOR);
                printf(".. . ...");
                ipl = false;
            }
            if (_kbhit() != 0) {
                key = _getch();
                switch (key) {
                    case ESC :
                        // end program
                        abort = true;
                        break;

                    case '?':                 // < > help screen
                        // help screen
                        help_screen();
                        redraw_screen(com_port, autoload);
                        break;

                    case ' ':                 // < > redraw screen
                        // redraw screen
                        redraw_screen(com_port, autoload);
                        break;

                    case 'A':                 // < > autoload toggle
                        // autoload toggle
                        if (autoload) {
                            autoload = false;
                        }else{
                            autoload = true;
                        }
                        clr_to_eol();
                        set_text_color(LABEL_COLOR, BACK_COLOR);
                        gopos(INPUT_ROW, 1);
                        redraw_screen(com_port, autoload);
                        break;

                    case 'R':                 // <R> reset radio
                        // reset radio
                        printf("Reset Radio                                     \n");
                        sprintf_s(obuf, "R\n");
                        putss(obuf);
                        get_serial(0);
                        break;

                    case 'F':                 // <F> file command
                        // file command
                        file_cmd(pll_array);
                        break;

                    case 'p':                 // <p> TX power
                        // TX power
                        clr_to_eol();
                        set_text_color(LABEL_COLOR, BACK_COLOR);
                        gopos(INPUT_ROW, 1);
                        _cputs("Enter TX power (1 = high, 0 = low): ");
                        clr_to_eol();
                        set_text_color(DATA_COLOR, BACK_COLOR);
                        gets_s(input_str, sizeof(input_str));
                        ii = atoi(input_str);
                        if (ii) {                                        // set rf power status
                            printf("HIGH power                           \n");
                            ii = 1;                                     // force to 1
                            pll_array[10] &= 0xdfffffff;                // invert in the data
                        }
                        else {
                            printf("Low power                            \n");
                            pll_array[10] |= 0x20000000;
                        }
                        sprintf_s(obuf, "m%02x\n", ii);                   // update radio
                        putss(obuf);
                        get_serial(0);
                        break;

                    case 'r':                 // <R> RX freq enter
                        // RX Freq
                        clr_to_eol();
                        set_text_color(LABEL_COLOR, BACK_COLOR);
                        gopos(INPUT_ROW, 1);
                        _cputs("Enter RX Freq in MHz: ");
                        clr_to_eol();
                        set_text_color(DATA_COLOR, BACK_COLOR);
                        gets_s(input_str, sizeof(input_str));
                        ffreq = atof(input_str);
                        if (ffreq > 1199.999) {
                            freq = (long)(ffreq / 0.01);                // align 1200 MHz frequencies to 10KHz boundary
                            freq *= 10;
                        }else{
                            freq = (long)(ffreq / 0.005);               // all others align to 5 KHz
                            freq *= 5;
                        }
                        printf("RX Freq entered = %u KHz                 \n", freq);
                        i = setpll(freq, pll_array, 0);
                        if (i) {                                        // set wbrx status
                            pll_array[10] |= 0x10000000;
                        }else {
                            pll_array[10] &= 0xefffffff;
                        }
                        break;

                    case 't':                 // <T> TX freq enter
                        // TX Freq
                        clr_to_eol();
                        set_text_color(LABEL_COLOR, BACK_COLOR);
                        gopos(INPUT_ROW, 1);
                        _cputs("Enter TX Freq in MHz: ");
                        clr_to_eol();
                        set_text_color(DATA_COLOR, BACK_COLOR);
                        gets_s(input_str, sizeof(input_str));
                        ffreq = atof(input_str);
                        if (ffreq > 1199.999) {
                            freq = (long)(ffreq / 0.01);                // align 1200 MHz frequencies to 10KHz boundary
                            freq *= 10;
                        }
                        else {
                            freq = (long)(ffreq / 0.005);               // all others align to 5 KHz
                            freq *= 5;
                        }
                        printf("TX Freq entered = %u KHz                \n", freq);
                        setpll(freq, pll_array, 1);
                        break;

                    case 'd':                 // display pll array
                        // display array
                        set_text_color(LABEL_COLOR, BACK_COLOR);
                        gopos(OUTPUT_ROW, 1);
                        _cputs("PLL Array:\n");
                        clr_to_eol();
                        set_text_color(DATA_COLOR, BACK_COLOR);
                        for (i = 0; i < 11; i++) {
                            printf("%02u %08x            \n", i, pll_array[i]);
                        }
                        break;

                    case 'S':                 // send pll array to target
                        // send array
                        set_text_color(LABEL_COLOR, BACK_COLOR);
                        gopos(OUTPUT_ROW, 1);
                        _cputs("Sending PLL Array...\n");
                        clr_to_eol();
                        set_text_color(DATA_COLOR, BACK_COLOR);
                        for (i = 0; i < 11; i++) {
                            sprintf_s(obuf,"p%x%08x\n", i, pll_array[i]);
                            putss(obuf);
                            get_serial(0);
                        }
                        break;

                    case 'v':                 // <V> VOL
                    case 'V':
                        // set VOL
                        set_text_color(LABEL_COLOR, BACK_COLOR);
                        gopos(INPUT_ROW, 1);
                        _cputs("Enter RX VOL (0 - 255): ");
                        clr_to_eol();
                        set_text_color(DATA_COLOR, BACK_COLOR);
                        gets_s(input_str, sizeof(input_str));
                        tt = atoi(input_str);
                        printf("VOL entered = %u                                     \n", tt);
                        pll_array[10] = (pll_array[10] & VOL_MASK) | tt;
                        sprintf_s(obuf, "v%02x\n", tt);
                        putss(obuf);
                        get_serial(0);
                        break;

                    case 'q':                 // <q> SQU
                        // set SQU
                        set_text_color(LABEL_COLOR, BACK_COLOR);
                        gopos(INPUT_ROW, 1);
                        _cputs("Enter SQU (0 {open} - 255 {closed}): ");
                        clr_to_eol();
                        set_text_color(DATA_COLOR, BACK_COLOR);
                        gets_s(input_str, sizeof(input_str));
                        tt = atoi(input_str);
                        printf("SQU entered = %u                                     \n", tt);
                        pll_array[10] = (pll_array[10] & SQU_MASK) | (tt << SQU_SHIFT);
                        sprintf_s(obuf, "s%02x\n", tt);
                        putss(obuf);
                        get_serial(0);
                        break;

                    case 'Q':                 // <Q> SQUA
                        // set SQUA
                        set_text_color(LABEL_COLOR, BACK_COLOR);
                        gopos(INPUT_ROW, 1);
                        _cputs("Enter SQUA (0 {open} - 255 {closed}): ");
                        clr_to_eol();
                        set_text_color(DATA_COLOR, BACK_COLOR);
                        gets_s(input_str, sizeof(input_str));
                        tt = atoi(input_str);
                        printf("SQU entered = %u                                     \n", tt);
                        pll_array[10] = (pll_array[10] & SQUA_MASK) | (tt << SQUA_SHIFT);
                        sprintf_s(obuf, "q%02x\n", tt);
                        putss(obuf);
                        get_serial(0);
                        break;

                    case 'P':                 // <p> PLL frame
                        // Manually set PLL frame
                        set_text_color(LABEL_COLOR, BACK_COLOR);
                        gopos(INPUT_ROW, 1);
                        _cputs("Enter PLL field offset (0 - 10): ");
                        clr_to_eol();
                        set_text_color(DATA_COLOR, BACK_COLOR);
                        gets_s(input_str, sizeof(input_str));
                        ii = atoi(input_str);
                        if (ii < 11) {
                            set_text_color(LABEL_COLOR, BACK_COLOR);
                            gopos(INPUT_ROW, 1);
                            _cputs("Enter PLL Hex data (8 chr max): ");
                            clr_to_eol();
                            set_text_color(DATA_COLOR, BACK_COLOR);
                            gets_s(input_str, sizeof(input_str));
                            i = sscanf_s(input_str, "%08x", &tt);
                            printf("PLL entered = %08x                                     \n", tt);
                            pll_array[(char)ii] = tt;
                        }else{
                            _cputs("ERROR, PLL field invalid\n");
                        }
                        break;

                    case 'D':                 // <D> read FLASH
                        // read FLASH from device
                        sprintf_s(obuf, "F\n");
                        putss(obuf);
                        get_serial(0);
                        k = SERIAL_ROW;
                        for (i = 0; i < 11; i++) {
//                            sprintf_s(obuf, "p%x%08x\n", i, pll_array[i]);
//                            putss(obuf);
                            getss(ibuf);
                            gopos(k++, 1);
                            set_text_color(LABEL_COLOR, BACK_COLOR);
                            _cputs(ibuf);
                            clr_to_eol();
                            if (k > BOTTOM_ROW) {
                                k = SERIAL_ROW;
                            }
                            j = sscanf_s(ibuf, "%08x", &tt);
                            pll_array[i] = tt;
                        }
                        set_text_color(LABEL_COLOR, BACK_COLOR);
                        gopos(INPUT_ROW, 1);
                        _cputs("Done.");
                        clr_to_eol();
                        break;

                    case 'E':                 // <E> Erase FLASH
                        // Erase FLASH
                        sprintf_s(obuf, "EA\n");
                        putss(obuf);
                        get_serial(0);
                        _cputs("\n");
                        sprintf_s(obuf, "Y");
                        putss(obuf);
                        get_serial(0);
                        break;

                    case 'W':                 // <W> Write FLASH
                        // Write FLASH
                        sprintf_s(obuf, "W\n");
                        putss(obuf);
                        get_serial(0);
                        _cputs("\n");
                        break;

                    default:
                        break;
                }
            }
        }
        closeCommPort();
    }
    else {
        printf("Error initializing comm port.\r\n");
        printf("Press any key to continue... \n");
        while (!(_getch()));
    }
}
#undef TX_RATE_MS
#undef MS_PER_SEC


///////////////////////////////////
//  redraw_screen
///////////////////////////////////
static void redraw_screen(int comm_port_num, bool autol)
{
    unsigned char row;

    clr_screen();
    set_text_color(LABEL_COLOR, BACK_COLOR);
    for (row=1; row<=WINDOW_LENGTH_IN_LINES; row++) {
        // set background color for window
        gopos(row, 1);
        clr_to_eol();
    }
    gopos(PROG_ID_ROW, 1);
    set_text_color(LABEL_COLOR, BACK_COLOR);
    _cprintf("UXFFront Config Tool version %s on comm %d  --  ESC to quit", VERSION, comm_port_num);

    gopos(MENU_ROW_1, 1);
    set_text_color(LABEL_COLOR, BACK_COLOR);
    _cputs("  !! Frequencies are in MHz and are truncated to the next lowest 5KHz !!");
    clr_to_eol();
    gopos(MENU_ROW_2, 1);
    set_text_color(LABEL_COLOR, BACK_COLOR);
    _cputs("  !!   1200 MHz Frequencies are truncated to the next lowest 10KHz    !!");
    clr_to_eol();
    gopos(MENU_ROW_3, 1);
    set_text_color(LABEL_COLOR, BACK_COLOR);
    _cputs("<?> Help                   <t> Set TX frequency      <p> TX power");
    clr_to_eol();
    gopos(MENU_ROW_4, 1);
    set_text_color(LABEL_COLOR, BACK_COLOR);
    _cputs("<R> reset radio            <r> Set RX frequency");
    clr_to_eol();
    gopos(MENU_ROW_5, 1);
    set_text_color(LABEL_COLOR, BACK_COLOR);
    _cputs("<V> Set VOL                <q> Set squelch           <Q> set AUX squelch");
    clr_to_eol();
    gopos(MENU_ROW_6, 1);
    set_text_color(LABEL_COLOR, BACK_COLOR);
    _cputs("<D> read FLASH             <P> Manual PLL frame entry");
    clr_to_eol();
    gopos(MENU_ROW_7, 1);
    set_text_color(LABEL_COLOR, BACK_COLOR);
    _cputs("<S> Send config to Radio   <W> Write Radio config to FLASH");
    clr_to_eol();
    gopos(MENU_ROW_8, 1);
    set_text_color(LABEL_COLOR, BACK_COLOR);
    _cputs("<E> Erase config FLASH (only use this command if there is a Write Error)");
    clr_to_eol();
    gopos(MENU_ROW_9, 1);
    set_text_color(LABEL_COLOR, BACK_COLOR);
    if (autol) {
        _cputs("<space> Re-draw screen     <A> Auto-load mode [ON]   <F> File Commands    ");
    }else{
        _cputs("<space> Re-draw screen     <A> Auto-load mode [off]  <F> File Commands    ");
    }
    clr_to_eol();
    gopos(PROG_ID_ROW, TR_STATUS_COL);
    set_text_color(WARN_COLOR, BACK_COLOR);
    printf(".. . ...");
}

///////////////////////////////////
//  help_screen
///////////////////////////////////
static void help_screen(void)
{
//    unsigned char row;

    clr_screen();
/*    set_text_color(LABEL_COLOR, BACK_COLOR);
    for (row = 1; row <= WINDOW_LENGTH_IN_LINES; row++) {
        // set background color for window
        gopos(row, 1);
        clr_to_eol();
    }*/
    gopos(PROG_ID_ROW, 1);
    set_text_color(LABEL_COLOR, BACK_COLOR);
    _cprintf("UXFFront Config Tool HELP, page 1 version %s (c) 2021 by Joseph M. Haas", VERSION);

    gopos(MENU_ROW_2, 1);
    set_text_color(LABEL_COLOR, BACK_COLOR);
//  _cputs("12345678901234567890123456789012345678901234567890123456789012345678901234567890123\n");    // ruler line
    _cputs("This software supports the UXFFront Module stand-alone project. It allows the\n");          // line 1
    _cputs("  operator to set Frequency, audio level (volume), squelch.  For the T/R versions,\n");     // line 2
    _cputs("  TX frequency and TX power are also configured.  Simple file operations (save and\n");     // line 3
    _cputs("  load) are also supported to allow for configurations to be saved for off-line\n");        // line 4
    _cputs("  storage.\n");                                                                             // line 5
    _cputs("\n");                                                                                       // line 6
    _cputs("The UXFF module communicates via RS-232 at 9600 baud, N81.  This PC must have a\n");        // line 7
    _cputs("  COM port of a known port number to connect to the target module.  The software\n");       // line 8
    _cputs("  starts with a blank configuration.  If auto-load is enabled, the software will\n");       // line 9
    _cputs("  detect when the radio powers-on and load its current configuration.\n");                  // line 10
    _cputs("\n");                                                                                       // line 11
    _cputs("First open this software and select a valid COM port. Then connect the to the COM\n");      // line 12
    _cputs("  port and apply power (if power is already applied, the auto-load feature won't\n");       // line 13
    _cputs("  activate - in this case, use the 'D' command to load the current radio config).\n");      // line 14
    _cputs("  Enter the frequency, TX power, Vol, and squelch prameters using the 1-letter\n");         // line 15
    _cputs("  commands in the previous screen.  Note: if the AUX Squelch is not used, set it\n");       // line 16
    _cputs("  to the same value as the squelch setting.\n");                                            // line 17
    _cputs("\n");                                                                                       // line 18
    _cputs("Once the configuration is complete, test the radio to verify proper operation.\n");         // line 19
    _cputs("  if correct, the configuration must be saved to FLASH using the 'W' command\n");           // line 20
    _cputs("  If an error results, use the 'EA' command to free space in the FLASH storage\n");         // line 21
    _cputs("  area, then repeat the 'W' commnd to save the configuration.\n");                          // line 22
    _cputs("\n");                                                                                       // line 23
    _cputs("Cycle power to the radio to verify its operation (First use the file commands\n");          // line 24
    _cputs("  to save the configuration).  If the radio operates correctly, it is ready\n");            // line 25
    _cputs("   for service.\n");                                                                        // line 26

    clr_to_eol();
    set_text_color(LABEL_COLOR, BACK_COLOR);
    gopos(BOTTOM_ROW + 1, 1);
    _cputs("Press any key to continue...");
    while(!_getch());

    clr_screen();
    gopos(PROG_ID_ROW, 1);
    set_text_color(LABEL_COLOR, BACK_COLOR);
    _cprintf("UXFFront Config Tool HELP, page 2");

    gopos(MENU_ROW_1, 1);
    set_text_color(LABEL_COLOR, BACK_COLOR);
//  _cputs("12345678901234567890123456789012345678901234567890123456789012345678901234567890123\n");    // ruler line
    _cputs("If desired, use the 'F' file commands to save the configuration to the PC disk.\n");        // line 1
    _cputs("  This file can then be retrieved later to restore or 'clone' the configuration\n");        // line 2
    _cputs("  to other modules.  The file commands are very simple and are best managed by\n");         // line 3
    _cputs("  placing the configuration files into the same directory as this application.\n");         // line 4
    _cputs("  The files are text format, so the '.txt' extention is recommended.\n");                   // line 5
    _cputs("\n");                                                                                       // line 6
    _cputs("See https://github.com/ke0ff/ICOM_UXFF for the latest version of this program\n");          // line 7
    _cputs("  as well as other support resources for the UXFF modules.\n");                             // line 8

    clr_to_eol();
    set_text_color(LABEL_COLOR, BACK_COLOR);
    gopos(BOTTOM_ROW + 1, 1);
    _cputs("Press any key to continue...");
    while (!_getch());
}

///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// file_cmd() saves or loads the config data to/from a text file
//-----------------------------------------------------------------------------
void file_cmd(long* p_array) {
    char    key;
    char    in_str[80];

    // TX power
    clr_to_eol();
    set_text_color(LABEL_COLOR, BACK_COLOR);
    gopos(INPUT_ROW, 1);
    _cputs("File (S)ave, (L)oad, or (A)bort: ");
    do {
        key = _getch();
    }while (!key);
    key = upcase(key);
    if((key == 'S') || (key == 'L')){
        gopos(INPUT_ROW, 1);
        clr_to_eol();
        _cputs("Enter filename (80 chr, max): ");
        set_text_color(DATA_COLOR, BACK_COLOR);
        gets_s(in_str, sizeof(in_str));
        _cputs(in_str);
    }
    gopos(INPUT_ROW, 1);
    clr_to_eol();
    gopos(INPUT_ROW+1, 1);
    clr_to_eol();
    return;
}

///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// setpll() calculates pll data using frequency in KHz
//    returns value of "1" if wbrx indicated, else "0"
//-----------------------------------------------------------------------------
char setpll(long freqKK, long *pllarray, char is_tx)
{
    char	i;
    char	band_id;
    char	wbrx = FALSE;		// disable wide band RX (BAND = 0)
    long pll;
    long ii;
    long jj;
    long tt;
    long* pllptr = pllarray;

    // calculate band_id using MM MM vfo digits
    i = get_bandid(freqKK / 1000L);
    band_id = (char)(i & 0x0f);
    if (i > 0x7f) wbrx = TRUE;
    // construct band select bitmap
    // convert VFO freq to channelized value for PLL formatting
    switch (band_id) {
    case ID10M:
        pll = (freqKK - BASE_RX_10M) / 5L;			// convert to 5KHz chan
        pll += PLL_10M;								// add base PLL bitmap
        if (is_tx) {
            pll -= BASE_TX_10M;						// subtract tx offset (for 10M and 6M)
            i = 5;
        }else{
            i = 0;
        }
        pll <<= 1;									// align bitmap (only for 10M and 6M)
        pllptr[i + 0] = INIT_PLL_6M | PLL_BITLEN;       // store init frame
        pllptr[i + 1] = pll | PLL_BITLEN;               // store PLL plus bit length
        pllptr[i + 2] = 0xffffffff;                     // clear unused frames
        pllptr[i + 3] = 0xffffffff;
        pllptr[i + 4] = 0xffffffff;
        break;

    case ID6M:
        pll = (freqKK - BASE_RX_6M) / 5L;			// convert to 5KHz chan
        pll += PLL_6M;								// add base PLL bitmap
        if (is_tx) {
            pll -= BASE_TX_6M;						// subtract tx offset (for 10M and 6M)
            i = 5;
        }else{
            i = 0;
        }
        pll <<= 1;									// align bitmap (only for 10M and 6M)
        pllptr[i + 1] = pll | PLL_BITLEN;               // store PLL plus bit length
        pllptr[i + 0] = INIT_PLL_6M | PLL_BITLEN;       // store init frame
        pllptr[i + 2] = 0xffffffff;                     // clear unused frames
        pllptr[i + 3] = 0xffffffff;
        pllptr[i + 4] = 0xffffffff;
        break;

    case ID2M:
        pll = (freqKK - BASE_RX_2M) / 5L;			// convert to 5KHz chan
        pll += PLL_2M;								// add base PLL bitmap
        if (is_tx) {
            pll += BASE_TX_2M;						// add tx offset
            i = 5;
        }else{
            i = 0;
        }
        // insert a "0" into bit 7
        pll = ((pll << 1) & 0x3ff80L) | (pll & 0x003fL);
        pllptr[i + 0] = pll | PLL_BITLEN;               // store PLL plus bit length
        pllptr[i + 1] = 0xffffffff;                     // clear unused frames
        pllptr[i + 2] = 0xffffffff;                     // clear unused frames
        pllptr[i + 3] = 0xffffffff;
        pllptr[i + 4] = 0xffffffff;
        break;

    case ID220:
        pll = (freqKK - BASE_RX_220) / 5L;			// convert to 5KHz chan
        pll += PLL_220;								// add base PLL bitmap
        if (is_tx) {
            pll += BASE_TX_220;						// add tx offset
            i = 5;
        }else{
            i = 0;
        }
        // insert a "0" into bit 7
        pll = ((pll << 1) & 0x3ff80L) | (pll & 0x003fL);
        pllptr[i + 0] = pll | PLL_BITLEN;               // store PLL plus bit length
        pllptr[i + 1] = 0xffffffff;                     // clear unused frames
        pllptr[i + 2] = 0xffffffff;                     // clear unused frames
        pllptr[i + 3] = 0xffffffff;
        pllptr[i + 4] = 0xffffffff;
        break;

    case ID440:
        pll = (freqKK - BASE_RX_440) / 5L;			// convert to 5KHz chan
        pll += PLL_440;								// add base PLL bitmap
        if (is_tx) {
            pll += BASE_TX_440;						// add tx offset
            i = 5;
        }else{
            i = 0;
        }
        pllptr[i + 0] = pll | PLL_BITLEN;               // store PLL plus bit length
        pllptr[i + 1] = 0xffffffff;                     // clear unused frames
        pllptr[i + 2] = 0xffffffff;                     // clear unused frames
        pllptr[i + 3] = 0xffffffff;
        pllptr[i + 4] = 0xffffffff;
        break;

    case ID1200:
        pll = (freqKK - BASE_RX_1200) / 10L;		    // convert to 10KHz chan
        pll += PLL_1200;							    // add base PLL bitmap
        if (is_tx) {
            pll += BASE_TX_1200;					    // add tx offset
            i = 5;
        }else{
            i = 0;
        }
        // bit reverse...
        // insert a "0" into bit 7
        pll = ((pll << 1) & 0x3ff80L) | (pll & 0x003fL);
        for (ii = 0x80000L, jj = 1L, tt = 0L; ii != 0; ii >>= 1, jj <<=1) {
            if (pll & ii) tt |= jj;
        }
        pllptr[i + 0] = INIT_PLL_1200 | PLL_BITLEN;     // init frame
        pllptr[i + 1] = tt | PLL_BITLEN;                // store PLL plus bit length
        pllptr[i + 2] = INIT_PLL_1201 | PLL_1200BITLEN; // post-init frames
        pllptr[i + 3] = INIT_PLL_1202 | PLL_1200BITLEN;
        pllptr[i + 4] = INIT_PLL_1203 | PLL_1200BITLEN;
        break;

    default:
        break;
    }
    // set squelch select
//	squ_out(band_id);
    return wbrx;
}

///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// get_bandid() calculates band id from freq in MHz
//-----------------------------------------------------------------------------
char get_bandid(long freqMM)
{
    char	j;
    char	i;

    j = (char)(freqMM / 100);
    switch (j) {
    case 0:										// is 10M or 6M?
        j = (char)(freqMM);
        if (j > 39) {								// is 6M (MM MM between 0040 and 0099)?
            i = ID6M;
            if ((j < 50) || (j >= 54)) {
                i |= 0x80;							// enable wide band RX (BAND = 1)
            }
        }
        else {
            if (j < 20) {							// is 10M (MM MM between 0020 and 0040)?
                i = 0x00;							// no,	freq invalid error (MM MM < 0020)
            }
            else {
                i = ID10M;							// is 10M
            }
        }
        break;

    case 1:										// is 2M ?
        i = ID2M;
        j = (char)(freqMM - 100);
        if ((j < 44) || (j >= 48)) {
            i |= 0x80;								// enable wide band RX (BAND = 1)
        }
        break;

    case 2:										// is 220 ?
        i = ID220;
        break;

    case 4:										// is 440 ?
        i = ID440;
        break;

    case 12:									// is 1200 ?
        i = ID1200;
        break;

    default:
        i = BANDOFF;							// is error
        break;
    }
    return i;
}

///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// get_hex() returns hex conversion of two chars at *s
//-----------------------------------------------------------------------------
char get_hex(char* s) {
    char    i;     // temps
    char    j;
    char*   lptr = s;

    i = upcase(*lptr++);
//    if (i > 'Z') i -= 'a' - 'A';        // upcase
    i -= '0';                           // convert to number
    if (i > 9) i -= 'A' - '9' - 1;
    j = upcase(*lptr);
//    if (j > 'Z') j -= 'a' - 'A';        // upcase
    j -= '0';                           // convert to number
    if (j > 9) j -= 'A' - '9' - 1;
    return ((i << 4) | j);
}

///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// upcase() returns upper case of c
//-----------------------------------------------------------------------------
char upcase(char c) {
    char    i = c;     // temp

    if ((c >= 'a') && (c <= 'z')) i = c - ('a' - 'A');        // upcase
    return (i);
}

///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// get_serial() gathers serial input up to timeout.  Displays to console window
//-----------------------------------------------------------------------------
void get_serial(char ipls) {
    char    i = 0;     // temps
    char    j;
    char    tilde = 0;
    long    timer = 1;
    char    qbuf[255];
    static char srow;

    if (ipls) {
        srow = SERIAL_ROW;
    }else {
        timer_ms(200, true);
        do {
            if (serialByteIn(&j)) {
                if (tilde) {
                    qbuf[i++] = j;
                }
                if (j == '~') {
                    tilde = 1;
                }
                if (((j == '\n') || (j == '.')) && (tilde)) {
                    timer = 0;
                }
            }
        } while (timer_ms(0, false) && (timer) && (i < 60));
        qbuf[i] = '\0';
        gopos(srow++, 1);
        set_text_color(LABEL_COLOR, BACK_COLOR);
        _cputs(qbuf);
        clr_to_eol();
        if (srow > BOTTOM_ROW) {
            srow = SERIAL_ROW;
        }
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// wait_ms() performs a "lock-down" (nothing else happens) delay.
//  "delay" is in ms.  Upper limit is 999, but a practical limit will be much less.
//      Since the ftime64 resource rolls over at 1000, delay values close to
//      1000 risk latencies causing overrun.  Unfortunately, establishing metrics
//      for such things is difficult, so one must simple guess at an upper limit.
//      750 is my current (first) guess...
//-----------------------------------------------------------------------------

void wait_ms(short int delay) {
    __timeb64        curr_time_sec;      // current time
    unsigned short   ct;                 // current time
    unsigned short   t;                  // start time
    unsigned short   dt;                 // end (delta) time
    bool             loop = true;        // loop flag

    _ftime64_s(&curr_time_sec);
    t = curr_time_sec.millitm;
    dt = t + delay;
    do {
        _ftime64_s(&curr_time_sec);
        ct = curr_time_sec.millitm;
        if (dt > t){
            if ((ct > dt) || (ct < t)) loop = false;
        }else {
            if ((ct > dt) && (ct < t)) loop = false;
        }
    } while (loop);
    return;
}

///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// timer_ms() sets or tests a ms timer.
//  "delay" is in ms.  Upper limit is 999.  This code depends on the ms time and
//      running seconds count being synchronized in the WIN OS.  No idea if this
//      is true yet...
//
//  Returns FALSE if timer is expired, TRUE if running.
//
//  Call with delay value and SET = true to start timer.
//  Call with delay = "don't care" and SET = false to check timer.
//  Overrun returns as expired timer, but no information is passed to determine
//      the degree of overrun.
//-----------------------------------------------------------------------------

bool timer_ms(short int delay, bool set) {
    __timeb64               curr_time_sec;      // current time
    static time_t           st;                 // modified "julean" sec
    static unsigned short   t;                  // start time (ms)
    static unsigned short   dt;                 // end (delta) time (ms)
    unsigned short          ct;                 // current time (ms)
    bool                    loop = true;        // loop flag

    _ftime64_s(&curr_time_sec);
    if (set) {
        st = curr_time_sec.time;                                // save running seconds
        t = curr_time_sec.millitm;                              // save running ms
        if (delay > 999) {
            dt = t + 999;                                       // force end ms to max
        }else {
            dt = t + delay;                                     // calc end ms
        }
        if (dt > 999) {
            st += 1;
            dt = dt - 1000;
        }
    }else {
        ct = curr_time_sec.millitm;
        if (((curr_time_sec.time == st) && (ct > dt)) || (curr_time_sec.time > st)) {
            loop = false;                                       // overrun, timer expired
        }else {                                                 // process the ms timer to see if timer expired...
            if (curr_time_sec.time == st) {
                if (dt > t) {
                    if ((ct > dt) || (ct < t)) loop = false;
                }
                else {
                    if ((ct > dt) && (ct < t)) loop = false;
                }
            }
        }
    }
    return loop;
}

///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
// alive_timer_ms() is a ms timer dedicated to "I'm alive" status of the target device.
//  "delay" is in sec.
//
//  Returns FALSE if timer is expired, TRUE if running.
//
//  Call with delay value and SET = true to start timer.
//  Call with delay = "don't care" and SET = false to check timer.
//-----------------------------------------------------------------------------

bool alive_timer_sec(char delay, bool set) {
    __timeb64       curr_time_sec;      // current time
    static time_t   dt;                 // end (delta) time (sec)
    time_t          ct;                 // current time (sec)
    bool            loop = true;        // loop flag

    _ftime64_s(&curr_time_sec);
    if (set) {
        dt = curr_time_sec.time + (time_t)delay;            // save running seconds
    }else {
        ct = curr_time_sec.time;
        if (ct > dt) loop = false;
    }
    return loop;
}

