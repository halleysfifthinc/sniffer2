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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "timer.h"
#include "delay2.h"
#include "avclandrv.h"
#include "com232.h"
#include "const.h"


//------------------------------------------------------------------------------

#define AVC_OUT_EN()	sbi(PORTD, 6); sbi(DDRD, 6);  sbi(DDRD, 7); sbi(ACSR, ACD); 
#define AVC_OUT_DIS()	cbi(PORTD, 6); cbi(DDRD, 6);  cbi(DDRD, 7); cbi(ACSR, ACD);
#define AVC_SET_1()  	sbi(PORTD, 6);
#define AVC_SET_0()  	cbi(PORTD, 6);


u08 CD_ID_1;
u08 CD_ID_2;

u08 HU_ID_1;
u08 HU_ID_2;

u08 parity_bit;

u08 repeatMode;
u08 randomMode;

u08 playMode;

u08 cd_Disc;
u08 cd_Track;
u08 cd_Time_Min;
u08 cd_Time_Sec;

u08 answerReq;

// we need check answer (to avclan check) timeout
// when is more then 1 min, FORCE answer.
u08 check_timeout;

#define SW_ID	0x12

// commands
const u08 stat1[]		= { 0x4,	0x00, 0x00, 0x01, 0x0A };
const u08 stat2[]		= { 0x4,	0x00, 0x00, 0x01, 0x08 };
const u08 stat3[]		= { 0x4,	0x00, 0x00, 0x01, 0x0D };
const u08 stat4[] 		= { 0x4,	0x00, 0x00, 0x01, 0x0C };

// broadcast
const u08 lan_stat1[]	= { 0x3,	0x00, 0x01, 0x0A };
const u08 lan_reg[]		= { 0x3,	SW_ID, 0x01, 0x00 };
const u08 lan_init[]	= { 0x3,	SW_ID, 0x01, 0x01 };
const u08 lan_check[]	= { 0x3,	SW_ID, 0x01, 0x20 };
const u08 lan_playit[]	= { 0x4,	SW_ID, 0x01, 0x45, 0x63 };



const u08 play_req1[]	= { 0x4,	0x00, 0x25, 0x63, 0x80 };

#ifdef __AVENSIS__
	const u08 play_req2[]	= { 0x6,	0x00, SW_ID, 0x63, 0x42 };
#else
	const u08 play_req2[]	= { 0x6,	0x00, SW_ID, 0x63, 0x42, 0x01, 0x00 };
#endif

const u08 play_req3[]	= { 0x6,	0x00, SW_ID, 0x63, 0x42, 0x41, 0x00 };
const u08 stop_req[]	= { 0x5,	0x00, SW_ID, 0x63, 0x43, 0x01 };
const u08 stop_req2[]	= { 0x5,	0x00, SW_ID, 0x63, 0x43, 0x41 };


// answers
const u08	CMD_REGISTER[]	= {0x1,		0x05,	0x00, 0x01,	SW_ID, 0x10, 0x63 };
const u08	CMD_STATUS1[]	= {0x1,		0x04,	0x00, 0x01, 0x00, 0x1A };
const u08	CMD_STATUS2[]	= {0x1,		0x04,	0x00, 0x01, 0x00, 0x18 };
const u08	CMD_STATUS3[]	= {0x1,		0x04,	0x00, 0x01, 0x00, 0x1D };
const u08	CMD_STATUS4[]	= {0x1,		0x05,	0x00, 0x01, 0x00, 0x1C, 0x00 };
u08			CMD_CHECK[]		= {0x1,		0x06,	0x00, 0x01, SW_ID, 0x30, 0x00, 0x00 };

const u08	CMD_STATUS5[]	= {0x1,		0x05,	0x00, 0x5C, 0x12, 0x53, 0x02 };
const u08	CMD_STATUS5A[]	= {0x0,		0x05,	0x5C, 0x31, 0xF1, 0x00, 0x00 };

const u08	CMD_STATUS6[]	= {0x1,		0x06,	0x00, 0x5C, 0x32, 0xF0, 0x02, 0x00 };


const u08	CMD_PLAY_OK1[]	= {0x1,		0x05,	0x00, 0x63, SW_ID, 0x50, 0x01 };
const u08	CMD_PLAY_OK2[]	= {0x1,		0x05,	0x00, 0x63, SW_ID, 0x52, 0x01 };
const u08	CMD_PLAY_OK3[]	= {0x0,		0x0B,	0x63, 0x31, 0xF1, 0x01, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x00, 0x80 };
u08			CMD_PLAY_OK4[]	= {0x0,		0x0B,	0x63, 0x31, 0xF1, 0x01, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80 };

const u08	CMD_STOP1[]		= {0x1,		0x05,	0x00, 0x63, SW_ID, 0x53, 0x01 };
u08			CMD_STOP2[]		= {0x0,		0x0B,	0x63, 0x31, 0xF1, 0x00, 0x30, 0x00, 0x00,0x00, 0x00, 0x00, 0x80 };

const u08	CMD_BEEP[]		= {0x1,		0x05,	0x00, 0x63, 0x29, 0x60, 0x02 };

//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
void AVC_HoldLine()
{
 STOPEvent;

 // wait for free line
 u08 T=0;
 u08 line_busy = 1;

 timer0_source(CK64);
 timer0_start();
 do {
 	while (INPUT_IS_CLEAR) {
		T = TCNT0;
		if (T >= 25) break;
 	}
 	if (T > 24) line_busy=0;
 } while (line_busy);

 // switch to out mode
 AVC_OUT_EN();
 AVC_SET_1();

 STARTEvent;
}
//------------------------------------------------------------------------------
void AVC_ReleaseLine()
{
 AVC_SET_0();
 AVC_OUT_DIS();
}
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
void AVCLan_Init()
{
 // AVCLan TX+/TX- 	 : internal comparator PINB2, PINB3
 
  
 // OUTPUT ( set as input for comparator )
 cbi(PORTD, 6);
 cbi(DDRD, 6);

 // INPUT
 cbi(PORTD, 7);
 cbi(DDRD, 7);

 // Analog comparator
 
 cbi(ADCSRB, ACME);	// Analog Comparator Multiplexer Enable - NO
/*
 cbi(ACSR, ACBG);	// Analog Comparator Bandgap Select
 					// ACI: Analog Comparator Interrupt Flag

 cbi(ACSR, ACIE);	// Analog Comparator Interrupt Enable - NO
 cbi(ACSR, ACIC);	// Analog Comparator Input Capture Enable - NO
*/
 cbi(ACSR, ACIS1);	// Analog Comparator Interrupt Mode Select
 cbi(ACSR, ACIS0);  // Comparator Interrupt on Output Toggle

 cbi(ACSR, ACD); 	// Analog Comparator Disbale - NO


 message_len   = 0;
 answerReq     = cmNull;
 check_timeout = 0;

 cd_Disc = 1;
 cd_Track = 1;
 cd_Time_Min = 0;
 cd_Time_Sec = 0;
 repeatMode = 0;
 randomMode = 0;
 playMode   = 0;
 CD_Mode    = stStop;

}
//------------------------------------------------------------------------------
u08 AVCLan_Read_Byte(u08 length)
{
 u08 byte = 0;
 u08 wT;
 
 while (1) {
   while (INPUT_IS_CLEAR);
   timer0_start();
   while (INPUT_IS_SET);
   wT = TCNT0;
   if (wT<8) { 
      byte++;
	  parity_bit++;
   }
   length--;
   if (!length) return byte;
   byte = byte << 1;
 } 
}
//------------------------------------------------------------------------------

u08 AVCLan_Send_StartBit()
{
 AVC_SET_1();
 delay1(166);
 
 AVC_SET_0();
 delay1(30);

 return 1;
}
//------------------------------------------------------------------------------
void AVCLan_Send_Bit1()
{
 AVC_SET_1();
 delay1(20);

 AVC_SET_0();
 delay1(16);							// 12-21
}
//------------------------------------------------------------------------------
void AVCLan_Send_Bit0()
{
 AVC_SET_1();
 delay1(32);							// 28-37
	
 AVC_SET_0();
 delay1(4);								// 00-09
}
//------------------------------------------------------------------------------
u08 AVCLan_Read_ACK()
{
 u08 time = 0;

 AVC_SET_1();
 delay1(19);

 AVC_SET_0();
 delay1(1);


 AVC_OUT_DIS(); // switch to read mode
 timer0_source(CK64);
 timer0_start();
 while(1) {
	time = TCNT0;
	if (INPUT_IS_SET && (time > 1)) break;
	if (time > 20) return 1;
 }
	
 while(INPUT_IS_SET);
 AVC_OUT_EN();// back to write mode
 return 0;

}
//------------------------------------------------------------------------------
u08 AVCLan_Send_ACK()
{	
 timer0_source(CK64);						//update every 1us
 timer0_start();	
 while (INPUT_IS_CLEAR)	{
 	if (TCNT0 >= 25) return 0;			// max wait time
 }

 AVC_OUT_EN();

 AVC_SET_1();
 delay1(32);								//28-37

 AVC_SET_0();
 delay1(4);									//00-09

 AVC_OUT_DIS();

 return 1;		
}
//------------------------------------------------------------------------------
u08 AVCLan_Send_Byte(u08 byte, u08 len)
{
 u08 b;
 if (len==8) {
 	b = byte;
 } else {
    b = byte << (8-len);
 }

 while (1) {
   if ( (b & 128)!=0 ) {
     AVCLan_Send_Bit1();
	 parity_bit++;
   } else { 
   	 AVCLan_Send_Bit0();
   }
   len--;
   if (!len) { 
     //if (INPUT_IS_SET) RS232_Print("SBER\n"); // Send Bit ERror
     return 1;
   }
   b = b << 1;
 } 

}
//------------------------------------------------------------------------------
u08 AVCLan_Send_ParityBit()
{
 if ( (parity_bit & 1)!=0 ) {
     AVCLan_Send_Bit1();
	 //parity_bit++;
 } else {
   	 AVCLan_Send_Bit0();
 }
 parity_bit=0;
 return 1;
}
//------------------------------------------------------------------------------
u08 CheckCmd(u08 *cmd)
{
 u08 i;
 u08 *c;
 u08 l;

 c = cmd;
 l = *c++;

 for (i=0; i<l; i++) {
 	if (message[i] != *c) return 0;
	c++;
 }
 return 1;
}
//------------------------------------------------------------------------------
u08 AVCLan_Read_Message()
{
 STOPEvent;						// disable timer1 interrupt

 u08 T = 0;

 u08 i;
 u08 for_me = 0;

 //RS232_Print("$ ");
 timer0_source(CK64);

 // check start bit
 timer0_start();
 while (INPUT_IS_SET) { 
 	T=TCNT0;
	if (T>254) {
		STARTEvent;
		RS232_Print("LAN>T1\n");
		return 0;
	}
 }


 if (T<10) {		// !!!!!!! 20 !!!!!!!!!!!
 	STARTEvent;
	RS232_Print("LAN>T2\n");
	return 0;
 }



 broadcast = AVCLan_Read_Byte(1);

 parity_bit = 0;
 master1 = AVCLan_Read_Byte(4);
 master2 = AVCLan_Read_Byte(8);
 if ((parity_bit&1)!=AVCLan_Read_Byte(1)) {
	STARTEvent;
	return 0;
 }

 parity_bit = 0;
 slave1 = AVCLan_Read_Byte(4);
 slave2 = AVCLan_Read_Byte(8);
 if ((parity_bit&1)!=AVCLan_Read_Byte(1)) {
	STARTEvent;
	return 0;
 }
 // is this command for me ?
 if ((slave1==CD_ID_1)&&(slave2==CD_ID_2)) {
 	for_me=1;
 }

 if (for_me) AVCLan_Send_ACK();
 		else AVCLan_Read_Byte(1);

 parity_bit = 0;
 AVCLan_Read_Byte(4);	// control - always 0xF
 if ((parity_bit&1)!=AVCLan_Read_Byte(1)) {
	STARTEvent;
	return 0;
 }
 if (for_me) AVCLan_Send_ACK();
 		else AVCLan_Read_Byte(1);

 parity_bit = 0;
 message_len = AVCLan_Read_Byte(8);
 if ((parity_bit&1)!=AVCLan_Read_Byte(1)) {
	STARTEvent;
	return 0;
 }
 if (for_me) AVCLan_Send_ACK();
 		else AVCLan_Read_Byte(1);

 if (message_len > MAXMSGLEN) {
//	RS232_Print("LAN> Command error");
	STARTEvent;
	return 0;
 }

 for (i=0; i<message_len; i++) {
	parity_bit = 0;
 	message[i] = AVCLan_Read_Byte(8);
	if ((parity_bit&1)!=AVCLan_Read_Byte(1)) {
		STARTEvent;
		return 0;
 	}
	if (for_me) {
		AVCLan_Send_ACK();
 	} else {
		AVCLan_Read_Byte(1);
	}
 }


 STARTEvent;

 if (showLog) ShowInMessage();

 if (for_me) {
 	
	if (CheckCmd((u08*)stat1)) { answerReq = cmStatus1; return 1; }
	if (CheckCmd((u08*)stat2)) { answerReq = cmStatus2; return 1; }
	if (CheckCmd((u08*)stat3)) { answerReq = cmStatus3; return 1; }
	if (CheckCmd((u08*)stat4)) { answerReq = cmStatus4; return 1; }
//	if (CheckCmd((u08*)stat5)) { answerReq = cmStatus5; return 1; }

	if (CheckCmd((u08*)play_req1)) { answerReq = cmPlayReq1; return 1; }
	if (CheckCmd((u08*)play_req2)) { answerReq = cmPlayReq2; return 1; }
	if (CheckCmd((u08*)play_req3)) { answerReq = cmPlayReq3; return 1; }
	if (CheckCmd((u08*)stop_req))  { answerReq = cmStopReq;  return 1; }
	if (CheckCmd((u08*)stop_req2)) { answerReq = cmStopReq2; return 1; }

 } else { // broadcast check

	if (CheckCmd((u08*)lan_playit))	{ answerReq = cmPlayIt;	return 1; }
	if (CheckCmd((u08*)lan_check))	{ 
			answerReq = cmCheck;
			CMD_CHECK[6]=message[3];
			return 1; 
	}
	if (CheckCmd((u08*)lan_reg))	{ answerReq = cmRegister;	return 1; }
	if (CheckCmd((u08*)lan_init))	{ answerReq = cmInit;		return 1; }
	if (CheckCmd((u08*)lan_stat1))	{ answerReq = cmStatus1;	return 1; }


 }
 answerReq = cmNull;
 return 1;
}
//------------------------------------------------------------------------------
u08 AVCLan_SendData()
{
 u08 i;

 STOPEvent;

 // wait for free line
 u08 T=0;
 u08 line_busy = 1;

 timer0_source(CK64);
 timer0_start();
 do {
 	while (INPUT_IS_CLEAR) {
		T = TCNT0;
		if (T >= 25) break;
 	}
 	if (T > 24) line_busy=0;
 } while (line_busy);


 // switch to output mode
 AVC_OUT_EN();

 AVCLan_Send_StartBit();
 AVCLan_Send_Byte(0x1,  1);		// regular communication


 parity_bit = 0;
 AVCLan_Send_Byte(CD_ID_1, 4);	// CD Changer ID as master
 AVCLan_Send_Byte(CD_ID_2, 8);
 AVCLan_Send_ParityBit();

 AVCLan_Send_Byte(HU_ID_1, 4);	// HeadUnit ID as slave
 AVCLan_Send_Byte(HU_ID_2, 8);

 AVCLan_Send_ParityBit();

 if (AVCLan_Read_ACK()) {
 	 AVC_OUT_DIS();
	 STARTEvent;
	 RS232_Print("E1\n");
	 return 1;
 }


 AVCLan_Send_Byte(0xF, 4);		// 0xf - control -> COMMAND WRITE
 AVCLan_Send_ParityBit();
 if (AVCLan_Read_ACK()) {
 	 AVC_OUT_DIS();
	 STARTEvent;
	 RS232_Print("E2\n");
	 return 2;
 }

 AVCLan_Send_Byte(data_len,  8);// data lenght
 AVCLan_Send_ParityBit();
 if (AVCLan_Read_ACK()) {
 	 AVC_OUT_DIS();
	 STARTEvent;
	 RS232_Print("E3\n");
	 return 3;
 }

 for (i=0;i<data_len;i++) {
	AVCLan_Send_Byte(data[i], 8);// data byte
 	AVCLan_Send_ParityBit();
 	if (AVCLan_Read_ACK()) {
	 	 AVC_OUT_DIS();
		 STARTEvent;
 		 RS232_Print("E4(");
		 RS232_PrintDec(i);
		 RS232_Print(")\n");
		 return 4;
 	}
 }

 // back to read mode
 AVC_OUT_DIS();

 STARTEvent;
 if (showLog) ShowOutMessage();
 return 0;
}
//------------------------------------------------------------------------------
u08 AVCLan_SendDataBroadcast()
{
 u08 i;

 STOPEvent;

 // wait for free line
 u08 T=0;
 u08 line_busy = 1;

 timer0_source(CK64);
 timer0_start();
 do {
 	while (INPUT_IS_CLEAR) {
		T = TCNT0;
		if (T >= 25) break;
 	}
 	if (T > 24) line_busy=0;
 } while (line_busy);


 AVC_OUT_EN();

 AVCLan_Send_StartBit();
 AVCLan_Send_Byte(0x0,  1);		// broadcast

 parity_bit = 0;
 AVCLan_Send_Byte(CD_ID_1, 4);	// CD Changer ID as master
 AVCLan_Send_Byte(CD_ID_2, 8);
 AVCLan_Send_ParityBit();

 AVCLan_Send_Byte(0x1, 4);		// all audio devices
 AVCLan_Send_Byte(0xFF, 8);
 AVCLan_Send_ParityBit();
 AVCLan_Send_Bit1();

 AVCLan_Send_Byte(0xF, 4);		// 0xf - control -> COMMAND WRITE
 AVCLan_Send_ParityBit();
 AVCLan_Send_Bit1();
 
 AVCLan_Send_Byte(data_len,  8);	// data lenght
 AVCLan_Send_ParityBit();
 AVCLan_Send_Bit1();

 for (i=0;i<data_len;i++) {
	AVCLan_Send_Byte(data[i], 8); // data byte
 	AVCLan_Send_ParityBit();
	AVCLan_Send_Bit1();
 }

 AVC_OUT_DIS();
 STARTEvent;
 if (showLog) ShowOutMessage();
 return 0;
}
//------------------------------------------------------------------------------
u08 AVCLan_SendAnswerFrame(u08 *cmd)
{
 u08 i;
 u08 *c;
 u08 b;

 c = cmd;
 
 b = *c++;
 data_control = 0xF;
 data_len	 = *c++;
 for (i=0; i<data_len; i++) {
 	data[i]= *c++;
 }
 if (b)
 	return AVCLan_SendData();
 else 
 	return AVCLan_SendDataBroadcast();
}
//------------------------------------------------------------------------------
u08 AVCLan_SendMyData(u08 *data_tmp, u08 s_len)
{
 u08 i;
 u08 *c;
 
 c = data_tmp;
 
 data_control = 0xF;
 data_len	 = s_len;
 for (i=0; i<data_len; i++) {
 	data[i]= *c++;
 }
 return AVCLan_SendData();
}
//------------------------------------------------------------------------------
u08 AVCLan_SendMyDataBroadcast(u08 *data_tmp, u08 s_len)
{
 u08 i;
 u08 *c;
 

 c = data_tmp;
 
 data_control = 0xF;
 data_len	 = s_len;
 for (i=0; i<data_len; i++) {
 	data[i]= *c++;
 }
 return AVCLan_SendDataBroadcast();
}
//------------------------------------------------------------------------------
u08 AVCLan_SendInitCommands()
{
 u08 r;

 const u08 c1[] = { 0x0, 0x0B,		0x63, 0x31, 0xF1, 0x00, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x80 };
 const u08 c2[] = { 0x0, 0x0A,		0x63, 0x31, 0xF3, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x02 };
 const u08 c3[] = { 0x0, 0x0A,		0x63, 0x31, 0xF3, 0x00, 0x3F, 0x00, 0x01, 0x00, 0x01, 0x02 };
 const u08 c4[] = { 0x0, 0x0A,		0x63, 0x31, 0xF3, 0x00, 0x3D, 0x00, 0x01, 0x00, 0x01, 0x02 };
 const u08 c5[] = { 0x0, 0x0A,		0x63, 0x31, 0xF3, 0x00, 0x39, 0x00, 0x01, 0x00, 0x01, 0x02 };
 const u08 c6[] = { 0x0, 0x0A,		0x63, 0x31, 0xF3, 0x00, 0x31, 0x00, 0x01, 0x00, 0x01, 0x02 };
 const u08 c7[] = { 0x0, 0x0A,		0x63, 0x31, 0xF3, 0x00, 0x21, 0x00, 0x01, 0x00, 0x01, 0x02 };
 const u08 c8[] = { 0x0, 0x0B,		0x63, 0x31, 0xF1, 0x00, 0x90, 0x01, 0xFF, 0xFF, 0xFF, 0x00, 0x80 };
 const u08 c9[] = { 0x0, 0x0A,		0x63, 0x31, 0xF3, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x02 };
 const u08 cA[] = { 0x0, 0x0B,		0x63, 0x31, 0xF1, 0x00, 0x30, 0x01, 0xFF, 0xFF, 0xFF, 0x00, 0x80 };

 r = AVCLan_SendAnswerFrame((u08*)c1);
 if (!r) r = AVCLan_SendAnswerFrame((u08*)c2);
 if (!r) r = AVCLan_SendAnswerFrame((u08*)c3);
 if (!r) r = AVCLan_SendAnswerFrame((u08*)c4);
 if (!r) r = AVCLan_SendAnswerFrame((u08*)c5);
 if (!r) r = AVCLan_SendAnswerFrame((u08*)c6);
 if (!r) r = AVCLan_SendAnswerFrame((u08*)c7);
 if (!r) r = AVCLan_SendAnswerFrame((u08*)c8);
 if (!r) r = AVCLan_SendAnswerFrame((u08*)c9);
 if (!r) r = AVCLan_SendAnswerFrame((u08*)cA);

 //const u08 c1[] = { 0x0, 0x0B,		0x63, 0x31, 0xF1, 0x00, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x80 };
 //r = AVCLan_SendAnswerFrame((u08*)c1);
 return r;
}
//------------------------------------------------------------------------------
void AVCLan_Send_Status()
{
//                                                        disc  track t_min t_sec
 u08 STATUS[] = {0x0, 0x0B, 0x63, 0x31, 0xF1, 0x01, 0x10, 0x01, 0x01, 0x00, 0x00, 0x00, 0x80 };
	
 STATUS[7] =  cd_Disc;
 STATUS[8] =  cd_Track;
 STATUS[9] =  cd_Time_Min;
 STATUS[10] = cd_Time_Sec;

 STATUS[11] = 0;

 AVCLan_SendAnswerFrame((u08*)STATUS);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
u08 AVCLan_SendAnswer()
{
 u08 r = 0 ;
 
 switch (answerReq) {
 	case cmStatus1:		r = AVCLan_SendAnswerFrame((u08*)CMD_STATUS1); 
						break;
 	case cmStatus2:		r = AVCLan_SendAnswerFrame((u08*)CMD_STATUS2); 
						break;
 	case cmStatus3:		r = AVCLan_SendAnswerFrame((u08*)CMD_STATUS3); 
						break;
 	case cmStatus4:		r = AVCLan_SendAnswerFrame((u08*)CMD_STATUS4); 
						break;
 	case cmRegister:	r = AVCLan_SendAnswerFrame((u08*)CMD_REGISTER); 
						break;
 	case cmInit:		//RS232_Print("INIT\n");
						r = AVCLan_SendInitCommands(); 
						break;
 	case cmCheck:		r = AVCLan_SendAnswerFrame((u08*)CMD_CHECK); 
						check_timeout = 0;
						CMD_CHECK[6]++;
 						RS232_Print("AVCCHK\n");
						break;
 	case cmPlayReq1:	playMode = 0;
						r = AVCLan_SendAnswerFrame((u08*)CMD_PLAY_OK1); 
						break;
 	case cmPlayReq2:	
	case cmPlayReq3:	playMode = 0;
						r = AVCLan_SendAnswerFrame((u08*)CMD_PLAY_OK2); 
						if (!r) r = AVCLan_SendAnswerFrame((u08*)CMD_PLAY_OK3);
						CD_Mode = stPlay;
						break;
	case cmPlayIt:		playMode = 1;
						RS232_Print("PLAY\n");
						CMD_PLAY_OK4[7]=cd_Disc;
						CMD_PLAY_OK4[8]=cd_Track;
						CMD_PLAY_OK4[9]=cd_Time_Min;
						CMD_PLAY_OK4[10]=cd_Time_Sec;
						r = AVCLan_SendAnswerFrame((u08*)CMD_PLAY_OK4); 
						if (!r) AVCLan_Send_Status();
						CD_Mode = stPlay;
						break;
	case cmStopReq:
	case cmStopReq2:	CD_Mode = stStop;
						playMode = 0;
						
						r = AVCLan_SendAnswerFrame((u08*)CMD_STOP1); 
						CMD_STOP2[7]=cd_Disc;
						CMD_STOP2[8]=cd_Track;
						CMD_STOP2[9]=cd_Time_Min;
						CMD_STOP2[10]=cd_Time_Sec;
						r = AVCLan_SendAnswerFrame((u08*)CMD_STOP2); 
						break;
	case cmBeep:		AVCLan_SendAnswerFrame((u08*)CMD_BEEP);
						break;
 }

 answerReq = cmNull;
 return r;
}
//------------------------------------------------------------------------------
void AVCLan_Register()
{
 RS232_Print("REG_ST\n");
 AVCLan_SendAnswerFrame((u08*)CMD_REGISTER); 
 RS232_Print("REG_END\n");
 //AVCLan_Command( cmRegister );
 AVCLan_Command( cmInit );
}
//------------------------------------------------------------------------------
u08	 AVCLan_Command(u08 command)
{
 u08 r;

 answerReq = command;
 r = AVCLan_SendAnswer(); 
 /*
 RS232_Print("ret=");
 RS232_PrintHex8(r);
 RS232_Print("\n");
 */
 return r;
}
//------------------------------------------------------------------------------
u08 HexInc(u08 data)
{
 if ((data & 0x9)==0x9) 
 	return (data + 7);
 
 return (data+1);
}
//------------------------------------------------------------------------------
u08 HexDec(u08 data)
{
 if ((data & 0xF)==0) 
 	return (data - 7);
 
 return (data-1);
}
//------------------------------------------------------------------------------
// encode decimal valute to 'toyota' format :-)
//  ex.   42 (dec)   =  0x42 (toy)
u08 Dec2Toy(u08 data)
{
 u08 d,d1;
 d = (u32)data/(u32)10;
 d1 = d * 16;
 d  = d1 + (data - 10*d);
 return d;
}
//------------------------------------------------------------------------------
void ShowInMessage()
{
 if (message_len==0) return;

 AVC_HoldLine();
 

 RS232_S((u16)PSTR("HU < ("));

 if (broadcast==0) RS232_S((u16)PSTR("bro) "));
 else RS232_Print("dir) ");

 RS232_PrintHex4(master1);
 RS232_PrintHex8(master2);
 RS232_Print("| ");
 RS232_PrintHex4(slave1);
 RS232_PrintHex8(slave2);
 RS232_Print("| ");
 
 u08 i;
 for (i=0;i<message_len;i++) {
	RS232_PrintHex8(message[i]);
	RS232_Print(" ");
 }
 RS232_Print("\n");

 AVC_ReleaseLine();
}
//------------------------------------------------------------------------------
void ShowOutMessage()
{
 u08 i;

 AVC_HoldLine();
 
 RS232_S((u16)PSTR("out > "));
 for (i=0; i<data_len; i++) {
	RS232_PrintHex8(data[i]);
	RS232_SendByte(' ');
 }
 RS232_Print("\n");

 AVC_ReleaseLine();
}

//------------------------------------------------------------------------------
