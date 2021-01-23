#pragma once

#define ALIVE_TIME	4					// alive timeout
#define SET_TIMER	1					// set timer cmd
#define READ_TIMER	0					// read timer cmd

// PLL calculation defines
// UX band module defines
#define BANDOFF 0x00
#define ID10M	0x01					// ordinal ID numbers
#define ID6M	0x02
#define ID2M	0x03
#define ID220	0x04
#define ID440	0x05
#define ID1200	0x06

// RX base freqs (KHz)
#define BASE_RX_10M		28000L
#define BASE_RX_6M		40000L
#define BASE_RX_2M		136000L			// bit7 skip
#define BASE_RX_220		220000L			// bit7 skip
#define BASE_RX_440		400000L
#define BASE_RX_1200	1200000L		// bit7 skip
// TX offset freqs (5KHz channels)
#define BASE_TX_10M		(10695L/5L)		// (-)
#define BASE_TX_6M		(13990L/5L)		// (-)
#define BASE_TX_2M		(17200L/5L)		// (+)
#define BASE_TX_220		(17200L/5L)		// (+) 
#define BASE_TX_440		(23150L/5L)		// (+)
#define BASE_TX_1200	(136600L/10L)	// (+) 1.2G is 10KHz channel
// PLL base bitmaps
#define INIT_PLL_6M		0x01325L			// PLL init pattern for 6M & 10M
#define PLL_10M			0x01e3bL			// must <<1 after adding freq
#define PLL_6M			0x02a2eL			// must <<1 after adding freq
#define PLL_2M			0x05CD0L
#define PLL_220			0x11e70L
#define PLL_440			0x1266AL
#define	PLL_BITLEN		0x14000000L		// PLL xfer bit length
#define	PLL_1200BITLEN	0x04000000L		// 1200 init frames PLL xfer bit length
// the TC9181 PLL IC takes LSB 1st, which requires all of the PLL streams to be bit reversed and MSB aligned
// prior to transmission with the msb-first drivers.
#define INIT_PLL_1200	0xA110L			// PLL init pattern for 1200 (sets ref osc)
#define INIT_PLL_1201	0x00			// "HL" config, 4 bits only
#define INIT_PLL_1202	0x0f			// "GPIO" reset config, 4 bits only
#define INIT_PLL_1203	0x0c			// "GPIO" config, 4 bits only
#define PLL_1200		0x19f64L			// must bit reverse

#define VOL_MASK		0xffffff00
#define SQU_MASK		0xffff00ff
#define SQU_SHIFT		8
#define SQUA_MASK		0xff00ffff
#define SQUA_SHIFT		16
#define BND_MASK		0x00ffffff
#define BND_SHIFT		24
