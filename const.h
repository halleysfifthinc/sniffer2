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

#ifndef __CONST_H
#define __CONST_H
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// variable type
#define u08	unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long


// port set/unset

#define sbi(port, bit) (port) |= (1 << (bit))
#define cbi(port, bit) (port) &= ~(1 << (bit))

//------------------------------------------------------------------------------


// max 10 events in fifo
extern u08 EventCount;
extern u08 EventCmd[10];
extern u08 Event;

#define EV_NOTHING	0
#define EV_DISPLAY	1
#define EV_STATUS	4

//------------------------------------------------------------------------------

// const
#define smYear		1
#define smMonth		2
#define smDay		3
#define smHour		4
#define smMin		5
#define smWDay		6



//#define STOPEvent  cbi(TIMSK, TOIE1); cbi(UCSRB, RXCIE);
//#define STARTEvent sbi(TIMSK, TOIE1); sbi(UCSRB, RXCIE);


extern u08 showLog;
extern u08 showLog2;

//------------------------------------------------------------------------------
#endif
