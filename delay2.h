/*
  Copyright (C) 2006 Marcin Slonicki <marcin@softservice.com.pl>.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 -----------------------------------------------------------------------
	this file is a part of the TOYOTA Corolla MP3 Player Project
 -----------------------------------------------------------------------
 		http://www.softservice.com.pl/corolla/avc

 May 28 / 2009	- version 2

*/


#ifndef _delay_h_
#define _delay_h_

#include <inttypes.h>
#include <avr/io.h>
#include "const.h"

/* delay function for microsec
   4 cpu cycles per loop + 1 cycles(?) overhead 
   when a constant is passed. */
static inline void delayloop16(uint16_t count)
{
	asm volatile (  "cp  %A0,__zero_reg__ \n\t"  \
                     "cpc %B0,__zero_reg__ \n\t"  \
                     "breq L_Exit_%=       \n\t"  \
                     "L_LOOP_%=:           \n\t"  \
                     "sbiw %0,1            \n\t"  \
                     "brne L_LOOP_%=       \n\t"  \
                     "L_Exit_%=:           \n\t"  \
                     : "=w" (count)
					 : "0"  (count)
                   );                            
}
// delayloop16(x) eats 4 cycles per x
#define DELAY_US_CONV(us) ((uint16_t)(((((us)*1000L)/(1000000000/F_CPU))-1)/4))
#define delay_us(us)	  delayloop16(DELAY_US_CONV(us))

/* delay function for millisec
  (6 cycles per x + 20(?) overhead) */
void delayloop32( uint32_t l); // not inline
#define DELAY_MS_CONV(ms) ( (uint32_t) (ms*(F_CPU/6000L)) ) 
#define delay_ms(ms)  delayloop32(DELAY_MS_CONV(ms))


void delay1us(volatile u08 delay, u08 count);
void delay1(u16 count);

#endif

