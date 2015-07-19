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


#ifndef __AVCLANDRV_H
#define __AVCLANDRV_H
//------------------------------------------------------------------------------
#include "const.h"


#define INPUT_IS_SET (ACSR & _BV(ACO))
#define INPUT_IS_CLEAR (!(ACSR & _BV(ACO)))


#define STOPEvent  cbi(TIMSK1, TOIE1); cbi(UCSR0B, RXCIE0);
#define STARTEvent sbi(TIMSK1, TOIE1); sbi(UCSR0B, RXCIE0);


#define CHECK_AVC_LINE		if (INPUT_IS_SET) AVCLan_Read_Message();

void AVC_HoldLine();
void AVC_ReleaseLine();

#define MAXMSGLEN	32

// Head Unid ID
extern u08 HU_ID_1;		//	0x01
extern u08 HU_ID_2;		//	0x40

extern u08 CD_ID_1;		// 0x03
extern u08 CD_ID_2;		// 0x60


// DVD CHANGER
//#define CD_ID_1	0x02
//#define CD_ID_2	0x50

#define cmNull		0
#define cmStatus1	1
#define cmStatus2	2
#define cmStatus3	3
#define cmStatus4	4


#define cmRegister		100
#define cmInit			101
#define cmCheck			102
#define cmPlayIt		103
#define cmBeep			110

#define cmNextTrack		120
#define cmPrevTrack		121
#define cmNextDisc		122
#define cmPrevDisc		123

#define cmScanModeOn	130
#define cmScanModeOff	131

#define cmPlayReq1	5
#define cmPlayReq2	6
#define cmPlayReq3	7
#define cmStopReq	8
#define cmStopReq2	9

typedef enum { stStop=0, stPlay=1 } cd_modes;
cd_modes CD_Mode;


u08 broadcast;
u08 master1;
u08 master2;
u08 slave1;
u08 slave2;
u08 message_len;
u08 message[MAXMSGLEN];

u08 data_control;
u08 data_len;
u08 data[MAXMSGLEN];

u08 AVCLan_Read_Message();
void AVCLan_Send_Status();

void AVCLan_Init();
void AVCLan_Register();
u08  AVCLan_SendData();
u08  AVCLan_SendAnswer();
u08  AVCLan_SendDataBroadcast();
u08	 AVCLan_Command(u08 command);

u08  HexInc(u08 data);
u08  HexDec(u08 data);
u08  Dec2Toy(u08 data);

extern u08 check_timeout;

extern u08 cd_Disc;
extern u08 cd_Track;
extern u08 cd_Time_Min;
extern u08 cd_Time_Sec;

extern u08 playMode;


u08 AVCLan_SendMyData(u08 *data_tmp, u08 s_len);
u08 AVCLan_SendMyDataBroadcast(u08 *data_tmp, u08 s_len);

void ShowInMessage();
void ShowOutMessage();

//------------------------------------------------------------------------------
extern u08 answerReq;
//------------------------------------------------------------------------------
#endif
