#ifndef PCSERIAL_H_
#define PCSERIAL_H_
/**
 *
 * @file pcSerial.h
 *
 * Serial input and output function declarations.
 *
 */


/**
 * configCommPort configures the PC comm port.
 *
 * Configure the specified com port baud rate, parity, byte size and stop bits.
 *
 * @param[in] port is the com port number (1, 2, etc.).
 * @param[in] baudRate is the desired baud rate.
 * @param[in] parity specifies the desired parity setting.
 *            Use NOPARITY, ODDPARITY, EVENPARITY, MARKPARITY or SPACEPARITY.
 * @param[in] byteSize is the number of bits in each data byte (usually 8).
 * @param[in] stopBits is the number of stop bits. Use ONESTOPBITS, ONE5STOPBITS
 *            (for 1.5 stop bits) or TWOSTOPBITS.
 *
 * @return true if configuration was successful
 */
bool configCommPort(BYTE port, long baudRate, BYTE parity,
                    BYTE byteSize, BYTE stopBits);

/**
 * closeCommPort closes the com port.
 *
 * Call the windows "close handle" function to release the resource.
 *
 * @param[in,out] none.
 *
 * @return void
 */
void closeCommPort(void);

/**
 * sendByte sends one byte over the com port.
 *
 * Write the specified byte to the com port.
 *
 * @param[in] c is the byte to be transmitted.
 *
 * @return void
 */
void sendByte(char c);


/**
 * putss sends string array over the com port.
 *
 * Write the specified byte to the com port.
 *
 * @param[in] string is the pointer to the start of the array.
 *
 * @return void
 */
void putss(char* string);

/**
 * getss recvs string array over the com port.
 *
 * @param[in] string is the pointer to the start of the array.
 *
 * @return end of string
 */
char* getss(char* string);

/**
 * serialByteIn returns one byte from the com port.
 *
 * Read one byte from the com port.
 *
 * @param[out] *c contains the byte read from the serial port when
 *             function returns TRUE.
 *
 * @return TRUE if a byte was read
 */
bool serialByteIn(char *c);

#endif   // PCSERIAL_H_
