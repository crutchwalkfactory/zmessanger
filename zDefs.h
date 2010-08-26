//
// Project: zMessanger (zIM)
//
// Version: 0.3
//
// Description: ICQ client for MOTOMAGX Platform.
//				In future planed support XMPP protocol.
//
// Author: Ant-ON <prozanton@gmail.com>, (C) 2009-2010
//

#ifndef ZDEFS_H
#define ZDEFS_H

// Log
#include "config.h"
#include <stdio.h>

#if defined(NEW_PLATFORM)&&!defined(CUTED_PLATFORM)&&!defined(_AllLogInFile)
#define logMes printf
#else
#define logMes qDebug
#endif

//#define logMes(s) std::cout<<s<<std::endl

#define logMes_1
#define logMes_2
#define logMes_3
#if LOG_LAVEL > 0
#undef  logMes_1
#define logMes_1 logMes
#endif
#if LOG_LAVEL > 1
#undef  logMes_2
#define logMes_2 logMes
#endif
#if LOG_LAVEL > 2
#undef  logMes_3
#define logMes_3 logMes
#endif

#ifdef DEBUG_ICQKID2
#define logMes_4 logMes
#else
#define logMes_4
#endif 

// Global
class ZGui;
class zEmotIcons;
class ZMyApplication;

extern ZGui * zgui;
extern zEmotIcons * zSmile;
extern ZMyApplication * app;
extern QString ProgDir;

extern QString cfg_inetProfile;
extern bool cfg_alertVibr;
extern bool cfg_alertRing;
extern int cfg_alertRingVol;
extern QString cfg_alertPath;
extern int cfg_timeKeepConnect;
extern int cfg_maxNumLines;
extern int cfg_sortType;
extern bool cfg_dontShowOffLine;
extern bool cfg_dontShowGroup;
extern bool cfg_rigthAlignXStatus;
extern bool cfg_notSendTypeMes;
extern bool cfg_InetOnUSB;
extern int cfg_chatFontSize;
extern int cfg_mesFontSize;
extern bool cfg_sendByCenter; 
extern bool cfg_noShowStatusBarInChat; 
extern QString oldMes;//For save old typed text
extern int oldSmile;//For save old smile position    
extern long int trafIN;
extern long int trafOUT;

// Key def
#define Z6KEY_LEFT              0x1012
#define Z6KEY_UP                0x1013
#define Z6KEY_RIGHT             0x1014
#define Z6KEY_DOWN              0x1015
#define Z6KEY_SIDE_SELECT       0x1005
#define Z6KEY_SIDE_UP           0x1016
#define Z6KEY_SIDE_DOWN         0x1017
#define Z6KEY_GREEN             0x1030
#define Z6KEY_C                 0x1032
#define Z6KEY_MUSIC             0x1048
#define Z6KEY_CENTER            0x1004
#define Z6KEY_RED               0x1031
#define Z6KEY_CARRIER           0x1033
#define Z6KEY_POUND             0x23
#define Z6KEY_LSK               0x1038
#define Z6KEY_RSK               0x103a

// For conact lis
enum CONTACT_TYPE
{
	ITEM_CONACT,
	ITEM_GROUP,
	ITEM_PROFILE,
	ITEM_SPLITER,
};

#ifdef SCREEN_GORIZONTAL
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define CONTACT_ITEM_TYPE "%I16%I%I6%M232%I%I16"
#define GROUP_ITEM_TYPE   "%I16%M224"
#define PROFILE_ITEM_TYPE "%c20%M270%I16"
#define SPLITER_ITEM_TYPE "%I320"
#else
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define CONTACT_ITEM_TYPE "%I16%I%I6%M154%I%I16"
#define GROUP_ITEM_TYPE   "%I16%M224"
#define PROFILE_ITEM_TYPE "%c20%M190%I16"
#define SPLITER_ITEM_TYPE "%I240"
#endif

#define CONVERT_TYPE(type)		((type==ITEM_CONACT)?CONTACT_ITEM_TYPE:(type==ITEM_GROUP)?GROUP_ITEM_TYPE:(type==ITEM_PROFILE)?PROFILE_ITEM_TYPE:SPLITER_ITEM_TYPE)
#define GET_NUM_NICK(type)		((type==ITEM_CONACT)?3:(type==ITEM_GROUP)?1:(type==ITEM_PROFILE)?1:-1)
#define GET_NUM_XSTATUS(type)	((type==ITEM_CONACT)?(cfg_rigthAlignXStatus?4:1):-1)
#define GET_NUM_FAKE(type)		((type==ITEM_CONACT)?4:-1)
#define GET_NUM_CLIENT(type)	((type==ITEM_CONACT)?5:-1)
#define GET_NUM_AUTH(type)		((type==ITEM_CONACT)?2:-1)
#define GET_NUM_STATUS(type)	((type==ITEM_PROFILE)?2:0)

//include constant
#include "icqkid2_constants.h"
#include "zXMPP_const.h"
#include "const_strings.h"

//For support zPlayer by BeZ
#define channelNowPlaying "/zIM/NowPlaying"

#endif
