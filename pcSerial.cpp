/**
 *
 * @pcSerial.cpp
 *
 * Serial input and output function declarations.
 *
 */


#include "stdafx.h"

#include <windows.h>
#include "pcSerial.h"

static HANDLE hComPort;            // handle for PC com port


// ===========================================================================
//
// configCommPort - documentation in header.
//
// ===========================================================================
bool configCommPort(BYTE port, long baudRate, BYTE parity, BYTE byteSize,
                  BYTE stopBits)
{
    bool success = false;          // return value
    char portName[8];              // com port "name"
    COMMTIMEOUTS commTimeouts;
    DCB  dcb;

    sprintf_s(portName, 8, "COM%d", port);
    hComPort = CreateFile(portName, GENERIC_READ+GENERIC_WRITE, 0,
                          NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if (hComPort != INVALID_HANDLE_VALUE)
    {
        // Configure the COM Port
        dcb.DCBlength = sizeof(dcb);
        if (GetCommState(hComPort, &dcb))
        {
            dcb.BaudRate = baudRate;
            dcb.Parity   = parity;
            dcb.ByteSize = byteSize;
            dcb.StopBits = stopBits;
            dcb.fOutxCtsFlow = 0;
            dcb.fOutxDsrFlow = 0;
            dcb.fDtrControl  = DTR_CONTROL_ENABLE;
            dcb.fRtsControl  = RTS_CONTROL_ENABLE;

            if (SetCommState(hComPort, &dcb))
            {
                // Establish Driver buffer sizes
                //if (SetupComm(hComPort, 1024, 1024))
                if (SetupComm(hComPort, 32768, 1024))
                {
                    if (GetCommTimeouts(hComPort, &commTimeouts))
                    {
                        commTimeouts.ReadIntervalTimeout        = MAXDWORD;
                        commTimeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
                        commTimeouts.ReadTotalTimeoutConstant   = 250;
                        commTimeouts.WriteTotalTimeoutMultiplier  = 0;
                        commTimeouts.WriteTotalTimeoutConstant    = 0;

                        if (SetCommTimeouts(hComPort, &commTimeouts))
                        {
                            success = true;
                        }
                        else
                        {
                            printf("Set CommTimeouts() failed.\n");
                        }
                    }
                    else
                    {
                       printf("Get CommTimeouts() failed.\n");
                    }
                }
                else
                {
                   printf("SetupComm() failed.\n");
                }
            }
            else
            {
                printf("SetCommState() failed.\n");
            }
        }
        else
        {
            printf("GetCommState() failed.\n");
        }
    }
    else
    {
        printf("COM Port open failed (%s).\n", portName);
    }
    return(success);
}


// ===========================================================================
//
// closeCommPort - documentation in header.
//
// ===========================================================================
void closeCommPort(void)
{
    CloseHandle(hComPort);
}


// ===========================================================================
//
// sendByte - documentation in header.
//
// ===========================================================================
void sendByte(char c)
{
    DWORD bytesOut = 0;

    WriteFile(hComPort, &c, 1, &bytesOut, NULL);
}


// ===========================================================================
//
// serialByteIn - documentation in header.
//
// ===========================================================================
bool serialByteIn(char *c)
{
    DWORD numBytesIn = 0;          // number of bytes received
    bool  byteReceived = false;    // return value

    if (ReadFile(hComPort, c, 1, &numBytesIn, NULL))
    {
        if (numBytesIn == 1)
        {
            byteReceived = true;
        }
    }
    return(byteReceived);
}


//-----------------------------------------------------------------------------
// putss() does puts w/o newline
//-----------------------------------------------------------------------------

void putss(char* string)
{
    char* sptr = string;

    while (*sptr) {
        if (*sptr == '\n') sendByte('\r');
        sendByte(*sptr++);
    }
    return;
}

//-----------------------------------------------------------------------------
// getss() puts data into an array (via a passed pointer) until '\n' is encountered
//-----------------------------------------------------------------------------

char* getss(char* string)
{
    char* sptr = string;
    char    ch = '\0';
    bool    ischr;

    do {
        ischr = serialByteIn(&ch);
        if (ischr) {
            *sptr++ = ch;
        }

    } while (ch != '\n');
    *sptr = '\0';
    return sptr;
}



