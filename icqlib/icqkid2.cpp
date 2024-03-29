/***************************************************************************
 *   Copyright (C) 2007 by Alexander S. Salieff                            *
 *   salieff@mail.ru                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "napi.h"

#include "icqkid2.h"
#include "tnetwork.h"
#include "flap.h"
#include "snaccache.h"
#include "md5.h"

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define CLOSE_SOCK(arg) close(arg); arg=-1;

#include <sys/timeb.h>

#include "zgui.h"
#include "zDefs.h"

#include <sstream>
using namespace std;

// Clear old SNAC Cache entry after 5 min
#define SNAC_CACHE_LIFETIME 300

#define OFFLINE_RECIEVE   (uint8_t)0x3c
#define OFFLINE_DELETE    (uint8_t)0x3e

#define SSI_ITEM_ADD       (uint16_t)0x0008
#define SSI_ITEM_CHANGE    (uint16_t)0x0009
#define SSI_ITEM_DELETE    (uint16_t)0x000a

#define AOL_SALT_STR "AOL Instant Messenger (SM)"

// --PUBLIC--PUBLIC--PUBLIC--PUBLIC--PUBLIC--PUBLIC--PUBLIC--PUBLIC--
// --PUBLIC--PUBLIC--PUBLIC--PUBLIC--PUBLIC--PUBLIC--PUBLIC--PUBLIC--
// --PUBLIC--PUBLIC--PUBLIC--PUBLIC--PUBLIC--PUBLIC--PUBLIC--PUBLIC--

void * pollKeepAlive ( void * );
void * startReadSNAC ( void * );

static const uint8_t XStatus_arr[37][16] =
{
	{0x01, 0xD8, 0xD7, 0xEE, 0xAC, 0x3B, 0x49, 0x2A, 0xA5, 0x8D, 0xD3, 0xD8, 0x77, 0xE6, 0x6B, 0x92},
	{0x5A, 0x58, 0x1E, 0xA1, 0xE5, 0x80, 0x43, 0x0C, 0xA0, 0x6F, 0x61, 0x22, 0x98, 0xB7, 0xE4, 0xC7},
	{0x83, 0xC9, 0xB7, 0x8E, 0x77, 0xE7, 0x43, 0x78, 0xB2, 0xC5, 0xFB, 0x6C, 0xFC, 0xC3, 0x5B, 0xEC},
	{0xE6, 0x01, 0xE4, 0x1C, 0x33, 0x73, 0x4B, 0xD1, 0xBC, 0x06, 0x81, 0x1D, 0x6C, 0x32, 0x3D, 0x81},
	{0x8C, 0x50, 0xDB, 0xAE, 0x81, 0xED, 0x47, 0x86, 0xAC, 0xCA, 0x16, 0xCC, 0x32, 0x13, 0xC7, 0xB7},
	{0x3F, 0xB0, 0xBD, 0x36, 0xAF, 0x3B, 0x4A, 0x60, 0x9E, 0xEF, 0xCF, 0x19, 0x0F, 0x6A, 0x5A, 0x7F},
	{0xF8, 0xE8, 0xD7, 0xB2, 0x82, 0xC4, 0x41, 0x42, 0x90, 0xF8, 0x10, 0xC6, 0xCE, 0x0A, 0x89, 0xA6},
	{0x80, 0x53, 0x7D, 0xE2, 0xA4, 0x67, 0x4A, 0x76, 0xB3, 0x54, 0x6D, 0xFD, 0x07, 0x5F, 0x5E, 0xC6},
	{0xF1, 0x8A, 0xB5, 0x2E, 0xDC, 0x57, 0x49, 0x1D, 0x99, 0xDC, 0x64, 0x44, 0x50, 0x24, 0x57, 0xAF},
	{0x1B, 0x78, 0xAE, 0x31, 0xFA, 0x0B, 0x4D, 0x38, 0x93, 0xD1, 0x99, 0x7E, 0xEE, 0xAF, 0xB2, 0x18},
	{0x61, 0xBE, 0xE0, 0xDD, 0x8B, 0xDD, 0x47, 0x5D, 0x8D, 0xEE, 0x5F, 0x4B, 0xAA, 0xCF, 0x19, 0xA7},
	{0x48, 0x8E, 0x14, 0x89, 0x8A, 0xCA, 0x4A, 0x08, 0x82, 0xAA, 0x77, 0xCE, 0x7A, 0x16, 0x52, 0x08},
	{0x10, 0x7A, 0x9A, 0x18, 0x12, 0x32, 0x4D, 0xA4, 0xB6, 0xCD, 0x08, 0x79, 0xDB, 0x78, 0x0F, 0x09},
	{0x6F, 0x49, 0x30, 0x98, 0x4F, 0x7C, 0x4A, 0xFF, 0xA2, 0x76, 0x34, 0xA0, 0x3B, 0xCE, 0xAE, 0xA7},
	{0x12, 0x92, 0xE5, 0x50, 0x1B, 0x64, 0x4F, 0x66, 0xB2, 0x06, 0xB2, 0x9A, 0xF3, 0x78, 0xE4, 0x8D},
	{0xD4, 0xA6, 0x11, 0xD0, 0x8F, 0x01, 0x4E, 0xC0, 0x92, 0x23, 0xC5, 0xB6, 0xBE, 0xC6, 0xCC, 0xF0},
	{0x60, 0x9D, 0x52, 0xF8, 0xA2, 0x9A, 0x49, 0xA6, 0xB2, 0xA0, 0x25, 0x24, 0xC5, 0xE9, 0xD2, 0x60},
	{0x63, 0x62, 0x73, 0x37, 0xA0, 0x3F, 0x49, 0xFF, 0x80, 0xE5, 0xF7, 0x09, 0xCD, 0xE0, 0xA4, 0xEE},
	{0x1F, 0x7A, 0x40, 0x71, 0xBF, 0x3B, 0x4E, 0x60, 0xBC, 0x32, 0x4C, 0x57, 0x87, 0xB0, 0x4C, 0xF1},
	{0x78, 0x5E, 0x8C, 0x48, 0x40, 0xD3, 0x4C, 0x65, 0x88, 0x6F, 0x04, 0xCF, 0x3F, 0x3F, 0x43, 0xDF},
	{0xA6, 0xED, 0x55, 0x7E, 0x6B, 0xF7, 0x44, 0xD4, 0xA5, 0xD4, 0xD2, 0xE7, 0xD9, 0x5C, 0xE8, 0x1F},
	{0x12, 0xD0, 0x7E, 0x3E, 0xF8, 0x85, 0x48, 0x9E, 0x8E, 0x97, 0xA7, 0x2A, 0x65, 0x51, 0xE5, 0x8D},
	{0xBA, 0x74, 0xDB, 0x3E, 0x9E, 0x24, 0x43, 0x4B, 0x87, 0xB6, 0x2F, 0x6B, 0x8D, 0xFE, 0xE5, 0x0F},
	{0x63, 0x4F, 0x6B, 0xD8, 0xAD, 0xD2, 0x4A, 0xA1, 0xAA, 0xB9, 0x11, 0x5B, 0xC2, 0x6D, 0x05, 0xA1},
	//Netvigator (China)
	{0x2C, 0xE0, 0xE4, 0xE5, 0x7C, 0x64, 0x43, 0x70, 0x9C, 0x3A, 0x7A, 0x1C, 0xE8, 0x78, 0xA7, 0xDC},
	{0x10, 0x11, 0x17, 0xC9, 0xA3, 0xB0, 0x40, 0xF9, 0x81, 0xAC, 0x49, 0xE1, 0x59, 0xFB, 0xD5, 0xD4},
	{0x16, 0x0C, 0x60, 0xBB, 0xDD, 0x44, 0x43, 0xF3, 0x91, 0x40, 0x05, 0x0F, 0x00, 0xE6, 0xC0, 0x09},
	{0x64, 0x43, 0xC6, 0xAF, 0x22, 0x60, 0x45, 0x17, 0xB5, 0x8C, 0xD7, 0xDF, 0x8E, 0x29, 0x03, 0x52},
	{0x16, 0xF5, 0xB7, 0x6F, 0xA9, 0xD2, 0x40, 0x35, 0x8C, 0xC5, 0xC0, 0x84, 0x70, 0x3C, 0x98, 0xFA},
	//ProSieben (De)
	{0x63, 0x14, 0x36, 0xff, 0x3f, 0x8a, 0x40, 0xd0, 0xa5, 0xcb, 0x7b, 0x66, 0xe0, 0x51, 0xb3, 0x64},
	{0xb7, 0x08, 0x67, 0xf5, 0x38, 0x25, 0x43, 0x27, 0xa1, 0xff, 0xcf, 0x4c, 0xc1, 0x93, 0x97, 0x97},
	{0xdd, 0xcf, 0x0e, 0xa9, 0x71, 0x95, 0x40, 0x48, 0xa9, 0xc6, 0x41, 0x32, 0x06, 0xd6, 0xf2, 0x80},
	//RnQ
	{0x3F, 0xB0, 0xBD, 0x36, 0xAF, 0x3B, 0x4A, 0x60, 0x9E, 0xEF, 0xCF, 0x19, 0x0F, 0x6A, 0x5A, 0x7E}, // XStatus(Smoking)
	//{0xe6, 0x01, 0xe4, 0x1c, 0x33, 0x73, 0x4b, 0xd1, 0xbc, 0x06, 0x81, 0x1d, 0x6c, 0x32, 0x3d, 0x82}, // XStatus(Smoking)
	{0xE6, 0x01, 0xE4, 0x1C, 0x33, 0x73, 0x4b, 0xD1, 0xBC, 0x06, 0x81, 0x1D, 0x6C, 0x32, 0x3D, 0x82},
	//{0xd6, 0x68, 0x7f, 0x4f, 0x3d, 0xc3, 0x4b, 0xdb, 0x8a, 0x8c, 0x4c, 0x1a, 0x57, 0x27, 0x63, 0xcd} //XStatus(Sex)
	//Rambler (Ru)
	{0xD4, 0xE2, 0xB0, 0xBA, 0x33, 0x4E, 0x4F, 0xA5, 0x98, 0xD0, 0x11, 0x7D, 0xBF, 0x4D, 0x3C, 0xC8}, // XStatus(Ru1)
	{0x00, 0x72, 0xD9, 0x08, 0x4A, 0xD1, 0x43, 0xDD, 0x91, 0x99, 0x6F, 0x02, 0x69, 0x66, 0x02, 0x6F},
// XStatus(Ru2)
	{0xCD, 0x56, 0x43, 0xA2, 0xC9, 0x4C, 0x47, 0x24, 0xB5, 0x2C, 0xDC, 0x01, 0x24, 0xA1, 0xD0, 0xCD},
// XStatus(Ru3)
};

extern "C" void __napi_bindsocket2link(int fd);
// ----------------=========ooooOOOOOOOOOoooo=========----------------
ICQKid2::ICQKid2()
		: sock ( -1 ), flap_seq_number ( 1 ), snac_seq_number ( 1 ), network_timeout ( -1 ), \
		connect_phase_percentage ( 0 ), \
		online_status ( STATUS_OFFLINE ), xStatus ( X_STATUS_NONE ), \
		myPrivSrvStatus ( PRIV_ALL_CAN_SEE ), myPrivSrvStatus_item_id ( 0 )
{
	snac_cache = new SnacCache;
	setLoginHost();
	last_keepalive_timestamp=time ( NULL );
	tzset();
//
	network_break_flag = false;
	noAutoXTrazRequest = false;
	noAutoMsgRequest = false;
	enabledEye = false;
	timeWaitSnec = 1500;
	delayReadSnec = 500;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
ICQKid2::~ICQKid2()
{
	//doDisconnect(); // no need
	delete snac_cache;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::doConnect ( uint32_t astat )
{
	logMes_4 ( "ICQKid2::doConnect" );
	stepConnect = 0;

	connect_error_code=0;
	connect_error_url="";

	uint32_t snac_sync;
	vector<uint8_t> md5_salt;
	string boss_host;
	int boss_port;
	vector<uint8_t> boss_cookie;

// PHASE I
	connect_phase_percentage=0;
	
	sock=directConnect ( loginhost, loginport );
	
	if ( sock<0 ) return false;
	connect_phase_percentage += 2;

	logMes_4 ( "sock ok" );
	stepConnect++; //1
	if ( !waitHello() ) { CLOSE_SOCK ( sock ); return false; }
	connect_phase_percentage += 2;

	logMes_4 ( "waitHello()" );
	stepConnect++; //2
	if ( !sendSignOn0() ) { CLOSE_SOCK ( sock ); return false; }
	connect_phase_percentage += 2;

	logMes_4 ( "sendSignOn0()" );
	stepConnect++; //3
	if ( !sendMD5saltrequest ( &snac_sync ) ) { CLOSE_SOCK ( sock ); return false; }
	connect_phase_percentage += 2;

	pthread_attr_t attr;
	pthread_attr_init ( &attr );
	pthread_attr_setdetachstate ( &attr, PTHREAD_CREATE_DETACHED );
	pthread_create ( &threadWaitSNAC, &attr, startReadSNAC, NULL );
	pthread_attr_destroy ( &attr );

	logMes_4 ( "sendMD5saltrequest()" );
	stepConnect++; //4
	if ( !getMD5salt ( snac_sync, md5_salt ) ) { CLOSE_SOCK ( sock ); return false; }
	connect_phase_percentage += 2;

	logMes_4 ( "getMD5salt()" );
	stepConnect++; //5
	if ( !sendMD5authorize ( &snac_sync, md5_salt ) ) { CLOSE_SOCK ( sock ); return false; }
	connect_phase_percentage += 2;

	logMes_4 ( "sendMD5authorize()" );
	stepConnect++; //6
	if ( !getBOSSparams ( snac_sync, boss_host, boss_port, boss_cookie ) ) { CLOSE_SOCK ( sock ); return false; }
	connect_phase_percentage += 2;

	logMes_4 ( "getBOSSparams()" );
	stepConnect++; //7
	( void ) sendSignOff();
	CLOSE_SOCK ( sock );

	pthread_cancel ( threadWaitSNAC );

	return doConnect_phase2 ( astat, boss_host, boss_port, boss_cookie );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::doConnect_phase2 ( uint32_t astat, string boss_host, int boss_port, vector<uint8_t> & boss_cookie, bool short_phase )
{
	logMes_4 ( "doConnect_phase2()" );
// PHASE II
	stepConnect++; //8

	connect_phase_percentage=14;
	
	sock=directConnect ( boss_host, boss_port );

	logMes_4 ( "sock=%d",sock );
	if ( sock<0 ) return false;
	connect_phase_percentage += 3;
	logMes_4 ( "sock<0" );
	stepConnect++; //9

	if ( !waitHello() ) { CLOSE_SOCK ( sock ); return false; }
	connect_phase_percentage += 3;
	logMes_4 ( "waitHello()" );
	stepConnect++; //10

	pthread_attr_t attr;
	pthread_attr_init ( &attr );
	pthread_attr_setdetachstate ( &attr, PTHREAD_CREATE_DETACHED );
	pthread_create ( &threadWaitSNAC, &attr, startReadSNAC, NULL );
	pthread_attr_destroy ( &attr );

	if ( !sendSignOn2 ( boss_cookie ) ) { CLOSE_SOCK ( sock ); return false; }
	connect_phase_percentage += 4;
	logMes_4 ( "sendSignOn2(boss_cookie)" );
	stepConnect++; //11

	if ( !getServiceList() ) { CLOSE_SOCK ( sock ); return false; } // 01,03
	connect_phase_percentage += 4;
	logMes_4 ( "getServiceList()" );
	stepConnect++; //12

	if ( !askServices() ) { CLOSE_SOCK ( sock ); return false; } // 01,17
	connect_phase_percentage += 3;
	logMes_4 ( "askServices() - 01,17" );
	stepConnect++; //13

	if ( !getServices() ) { CLOSE_SOCK ( sock ); return false; } // 01,18
	connect_phase_percentage += 3; //34
	logMes_4 ( "getServices() - 01,18" );
	stepConnect++; //14


// Skip message of the day // 01,13

	uint32_t sync_id;
	if ( !askRateLimits ( &sync_id ) ) { CLOSE_SOCK ( sock ); return false; } // 01,06
	connect_phase_percentage += 4;
	logMes_4 ( "askRateLimits(&sync_id)" );
	stepConnect++; //15

	if ( !getRateLimits ( sync_id ) ) { CLOSE_SOCK ( sock ); return false; } // 01,07
	connect_phase_percentage += 4;
	logMes_4 ( "getRateLimits(sync_id)" );
	stepConnect++; //16

	if ( !sendRateLimits() ) { CLOSE_SOCK ( sock ); return false; } // 01,08
	connect_phase_percentage += 3;
	logMes_4 ( "sendRateLimits()" );
	stepConnect++; //17

	if ( !short_phase )
	{
		// PHASE III
		logMes_4 ( "PHASE III" );
		stepConnect++; //18
		uint32_t self_sync_id;
		uint32_t ssi_sync_id, ssi_copy_id;
		uint32_t loc_sync_id;
		uint32_t blm_sync_id;
		uint32_t icbm_sync_id;
		uint32_t prm_sync_id;

		if ( !askSelfInfo ( &self_sync_id ) ) { CLOSE_SOCK ( sock ); return false; } // 01,0e
		connect_phase_percentage += 1;
		logMes_4 ( "askSelfInfo(&self_sync_id)" );
		stepConnect++; //19
		if ( !askSSILimit ( &ssi_sync_id ) ) { CLOSE_SOCK ( sock ); return false; } // 13,02
		connect_phase_percentage += 3;
		logMes_4 ( "skSSILimit(&ssi_sync_id)" );
		stepConnect++; //20
		if ( !askSSICopy ( &ssi_copy_id ) ) { CLOSE_SOCK ( sock ); return false; } // 13,04 Not 05 - we havent got local copy
		connect_phase_percentage += 3;
		logMes_4 ( "askSSICopy(&ssi_copy_id)" );
		stepConnect++; //21
		if ( !askLocationLimit ( &loc_sync_id ) ) { CLOSE_SOCK ( sock ); return false; } // 02,02
		connect_phase_percentage += 3;
		logMes_4 ( "askLocationLimit(&loc_sync_id)" );
		stepConnect++; //22
		if ( !askBLMLimit ( &blm_sync_id ) ) { CLOSE_SOCK ( sock ); return false; } // 03,02
		connect_phase_percentage += 3;
		logMes_4 ( "askBLMLimit(&blm_sync_id)" );
		stepConnect++; //23
		if ( !askICBMParams ( &icbm_sync_id ) ) { CLOSE_SOCK ( sock ); return false; } // 04,04
		connect_phase_percentage += 3;
		logMes_4 ( "askICBMParams(&icbm_sync_id)" );
		stepConnect++; //24
		if ( !askPRMLimit ( &prm_sync_id ) ) { CLOSE_SOCK ( sock ); return false; } // 09,02
		connect_phase_percentage += 3;
		logMes_4 ( "askPRMLimit(&prm_sync_id)" );
		stepConnect++; //25
		if ( !getLocationLimit ( loc_sync_id ) ) { CLOSE_SOCK ( sock ); return false; } // 02,03
		connect_phase_percentage += 3;
		logMes_4 ( "getLocationLimit(loc_sync_id)" );
		stepConnect++; //26
		if ( !getBLMLimit ( blm_sync_id ) ) { CLOSE_SOCK ( sock ); return false; } // 03,03
		connect_phase_percentage += 3;
		logMes_4 ( "getBLMLimit(blm_sync_id)" );
		stepConnect++; //27
		if ( !getICBMParams ( icbm_sync_id ) ) { CLOSE_SOCK ( sock ); return false; } // 04,05
		connect_phase_percentage += 4;
		logMes_4 ( "getICBMParams(icbm_sync_id)" );
		stepConnect++; //28
		if ( !getPRMLimit ( prm_sync_id ) ) { CLOSE_SOCK ( sock ); return false; } // 09,03
		connect_phase_percentage += 3;
		logMes_4 ( "getPRMLimit(prm_sync_id)" );
		stepConnect++; //29
		if ( !getSSILimit ( ssi_sync_id ) ) { CLOSE_SOCK ( sock ); return false; } // 13,03
		connect_phase_percentage += 4;
		logMes_4 ( "getSSILimit(ssi_sync_id)" );
		stepConnect++; //30
		if ( !getSSICopy ( ssi_copy_id ) ) { CLOSE_SOCK ( sock ); logMes_4 ( "Error on getSSICopy" ); return false; } // 13,06 Not 0F - our copy always out-to-date
		connect_phase_percentage += 3;
		logMes_4 ( "getSSICopy(ssi_copy_id)" );
		stepConnect++; //31
		if ( !sendLocationInfo() ) { CLOSE_SOCK ( sock ); return false; } // 02,04
		connect_phase_percentage += 4;
		logMes_4 ( "sendLocationInfo()" );
		stepConnect++; //32
		if ( !getSelfInfo ( self_sync_id ) ) { CLOSE_SOCK ( sock ); return false; } // 01,0f
		connect_phase_percentage += 1;
		logMes_4 ( "getSelfInfo(self_sync_id)" );
		stepConnect++; //33
		if ( !sendICBMParams() ) { CLOSE_SOCK ( sock ); return false; } // 04,02
		connect_phase_percentage += 4;
		logMes_4 ( "sendICBMParams()" );
		stepConnect++; //34
		if ( !sendStatus ( astat, true ) ) { CLOSE_SOCK ( sock ); return false; } // 01,1E
		logMes_4 ( "sendStatus(astat, true)" );
		stepConnect++; //35
		online_status=astat;
		// Skip 01,0f
	}
	logMes_4 ( "PHASE IV" );
	stepConnect++; //36
// PHASE IV
	if ( !sendReady() ) { CLOSE_SOCK ( sock ); return false; } // 01,02
// Skip 0b,02
	connect_phase_percentage += 3;
	logMes_4 ( "sendReady()" );
	stepConnect++; //37

	if ( !short_phase )
	{
		logMes_4 ( "> short_phase" );
		stepConnect++; //38
		if ( !activateSSI() ) { CLOSE_SOCK ( sock ); return false; } // 13,07
		connect_phase_percentage += 4;
		logMes_4 ( "activateSSI()" );
		stepConnect++; //39
		if ( !sendStatus ( astat ) ) { CLOSE_SOCK ( sock ); return false; } // 01,1E
		logMes_4 ( "sendStatus(astat)" );
		stepConnect++; //40
		online_status=astat;
		// Skip 01,0f
	}

	connect_phase_percentage = 100;
	logMes_4 ( "connect_phase_percentage = 100" );
	return true;
}


// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::setStatus ( uint32_t astat )
{
	logMes_4 ( "ICQKid2::setStatus - "+QString::number(astat) );
	if ( !sendStatus ( astat ) ) return false;
	if ( !getSelfInfo ( 0 ) ) return false;
	online_status=astat;
	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::setXStatus ( size_t x_stat, string title, string descr )
{
	logMes_4 ( "ICQKid2::setXStatus" );
	xStatus=x_stat;
	xStatusTitle=title;
	xStatusDescription=descr;

	return sendLocationInfo();
}

bool ICQKid2::setXStatusDescription ( string descr )
{
	logMes_4 ( "ICQKid2::setXStatus Description" );
	xStatusDescription=descr;

	return sendLocationInfo();
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::changePassword ( string new_password )
{
	logMes_4 ( "ICQKid2::changePassword" );
	TLVField tlv ( NULL, 0, 0x0001 );
	uint16_t chunk_size=10+2+new_password.length() +1;
	tlv.data.resize ( chunk_size+2 );

	size_t curr_pos=0;
	memcpy ( &tlv.data[curr_pos], &chunk_size, sizeof ( chunk_size ) ); // Little Endian!!!
	curr_pos += sizeof ( chunk_size );

	uint32_t int_uin = atol ( myuin.c_str() );
	memcpy ( &tlv.data[curr_pos], &int_uin, sizeof ( int_uin ) ); // Little Endian!!!
	curr_pos += sizeof ( int_uin );

	tlv.data[curr_pos]=0xd0;
	tlv.data[curr_pos+1]=0x07; // 07D0 - META_DATA_REQ
	tlv.data[curr_pos+2]=0x02;
	tlv.data[curr_pos+3]=0x00; // 0002 - request sequence number
	tlv.data[curr_pos+4]=0x2e;
	tlv.data[curr_pos+5]=0x04; // 042e - META_SET_PASSWORD
	curr_pos += 6;

	uint16_t pass_len = new_password.length() +1; // +1 for NULL-terminator
	memcpy ( &tlv.data[curr_pos], &pass_len, sizeof ( pass_len ) ); // Little Endian!!!
	curr_pos += sizeof ( pass_len );
	memcpy ( &tlv.data[curr_pos], new_password.c_str(), pass_len );

	vector<uint8_t> data_vec;
	tlv.encode_to ( data_vec, 0 );

	uint32_t snac_sync;
	if ( sendSNAC ( 0x0015, 0x0002, &snac_sync, &data_vec ) !=1 ) return false;

	SNACData snd;
	snd.service_id=0x0015;
	snd.subtype_id=0x0003;
	snd.req_id=snac_sync;
	if ( waitSNAC ( &snd ) !=1 ) return false;
	tlv.data.clear();
	if ( !tlv.decode_from ( snd.data, 0 ) ) return false;
	if ( tlv.type!=0x0001 ) return false;
	if ( tlv.data.size() <13 ) return false;
	if ( memcmp ( &tlv.data[2], &int_uin, sizeof ( int_uin ) ) !=0 ) return false;
	if ( tlv.data[6]!=0xda || tlv.data[7]!=0x07 ) return false; // META_DATA_REQ
	if ( tlv.data[8]!=0x02 || tlv.data[9]!=0x00 ) return false; // sequence number
	if ( tlv.data[10]!=0xaa || tlv.data[11]!=0x00 ) return false; // data subtype: META_SET_PASSWORD_ACK
	if ( tlv.data[12]!=0x0a ) return false; // success byte

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
void ICQKid2::doDisconnect ( void )
{
	logMes_4 ( "ICQKid2::doDisconnect" );
	pthread_cancel ( keepConnect );
	pthread_cancel ( threadWaitSNAC );
	if ( sock<0 )
	{
		logMes_4 ( "ICQKid2::doDisconnect end" );
		return;
	}
	CLOSE_SOCK ( sock );
	sock=-1;
	logMes_4 ( "ICQKid2::doDisconnect end" );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::sendMessage ( ICQKid2Message msg )
{
	logMes_4 ( "ICQKid2::sendMessage" );
	int uen_ind=findCLUIN ( msg.uin );

	if ( uen_ind<0 && msg.enc_type!=ICQKid2Message::LOCAL8BIT && msg.enc_type!=ICQKid2Message::USASCII ) return false; // I cannot send unicode message to users who aren't in my contact list
	if ( uen_ind>=0 && ContactListUins[uen_ind].online_status==STATUS_OFFLINE && msg.enc_type!=ICQKid2Message::LOCAL8BIT && msg.enc_type!=ICQKid2Message::USASCII ) return false; // I cannot send unicode message to offline users

	if ( uen_ind>=0 && msg.enc_type==ICQKid2Message::UCS2BE && !ContactListUins[uen_ind].unicode_cap ) return false;
	if ( uen_ind>=0 && msg.enc_type==ICQKid2Message::UTF8 && ( !ContactListUins[uen_ind].unicode_cap || !ContactListUins[uen_ind].srv_relay_cap ) ) return false;

	BaseOutgoingMsg bom;
	bom.uin=msg.uin;

	switch ( msg.enc_type )
	{
		case ICQKid2Message::USASCII :
		{
			SimpleMsg out_sim;
			out_sim.message_str=msg.text;
			out_sim.charset=0;
			out_sim.sub_charset=msg.codepage;
			bom.msg_channel=0x0001;
			out_sim.encode_to ( bom.data );
		}
		break;

		case ICQKid2Message::LOCAL8BIT :
		{
			SimpleMsg out_sim;
			out_sim.message_str=msg.text;
			out_sim.charset=3;
			out_sim.sub_charset=msg.codepage;
			bom.msg_channel=0x0001;
			out_sim.encode_to ( bom.data );
		}
		break;

		case ICQKid2Message::UCS2BE :
		{
			SimpleMsg out_sim;
			out_sim.message_str=msg.text;
			out_sim.charset=2;
			out_sim.sub_charset=msg.codepage;
			bom.msg_channel=0x0001;
			out_sim.encode_to ( bom.data );
		}
		break;

		case ICQKid2Message::UTF8 :
		{
			ExtendedMsg out_exm;
			out_exm.rv_msg.type=0; // Request
			memset ( out_exm.rv_msg.cookie, 'Y', 8 );
			uint8_t ser_rel[] = {0x09, 0x46, 0x13, 0x49, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00};  // ICQ Server relaying
			memcpy ( out_exm.rv_msg.capability, ser_rel, sizeof ( ser_rel ) );
			out_exm.rv_msg.external_ip=0x0100007f; // 127.0.0.1
			out_exm.rv_msg.external_port=0x220c; // 1234

			out_exm.rv_msg.tlv2711.version=8;
			out_exm.rv_msg.tlv2711.msg_type=1; // Plain text message
			out_exm.rv_msg.tlv2711.flags=0; // Normal message
			out_exm.rv_msg.tlv2711.status=0;
			out_exm.rv_msg.tlv2711.priority=1;
			out_exm.rv_msg.tlv2711.text=msg.text;
			memcpy ( out_exm.rv_msg.tlv2711.text_color, msg.text_color, 4 );
			memcpy ( out_exm.rv_msg.tlv2711.bg_color, msg.bg_color, 4 );
			out_exm.rv_msg.tlv2711.text_guid="{0946134E-4C7F-11D1-8222-444553540000}"; // UTF-8
			bom.msg_channel=0x0002;
			out_exm.encode_to ( bom.data );
		}
		break;
	}

	vector<uint8_t> vec;
	bom.encode_to ( vec );

	return ( sendSNAC ( 0x0004, 0x0006, NULL, &vec ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::sendMTN ( string to, uint16_t atype )
{
	logMes_4 ( "ICQKid2::sendMTN" );
	vector<uint8_t> data_vec ( 11+to.length() +2, 0 );
	data_vec[9]=1;
	data_vec[10]=to.length();
	memcpy ( &data_vec[11], to.data(), to.length() );
	uint16_t tmp_type = htons ( atype );
	memcpy ( &data_vec[11+to.length() ], &tmp_type, sizeof ( tmp_type ) );

	return ( sendSNAC ( 0x0004, 0x0014, NULL, &data_vec ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::getUserInfo ( string uin, ICQKidShortUserInfo & info, bool force_update )
{
	logMes_4 ( "ICQKid2::getUserInfo" );
	map<string, ICQKidShortUserInfo>::iterator mi = userinfo_map.find ( uin );
	if ( mi!=userinfo_map.end() && !force_update )
	{
		info=mi->second;
		return true;
	}

	uint32_t sync_id;
	if ( !askShortInfo ( uin, &sync_id ) ) return false;
	if ( !getShortInfo ( info, sync_id ) ) return false;

	info.Uin=uin;
	userinfo_map[uin]=info;
	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::getFullUserInfo ( string uin, ICQKidFullUserInfo & info, bool force_update )
{
	logMes_4 ( "ICQKid2::getFullUserInfo" );
	map<string, ICQKidFullUserInfo>::iterator mi = userfullinfo_map.find ( uin );
	if ( mi!=userfullinfo_map.end() && !force_update )
	{
		info=mi->second;
		return true;
	}
	logMes_4 ( "no Cash" );
	uint32_t sync_id;
	if ( !askFullInfo ( uin, &sync_id ) ) return false;
	logMes_4 ( "askFullInfo" );
	if ( !getFullInfo ( info, sync_id ) ) return false;
	logMes_4 ( "getFullInfo" );

	info.Uin=uin;
	userfullinfo_map[uin]=info;
	logMes_4 ( "return" );
	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::publicMyInfo ( ICQKidFullUserInfo & info )
{
	logMes_4 ( "ICQKid2::publicMyInfo" );
#define PACK_LE(arg) \
 if (tlv_vec.size()<(tlv_pos+sizeof(arg))) tlv_vec.resize(tlv_pos+sizeof(arg)); \
 memcpy(&tlv_vec[tlv_pos], &arg, sizeof(arg)); \
 tlv_pos+=sizeof(arg);

#define PACK_ASCIIZ(arg) \
 { \
 uint16_t str_len=arg.length()+1; \
 PACK_LE(str_len) \
 if (tlv_vec.size()<(tlv_pos+str_len)) tlv_vec.resize(tlv_pos+str_len); \
 memcpy(&tlv_vec[tlv_pos], arg.c_str(), str_len); \
 tlv_pos+=str_len; \
 }

#define PACK_TLV(arg) \
 { \
 uint16_t letlv_type=arg; \
 uint16_t letlv_size=tlv_vec.size(); \
 tlv_pack.insert(tlv_pack.end(), (uint8_t*)(&letlv_type), ((uint8_t*)(&letlv_type))+sizeof(letlv_type)); /* LE */ \
 tlv_pack.insert(tlv_pack.end(), (uint8_t*)(&letlv_size), ((uint8_t*)(&letlv_size))+sizeof(letlv_size)); /* LE */ \
 tlv_pack.insert(tlv_pack.end(), tlv_vec.begin(), tlv_vec.end()); \
 tlv_vec.clear(); \
 tlv_pos=0; \
 }

	vector<uint8_t> tlv_pack;
	vector<uint8_t> tlv_vec;
	size_t tlv_pos=0;

	PACK_ASCIIZ ( info.Nickname )
	PACK_TLV ( 0x0154 )

	PACK_ASCIIZ ( info.Firstname )
	PACK_TLV ( 0x0140 )

	PACK_ASCIIZ ( info.Lastname )
	PACK_TLV ( 0x014A )

	PACK_ASCIIZ ( info.Email )
	PACK_LE ( info.Pubprimarymail )
	PACK_TLV ( 0x015E )

	for ( size_t i=0; i<info.Emails.size(); ++i )
	{
		PACK_ASCIIZ ( info.Emails[i].first )
		PACK_LE ( info.Emails[i].second )
		PACK_TLV ( 0x015E )
	}

	PACK_ASCIIZ ( info.Homecity )
	PACK_TLV ( 0x0190 )

	PACK_ASCIIZ ( info.Homestate )
	PACK_TLV ( 0x019A )

	PACK_ASCIIZ ( info.Homephone )
	PACK_TLV ( 0x0276 )

	PACK_ASCIIZ ( info.Homefax )
	PACK_TLV ( 0x0280 )

	PACK_ASCIIZ ( info.Homeaddress )
	PACK_TLV ( 0x0262 )

	PACK_ASCIIZ ( info.Cellphone )
	PACK_TLV ( 0x028A )

	PACK_ASCIIZ ( info.Homezip )
	PACK_TLV ( 0x026D )

	PACK_LE ( info.Homecountry )
	PACK_TLV ( 0x01A4 )

	PACK_LE ( info.GMToffset )
	PACK_TLV ( 0x0316 )

	PACK_LE ( info.Auth )
	PACK_TLV ( 0x030C )

	PACK_LE ( info.Webaware )
	PACK_TLV ( 0x02F8 )

	PACK_LE ( info.Age )
	PACK_TLV ( 0x0172 )

	PACK_LE ( info.Gender )
	PACK_TLV ( 0x017C )

	PACK_ASCIIZ ( info.Homepage )
	PACK_TLV ( 0x0213 )

	uint16_t bmonth16=info.Birthmonth;
	uint16_t bday16=info.Birthday;
	PACK_LE ( info.Birthyear )
	PACK_LE ( bmonth16 )
	PACK_LE ( bday16 )
	PACK_TLV ( 0x023A )

	uint16_t lang16_1=info.Language1;
	PACK_LE ( lang16_1 )
	PACK_TLV ( 0x0186 )

	uint16_t lang16_2=info.Language2;
	PACK_LE ( lang16_2 )
	PACK_TLV ( 0x0186 )

	uint16_t lang16_3=info.Language3;
	PACK_LE ( lang16_3 )
	PACK_TLV ( 0x0186 )

	PACK_ASCIIZ ( info.Origcity )
	PACK_TLV ( 0x0320 )

	PACK_ASCIIZ ( info.Origstate )
	PACK_TLV ( 0x032A )

	PACK_LE ( info.Origcountry )
	PACK_TLV ( 0x0334 )

	PACK_LE ( info.Maritalstatus )
	PACK_TLV ( 0x033E )

	PACK_LE ( info.Allowspam )
	PACK_TLV ( 0x0348 )

	PACK_LE ( info.Codepage )
	PACK_TLV ( 0x0352 )

	PACK_ASCIIZ ( info.Workcity )
	PACK_TLV ( 0x029E )

	PACK_ASCIIZ ( info.Workstate )
	PACK_TLV ( 0x02A8 )

	PACK_ASCIIZ ( info.Workphone )
	PACK_TLV ( 0x02C6 )

	PACK_ASCIIZ ( info.Workfax )
	PACK_TLV ( 0x02D0 )

	PACK_ASCIIZ ( info.Workaddress )
	PACK_TLV ( 0x0294 )

	PACK_ASCIIZ ( info.Workzip )
	PACK_TLV ( 0x02BD )

	PACK_LE ( info.Workcountry )
	PACK_TLV ( 0x02B2 )

	PACK_ASCIIZ ( info.Workcompany )
	PACK_TLV ( 0x01AE )

	PACK_ASCIIZ ( info.Workdepartment )
	PACK_TLV ( 0x01B8 )

	PACK_ASCIIZ ( info.Workposition )
	PACK_TLV ( 0x01C2 )

	PACK_LE ( info.Workoccupation )
	PACK_TLV ( 0x01CC )

	PACK_ASCIIZ ( info.Workwebpage )
	PACK_TLV ( 0x02DA )

	PACK_ASCIIZ ( info.Notes )
	PACK_TLV ( 0x0258 )

	PACK_LE ( info.Interest1category )
	PACK_ASCIIZ ( info.Interest1 )
	PACK_TLV ( 0x01EA )

	PACK_LE ( info.Interest2category )
	PACK_ASCIIZ ( info.Interest2 )
	PACK_TLV ( 0x01EA )

	PACK_LE ( info.Interest3category )
	PACK_ASCIIZ ( info.Interest3 )
	PACK_TLV ( 0x01EA )

	PACK_LE ( info.Interest4category )
	PACK_ASCIIZ ( info.Interest4 )
	PACK_TLV ( 0x01EA )

// In iServerd guide tlv types for past and affiliation were changed
	PACK_LE ( info.Past1category )
	PACK_ASCIIZ ( info.Past1 )
	PACK_TLV ( 0x01D6 )

	PACK_LE ( info.Past2category )
	PACK_ASCIIZ ( info.Past2 )
	PACK_TLV ( 0x01D6 )

	PACK_LE ( info.Past3category )
	PACK_ASCIIZ ( info.Past3 )
	PACK_TLV ( 0x01D6 )

	PACK_LE ( info.Affiliation1category )
	PACK_ASCIIZ ( info.Affiliation1 )
	PACK_TLV ( 0x01FE )

	PACK_LE ( info.Affiliation2category )
	PACK_ASCIIZ ( info.Affiliation2 )
	PACK_TLV ( 0x01FE )

	PACK_LE ( info.Affiliation3category )
	PACK_ASCIIZ ( info.Affiliation3 )
	PACK_TLV ( 0x01FE )

	uint16_t chunk_size=tlv_pack.size() +10;
	uint32_t my_int_uin=atoi ( myuin.c_str() );
	uint8_t tail[] = { 0xd0, 0x07,   /* data type: META_DATA_REQ */ \
	                   0x02, 0x00,   /* request sequence number */ \
	                   0x3a, 0x0c
	                 }; /* data subtype: CLI_SET_FULLINFO */

// Reuse tlv_vec
	tlv_vec.clear();
	PACK_LE ( chunk_size )
	PACK_LE ( my_int_uin )
	tlv_vec.insert ( tlv_vec.end(), tail, tail+sizeof ( tail ) );
	tlv_vec.insert ( tlv_vec.end(), tlv_pack.begin(), tlv_pack.end() );

#undef PACK_TLV
#undef PACK_ASCIIZ
#undef PACK_LE

	TLVField tlv_f ( tlv_vec, 0x0001 ); // encapsulated META_DATA
	tlv_vec.clear();
	tlv_f.encode_to ( tlv_vec, 0 );

	uint32_t snac_sync;
	if ( sendSNAC ( 0x0015, 0x0002, &snac_sync, &tlv_vec ) !=1 ) return false;

	SNACData snd;
	snd.service_id=0x0015;
	snd.subtype_id=0x0003;
	snd.req_id=snac_sync;
	if ( waitSNAC ( &snd ) !=1 ) return false;
	if ( !tlv_f.decode_from ( snd.data, 0 ) ) return false;
	if ( tlv_f.type!=0x0001 ) return false;
	if ( tlv_f.data.size() <13 ) return false;
	if ( memcmp ( &tlv_f.data[2], &my_int_uin, sizeof ( my_int_uin ) ) !=0 ) return false;
	if ( tlv_f.data[6]!=0xda || tlv_f.data[7]!=0x07 || tlv_f.data[10]!=0x3f || tlv_f.data[11]!=0x0c || tlv_f.data[12]!=0x0a ) return false;
	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::searchByWhitePages ( ICQKidFullUserInfo & info, vector<ICQKidShortUserInfo> & result_vec )
{
	logMes_4 ( "ICQKid2::searchByWhitePages" );
#define PACK_LE(arg) \
 if (tlv_vec.size()<(tlv_pos+sizeof(arg))) tlv_vec.resize(tlv_pos+sizeof(arg)); \
 memcpy(&tlv_vec[tlv_pos], &arg, sizeof(arg)); \
 tlv_pos+=sizeof(arg);

#define PACK_ASCIIZ(arg) \
 { \
 uint16_t str_len=arg.length()+1; \
 PACK_LE(str_len) \
 if (tlv_vec.size()<(tlv_pos+str_len)) tlv_vec.resize(tlv_pos+str_len); \
 memcpy(&tlv_vec[tlv_pos], arg.c_str(), str_len); \
 tlv_pos+=str_len; \
 }

#define PACK_TLV(arg) \
 { \
 uint16_t letlv_type=arg; \
 uint16_t letlv_size=tlv_vec.size(); \
 tlv_pack.insert(tlv_pack.end(), (uint8_t*)(&letlv_type), ((uint8_t*)(&letlv_type))+sizeof(letlv_type)); /* LE */ \
 tlv_pack.insert(tlv_pack.end(), (uint8_t*)(&letlv_size), ((uint8_t*)(&letlv_size))+sizeof(letlv_size)); /* LE */ \
 tlv_pack.insert(tlv_pack.end(), tlv_vec.begin(), tlv_vec.end()); \
 tlv_vec.clear(); \
 tlv_pos=0; \
 }

#define CHECK_FOR_ASCIIZ(str, tlv_type) \
 if (str!="") \
  { \
  PACK_ASCIIZ(str) \
  PACK_TLV(tlv_type) \
  }

#define CHECK_FOR_LE(arg, tlv_type) \
 if (arg!=0) \
  { \
  PACK_LE(arg) \
  PACK_TLV(tlv_type) \
  }

	vector<uint8_t> tlv_pack;
	vector<uint8_t> tlv_vec;
	size_t tlv_pos=0;

	CHECK_FOR_ASCIIZ ( info.Firstname, 0x0140 )
	CHECK_FOR_ASCIIZ ( info.Lastname, 0x014A )
	CHECK_FOR_ASCIIZ ( info.Nickname, 0x0154 )
	CHECK_FOR_ASCIIZ ( info.Email, 0x015E )
	CHECK_FOR_ASCIIZ ( info.Homecity, 0x0190 )
	CHECK_FOR_ASCIIZ ( info.Homestate, 0x019A )
	CHECK_FOR_LE ( info.Homecountry, 0x01A4 )
	CHECK_FOR_ASCIIZ ( info.Workcompany, 0x01AE )
	CHECK_FOR_ASCIIZ ( info.Workdepartment, 0x01B8 )
	CHECK_FOR_ASCIIZ ( info.Workposition, 0x01C2 )
	CHECK_FOR_LE ( info.Workoccupation, 0x01CC )
	CHECK_FOR_ASCIIZ ( info.Searchkey, 0x0226 )

	if ( info.Minage>0 && info.Maxage>0 && info.Minage<=info.Maxage )
	{
		PACK_LE ( info.Minage )
		PACK_LE ( info.Maxage )
		PACK_TLV ( 0x0168 )
	}

	CHECK_FOR_LE ( info.Gender, 0x017C )
	CHECK_FOR_LE ( info.Maritalstatus, 0x033E )

	uint16_t lang16_1=info.Language1;
	CHECK_FOR_LE ( lang16_1, 0x0186 )

	if ( info.Past1category!=0 || info.Past1!="" )
	{
		PACK_LE ( info.Past1category )
		PACK_ASCIIZ ( info.Past1 )
		PACK_TLV ( 0x01D6 )
	}

	if ( info.Interest1category!=0 || info.Interest1!="" )
	{
		PACK_LE ( info.Interest1category )
		PACK_ASCIIZ ( info.Interest1 )
		PACK_TLV ( 0x01EA )
	}

	if ( info.Affiliation1category!=0 || info.Affiliation1!="" )
	{
		PACK_LE ( info.Affiliation1category )
		PACK_ASCIIZ ( info.Affiliation1 )
		PACK_TLV ( 0x01FE )
	}

	if ( info.Homepagecategory!=0 || info.Homepagekeywords!="" )
	{
		PACK_LE ( info.Homepagecategory )
		PACK_ASCIIZ ( info.Homepagekeywords )
		PACK_TLV ( 0x0212 )
	}

	PACK_LE ( info.Searchonline )
	PACK_TLV ( 0x0230 )

	uint16_t chunk_size=tlv_pack.size() +10;
	uint32_t my_int_uin=atoi ( myuin.c_str() );
	uint8_t tail[] = { 0xd0, 0x07,   /* data type: META_DATA_REQ */ \
	                   0x02, 0x00,   /* request sequence number */ \
	                   0x5f, 0x05
	                 }; /* data subtype: CLI_WHITE_PAGES_SEARCH2 */

// Reuse tlv_vec
	tlv_vec.clear();
	PACK_LE ( chunk_size )
	PACK_LE ( my_int_uin )
	tlv_vec.insert ( tlv_vec.end(), tail, tail+sizeof ( tail ) );
	tlv_vec.insert ( tlv_vec.end(), tlv_pack.begin(), tlv_pack.end() );

#undef PACK_TLV
#undef PACK_ASCIIZ
#undef PACK_LE
#undef CHECK_FOR_ASCIIZ
#undef CHECK_FOR_LE

	TLVField tlv_f ( tlv_vec, 0x0001 ); // encapsulated META_DATA
	tlv_vec.clear();
	tlv_f.encode_to ( tlv_vec, 0 );

	uint32_t snac_sync;
	if ( sendSNAC ( 0x0015, 0x0002, &snac_sync, &tlv_vec ) !=1 ) return false;

	return getSearchAnswer ( result_vec, snac_sync );
}

/*
// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::searchByMail ( string email, vector<ICQKidShortUserInfo> & result_vec )
{
	logMes_4 ( "ICQKid2::searchByMail" );
	TLVField tlv ( NULL, 0, 0x0001 );
	uint16_t chunk_size=10+4+2+email.length() +1;
	tlv.data.resize ( chunk_size+2 );

	size_t curr_pos=0;
	memcpy ( &tlv.data[curr_pos], &chunk_size, sizeof ( chunk_size ) ); // Little Endian!!!
	curr_pos += sizeof ( chunk_size );

	uint32_t int_uin = atol ( myuin.c_str() );
	memcpy ( &tlv.data[curr_pos], &int_uin, sizeof ( int_uin ) ); // Little Endian!!!
	curr_pos += sizeof ( int_uin );

	tlv.data[curr_pos]=0xd0;
	tlv.data[curr_pos+1]=0x07; // 07D0 - META_DATA_REQ
	tlv.data[curr_pos+2]=0x02;
	tlv.data[curr_pos+3]=0x00; // 0002 - request sequence number
	tlv.data[curr_pos+4]=0x73;
	tlv.data[curr_pos+5]=0x05; // 0573 - META_SEARCH_BY_EMAIL_TLV
	curr_pos += 6;

	tlv.data[curr_pos++]=0x5e;
	tlv.data[curr_pos++]=0x01; // TLV.Type(0x015E) - email to search
	uint16_t subtlv_len = 2+email.length() +1;
	memcpy ( &tlv.data[curr_pos], &subtlv_len, sizeof ( subtlv_len ) ); // Little Endian!!!
	curr_pos += sizeof ( subtlv_len );

	uint16_t email_len = email.length() +1; // +1 for NULL-terminator
	memcpy ( &tlv.data[curr_pos], &email_len, sizeof ( email_len ) ); // Little Endian!!!
	curr_pos += sizeof ( email_len );
	memcpy ( &tlv.data[curr_pos], email.c_str(), email_len );

	vector<uint8_t> data_vec;
	tlv.encode_to ( data_vec, 0 );

	uint32_t snac_sync;
	if ( sendSNAC ( 0x0015, 0x0002, &snac_sync, &data_vec ) !=1 ) return false;

	return getSearchAnswer ( result_vec, snac_sync );
}
*/

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::searchByUIN ( string uin, vector<ICQKidShortUserInfo> & result_vec )
{
	logMes_4 ( "ICQKid2::searchByUIN" );
	TLVField tlv ( NULL, 0, 0x0001 );
	uint16_t chunk_size=10+4+4;
	tlv.data.resize ( chunk_size+2 );

	size_t curr_pos=0;
	memcpy ( &tlv.data[curr_pos], &chunk_size, sizeof ( chunk_size ) ); // Little Endian!!!
	curr_pos += sizeof ( chunk_size );

	uint32_t int_my_uin = atol ( myuin.c_str() );
	memcpy ( &tlv.data[curr_pos], &int_my_uin, sizeof ( int_my_uin ) ); // Little Endian!!!
	curr_pos += sizeof ( int_my_uin );

	tlv.data[curr_pos]=0xd0;
	tlv.data[curr_pos+1]=0x07; // 07D0 - META_DATA_REQ
	tlv.data[curr_pos+2]=0x02;
	tlv.data[curr_pos+3]=0x00; // 0002 - request sequence number
	tlv.data[curr_pos+4]=0x69;
	tlv.data[curr_pos+5]=0x05; // 0569 - META_SEARCH_BY_UIN_TLV
	curr_pos += 6;

	tlv.data[curr_pos++]=0x36;
	tlv.data[curr_pos++]=0x01; // TLV.Type(0x0136) - uin to search
	tlv.data[curr_pos++]=0x04;
	tlv.data[curr_pos++]=0x00; // TLV length

	uint32_t int_uin = atol ( uin.c_str() );
	memcpy ( &tlv.data[curr_pos], &int_uin, sizeof ( int_uin ) ); // Little Endian!!!
	curr_pos += sizeof ( int_uin );

	vector<uint8_t> data_vec;
	tlv.encode_to ( data_vec, 0 );

	uint32_t snac_sync;
	if ( sendSNAC ( 0x0015, 0x0002, &snac_sync, &data_vec ) !=1 ) return false;

	return getSearchAnswer ( result_vec, snac_sync );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::sub_ssi_addContact ( string uin, string nick, string groupname, uint16_t * retflag )
{
	logMes_4 ( "ICQKid2::sub_ssi_addContact" );
	int clg_ind = findCLGroup ( groupname );
	if ( clg_ind<0 )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NOTFOUND;
		return false;
	}

	SSIUINEntry uen;
	uen.uin=uin;
	uen.nick=nick;
	uen.groupid=ContactListGroups[clg_ind].id;
	uen.groupname=ContactListGroups[clg_ind].name;
	uen.itemid=getUnusedItemID();
	if ( uen.itemid==0 )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_LIMIT;
		return false;
	}
	uen.waitauth=false;

	uint16_t my_retflag;
	uint32_t sync_id;

	if ( !addDeleSSIUIN ( uen, SSI_ITEM_ADD, &sync_id ) )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}
	if ( !getSSIEditAck ( my_retflag, sync_id ) )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}

	// If need auth, try add it as awaiting auth
	if ( my_retflag==SSI_EDIT_ERR_NEEDAUTH )
	{
		uen.waitauth=true;
		if ( !addDeleSSIUIN ( uen, SSI_ITEM_ADD, &sync_id ) )
		{
			if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
			return false;
		}
		if ( !getSSIEditAck ( my_retflag, sync_id ) )
		{
			if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
			return false;
		}
	}
	if ( retflag!=NULL ) *retflag=my_retflag;

	if ( my_retflag==SSI_EDIT_OK ) ContactListUins.push_back ( uen );
	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::addContact ( string uin, string nick, string groupname, uint16_t * retflag )
{
	logMes_4 ( "ICQKid2::addContact" );
	if ( !startSSITransact() )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}

	if ( !sub_ssi_addContact ( uin, nick, groupname, retflag ) )
	{
		if ( !endSSITransact() && retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}

	int uen_ind = findCLUIN ( uin );
	if ( uen_ind>=0 )
	{
		uint32_t sync_id;
		if ( !updateSSIGroupContent ( ContactListUins[uen_ind].groupid, &sync_id ) )
		{
			if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
			return false;
		}
		uint16_t my_retflag;
		if ( !getSSIEditAck ( my_retflag, sync_id ) )
		{
			if ( retflag!=NULL ) *retflag=my_retflag;
			return false;
		}
	}

	if ( !endSSITransact() )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}

	if ( retflag!=SSI_EDIT_OK ) return false;
	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::sub_ssi_removeContact ( string uin, uint16_t * retflag )
{
	logMes_4 ( "sub_ssi_removeContact" );
	int uen_ind = findCLUIN ( uin );
	if ( uen_ind<0 )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NOTFOUND;
		return false;
	}

	uint16_t my_retflag;
	uint32_t sync_id;

	if ( !addDeleSSIUIN ( ContactListUins[uen_ind], SSI_ITEM_DELETE, &sync_id ) )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}

	if ( !getSSIEditAck ( my_retflag, sync_id ) )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}

	if ( retflag!=NULL ) *retflag=my_retflag;

	if ( my_retflag==SSI_EDIT_OK ) ContactListUins.erase ( ContactListUins.begin() +uen_ind );
	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::removeContact ( string uin, uint16_t * retflag )
{
	logMes_4 ( "ICQKid2::removeContact" );
	if ( !startSSITransact() )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}

	uint16_t gr_id=0;
	int uen_ind = findCLUIN ( uin );
	if ( uen_ind>=0 ) gr_id=ContactListUins[uen_ind].groupid;

	if ( !sub_ssi_removeContact ( uin, retflag ) )
	{
		if ( !endSSITransact() && retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}

	if ( gr_id>0 )
	{
		uint32_t sync_id;
		if ( !updateSSIGroupContent ( gr_id, &sync_id ) )
		{
			if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
			return false;
		}
		uint16_t my_retflag;
		if ( !getSSIEditAck ( my_retflag, sync_id ) )
		{
			if ( retflag!=NULL ) *retflag=my_retflag;
			return false;
		}
	}

	if ( !endSSITransact() )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}

	if ( retflag!=SSI_EDIT_OK ) return false;
	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::sub_ssi_renameContact ( string uin, string nick, uint16_t * retflag )
{
	logMes_4 ( "ICQKid2::sub_ssi_renameContact" );
	int uen_ind = findCLUIN ( uin );
	if ( uen_ind<0 )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NOTFOUND;
		return false;
	}

	// After my tests i think that we cant rename contacts
	// which awaiting auth, so we delete it and add once again
	if ( ContactListUins[uen_ind].waitauth )
	{
		string groupname=ContactListUins[uen_ind].groupname;
		if ( !sub_ssi_removeContact ( uin, retflag ) ) return false;
		return sub_ssi_addContact ( uin, nick, groupname, retflag );
	}

	SSIUINEntry uen=ContactListUins[uen_ind];


	uen.nick=nick;

	uint16_t my_retflag;
	uint32_t sync_id;

	if ( !addDeleSSIUIN ( uen, SSI_ITEM_CHANGE, &sync_id ) )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}
	if ( !getSSIEditAck ( my_retflag, sync_id ) )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}

	if ( retflag!=NULL ) *retflag=my_retflag;

	if ( my_retflag!=SSI_EDIT_OK ) ContactListUins[uen_ind]=uen;
	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::addContacts ( vector<SSITransactContact> & cont_vec )
{
	logMes_4 ( "ICQKid2::addContacts" );
	if ( !startSSITransact() )
	{
		for ( size_t i=0; i<cont_vec.size(); ++i ) cont_vec[i].retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}

	set<uint16_t> group_ids;
	for ( size_t i=0; i<cont_vec.size(); ++i )
	{
		if ( !sub_ssi_addContact ( cont_vec[i].uin, cont_vec[i].nick, cont_vec[i].groupname, &cont_vec[i].retflag ) ) continue;
		int uen_ind = findCLUIN ( cont_vec[i].uin );
		if ( uen_ind>=0 ) group_ids.insert ( ContactListUins[uen_ind].groupid );
	}

	for ( set<uint16_t>::iterator i=group_ids.begin(); i!=group_ids.end(); ++i )
	{
		uint32_t sync_id;
		if ( !updateSSIGroupContent ( *i, &sync_id ) )
		{
			for ( size_t j=0; j<cont_vec.size(); ++j ) cont_vec[j].retflag=SSI_EDIT_ERR_NETWORK;
			return false;
		}
		uint16_t retflag;
		if ( !getSSIEditAck ( retflag, sync_id ) )
		{
			for ( size_t j=0; j<cont_vec.size(); ++j ) cont_vec[j].retflag=retflag;
			return false;
		}
	}

	if ( !endSSITransact() )
	{
		for ( size_t i=0; i<cont_vec.size(); ++i ) cont_vec[i].retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::removeContacts ( vector<SSITransactContact> & cont_vec )
{
	logMes_4 ( "ICQKid2::removeContacts" );
	if ( !startSSITransact() )
	{
		for ( size_t i=0; i<cont_vec.size(); ++i ) cont_vec[i].retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}

	set<uint16_t> group_ids;
	for ( size_t i=0; i<cont_vec.size(); ++i )
	{
		int uen_ind = findCLUIN ( cont_vec[i].uin );
		if ( uen_ind>=0 ) group_ids.insert ( ContactListUins[uen_ind].groupid );
	}

	for ( size_t i=0; i<cont_vec.size(); ++i )
	{
		if ( !sub_ssi_removeContact ( cont_vec[i].uin, &cont_vec[i].retflag ) ) continue;
	}

	for ( set<uint16_t>::iterator i=group_ids.begin(); i!=group_ids.end(); ++i )
	{
		uint32_t sync_id;
		if ( !updateSSIGroupContent ( *i, &sync_id ) )
		{
			for ( size_t j=0; j<cont_vec.size(); ++j ) cont_vec[j].retflag=SSI_EDIT_ERR_NETWORK;
			return false;
		}
		uint16_t retflag;
		if ( !getSSIEditAck ( retflag, sync_id ) )
		{
			for ( size_t j=0; j<cont_vec.size(); ++j ) cont_vec[j].retflag=retflag;
			return false;
		}
	}

	if ( !endSSITransact() )
	{
		for ( size_t i=0; i<cont_vec.size(); ++i ) cont_vec[i].retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::renameContacts ( vector<SSITransactContact> & cont_vec )
{
	logMes_4 ( "ICQKid2::renameContacts" );
	if ( !startSSITransact() )
	{
		for ( size_t i=0; i<cont_vec.size(); ++i ) cont_vec[i].retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}

	set<uint16_t> group_ids;
	for ( size_t i=0; i<cont_vec.size(); ++i )
	{
		uint16_t it_id1=0;
		int uen_ind = findCLUIN ( cont_vec[i].uin );
		if ( uen_ind>=0 ) it_id1=ContactListUins[uen_ind].itemid;
		if ( !sub_ssi_renameContact ( cont_vec[i].uin, cont_vec[i].nick, &cont_vec[i].retflag ) ) continue;
		uint16_t it_id2=0;
		uen_ind = findCLUIN ( cont_vec[i].uin );
		if ( uen_ind>=0 )
		{
			it_id2=ContactListUins[uen_ind].itemid;
			if ( it_id1!=it_id2 ) group_ids.insert ( ContactListUins[uen_ind].groupid );
		}
	}

	for ( set<uint16_t>::iterator i=group_ids.begin(); i!=group_ids.end(); ++i )
	{
		uint32_t sync_id;
		if ( !updateSSIGroupContent ( *i, &sync_id ) )
		{
			for ( size_t j=0; j<cont_vec.size(); ++j ) cont_vec[j].retflag=SSI_EDIT_ERR_NETWORK;
			return false;
		}
		uint16_t retflag;
		if ( !getSSIEditAck ( retflag, sync_id ) )
		{
			for ( size_t j=0; j<cont_vec.size(); ++j ) cont_vec[j].retflag=retflag;
			return false;
		}
	}

	if ( !endSSITransact() )
	{
		for ( size_t i=0; i<cont_vec.size(); ++i ) cont_vec[i].retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::addBLMContact ( string uin )
{
	logMes_4 ( "ICQKid2::addBLMContact" );
	vector<string> uin_vec ( 1, uin );
	return addBLMContacts ( uin_vec );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::removeBLMContact ( string uin )
{
	logMes_4 ( "ICQKid2::removeBLMContact" );
	vector<string> uin_vec ( 1, uin );
	return removeBLMContacts ( uin_vec );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::addBLMContacts ( vector<string> & uins )
{
	logMes_4 ( "ICQKid2::addBLMContacts" );
	vector<uint8_t> data;
	size_t data_size=0;

	for ( size_t i=0; i<uins.size(); ++i )
	{
		data.resize ( data_size+uins[i].length() +1 );
		data[data_size]=uins[i].length();
		memcpy ( &data[data_size+1], uins[i].data(), uins[i].length() );
		data_size += uins[i].length() +1;
	}

	if ( sendSNAC ( 0x0003, 0x0004, NULL, &data ) !=1 ) return false;

	for ( size_t i=0; i<uins.size(); ++i )
	{
		int uen_ind=findCLUIN ( uins[i] );
		if ( uen_ind>=0 ) continue;
		SSIUINEntry uen;
		uen.uin=uins[i];
		uen.isBLM=true;
		ContactListUins.push_back ( uen );
	}

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::removeBLMContacts ( vector<string> & uins )
{
	logMes_4 ( "ICQKid2::removeBLMContacts" );
	vector<uint8_t> data;
	size_t data_size=0;

	for ( size_t i=0; i<uins.size(); ++i )
	{
		data.resize ( data_size+uins[i].length() +1 );
		data[data_size]=uins[i].length();
		memcpy ( &data[data_size+1], uins[i].data(), uins[i].length() );
		data_size += uins[i].length() +1;
	}

	if ( sendSNAC ( 0x0003, 0x0005, NULL, &data ) !=1 ) return false;

	for ( size_t i=0; i<uins.size(); /* empty */ )
	{
		int uen_ind=findCLUIN ( uins[i] );
		if ( uen_ind>=0 && ContactListUins[uen_ind].isBLM ) ContactListUins.erase ( ContactListUins.begin() +uen_ind );
		else ++i;
	}

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::addGroup ( string groupname, uint16_t * retflag )
{
	logMes_4 ( "ICQKid2::addGroup" );
	SSIGroupEntry gen;
	gen.id=getUnusedGroupID();
	if ( gen.id==0 )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_LIMIT;
		return false;
	}
	gen.name=groupname;
	if ( gen.name=="" ) // Name "" reserved for MasterGroup
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_CANTADD;
		return false;
	}

	uint16_t my_retflag;
	uint32_t sync_id;
	if ( !startSSITransact() )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}
	if ( !addDeleSSIGroup ( gen, SSI_ITEM_ADD, &sync_id ) )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}
	if ( !getSSIEditAck ( my_retflag, sync_id ) )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}
	if ( !updateSSIGroupContent ( 0, &sync_id ) )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}
	if ( !getSSIEditAck ( my_retflag, sync_id ) )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}
	if ( !endSSITransact() )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}

	if ( retflag!=NULL ) *retflag=my_retflag;
	if ( my_retflag!=SSI_EDIT_OK ) return false;

	ContactListGroups.push_back ( gen );
	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::removeGroup ( string groupname, uint16_t * retflag )
{
	logMes_4 ( "ICQKid2::removeGroup" );
	int gr_ind = findCLGroup ( groupname );
	if ( gr_ind<0 )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NOTFOUND;
		return false;
	}

	SSIGroupEntry gen=ContactListGroups[gr_ind];

	// Remove uins from this group
	vector<SSITransactContact> cont_vec;
	for ( size_t i=0; i<ContactListUins.size(); ++i )
	{
		if ( ContactListUins[i].groupid==gen.id )
		{
			SSITransactContact stc;
			stc.uin=ContactListUins[i].uin;
			stc.nick=ContactListUins[i].nick;
			stc.groupname=ContactListUins[i].groupname;
			cont_vec.push_back ( stc );
		}
	}

	if ( !removeContacts ( cont_vec ) )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}

	uint16_t my_retflag;
	uint32_t sync_id;
	if ( !startSSITransact() )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}
	if ( !addDeleSSIGroup ( gen, SSI_ITEM_DELETE, &sync_id ) )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}
	if ( !getSSIEditAck ( my_retflag, sync_id ) )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}
	if ( !updateSSIGroupContent ( 0, &sync_id ) )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}
	if ( !getSSIEditAck ( my_retflag, sync_id ) )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}
	if ( !endSSITransact() )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}

	if ( retflag!=NULL ) *retflag=my_retflag;
	if ( my_retflag!=SSI_EDIT_OK ) return false;

	ContactListGroups.erase ( ContactListGroups.begin() +gr_ind );
	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::renameGroup ( string groupname, string newname, uint16_t * retflag )
{
	logMes_4 ( "ICQKid2::renameGroup" );
	int gr_ind = findCLGroup ( groupname );
	if ( gr_ind<0 )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NOTFOUND;
		return false;
	}

	SSIGroupEntry gen=ContactListGroups[gr_ind];
	gen.name=newname;

	uint16_t my_retflag;
	uint32_t sync_id;
	if ( !startSSITransact() )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}
	if ( !addDeleSSIGroup ( gen, SSI_ITEM_CHANGE, &sync_id ) )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}
	if ( !getSSIEditAck ( my_retflag, sync_id ) )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}
	if ( !endSSITransact() )
	{
		if ( retflag!=NULL ) *retflag=SSI_EDIT_ERR_NETWORK;
		return false;
	}

	if ( retflag!=NULL ) *retflag=my_retflag;
	if ( my_retflag!=SSI_EDIT_OK ) return false;

	// Rename groupname in uins from this group
	for ( size_t i=0; i<ContactListUins.size(); ++i )
		if ( ContactListUins[i].groupid==gen.id )
			ContactListUins[i].groupname=newname;

	ContactListGroups[gr_ind].name=newname;
	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::updateContactList ( void )
{
	logMes_4 ( "ICQKid2::updateContactList" );
	uint32_t sync_id;
	if ( !askSSICopy ( &sync_id ) ) return false;
	if ( !getSSICopy ( sync_id ) ) return false;

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::authRequest ( string uin, string text )
{
	logMes_4 ( "ICQKid2::authRequest" );
	return sendAuthRequest ( uin, text );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::authReply ( string uin, string text, uint8_t aflag )
{
	logMes_4 ( "ICQKid2::authReply" );
	return sendAuthResponse ( uin, text, aflag );
}

void * pollKeepAlive ( void * )
{
	while ( !zgui->icq->network_break_flag )
	{
		sleep ( cfg_timeKeepConnect );
		if ( zgui->icq->network_break_flag )
			break;
		if ( ( time ( NULL )-zgui->icq->last_keepalive_timestamp ) > cfg_timeKeepConnect )
			zgui->icq->sendKeepAlive();
		logMes_4 ( "Keep connect" );
	}
	pthread_exit ( NULL );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::mainLoop()
{
	logMes_4 ( "mainLoop()" );

	pthread_attr_t attr;
	pthread_attr_init ( &attr );
	pthread_attr_setdetachstate ( &attr, PTHREAD_CREATE_DETACHED );
	pthread_create ( &keepConnect, &attr, pollKeepAlive, NULL );
	pthread_attr_destroy ( &attr );

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
int ICQKid2::findCLUIN ( string uin )
{
	logMes_4 ( "ICQKid2::findCLUIN "+QString ( uin.c_str() ) );
	for ( size_t i=0; i<ContactListUins.size(); ++i )
		if ( ContactListUins[i].uin==uin )
		{
			logMes_4 ( "finding contact %d",i );
			return i;
		}
	logMes_4 ( "notfinding contact" );
	return -1;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
int ICQKid2::findCLUIN ( uint16_t item_id )
{
	logMes_4 ( "ICQKid2::findCLUINс id %d", item_id );
	for ( size_t i=0; i<ContactListUins.size(); ++i )
		if ( ContactListUins[i].itemid==item_id )
			return i;

	return -1;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
int ICQKid2::findCLGroup ( string groupname )
{
	logMes_4 ( "ICQKid2::findCLGroup"+QString ( groupname.c_str() ) );
	for ( size_t i=0; i<ContactListGroups.size(); ++i )
		if ( ContactListGroups[i].name==groupname )
			return i;

	return -1;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
int ICQKid2::findCLGroup ( uint16_t group_id )
{
	logMes_4 ( "ICQKid2::findCLGroup id %d", group_id );
	for ( size_t i=0; i<ContactListGroups.size(); ++i )
		if ( ContactListGroups[i].id==group_id )
			return i;

	return -1;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::setMyPrivacyStatus ( uint8_t stat )
{
	logMes_4 ( "ICQKid2::setMyPrivacyStatus" );
	if ( stat<PRIV_ALL_CAN_SEE || stat>PRIV_CONTACTLIST_CAN_SEE ) return false;

	SSIContactListItem cli;
	cli.name="";
	cli.group_id=0;
	cli.type=0x0004;
	cli.payload.push_back ( TLVField ( stat, 0x00ca ) );

	uint16_t oper;
	if ( myPrivSrvStatus_item_id==0 ) { cli.item_id=getUnusedItemID(); oper=SSI_ITEM_ADD; }
	else { cli.item_id=myPrivSrvStatus_item_id; oper=SSI_ITEM_CHANGE; }

	vector<uint8_t> data;
	cli.encode_to ( data, 0 );
	uint32_t sync_id;
	if ( sendSNAC ( 0x0013, oper, &sync_id, &data ) !=1 ) return false;
	uint16_t retflag;
	if ( !getSSIEditAck ( retflag, sync_id ) ) return false;
	if ( retflag!=SSI_EDIT_OK ) return false;

	myPrivSrvStatus=stat;
	myPrivSrvStatus_item_id=cli.item_id;
	return true;
}

// --PRIVATE--PRIVATE--PRIVATE--PRIVATE--PRIVATE--PRIVATE--
// --PRIVATE--PRIVATE--PRIVATE--PRIVATE--PRIVATE--PRIVATE--
// --PRIVATE--PRIVATE--PRIVATE--PRIVATE--PRIVATE--PRIVATE--

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::getServiceList ( void )
{
	logMes_4 ( "ICQKid2::getServiceList" );
	SNACData snd;
	snd.service_id=0x0001;
	snd.subtype_id=0x0003;
	snd.req_id=0;
	if ( waitSNAC ( &snd ) !=1 ) return false;

	// Server send to me SNAC families, which it supports.
	size_t curr_pos=0;
	server_services_list.clear();
	
	size_t oldSize = 0;
	int i = 0;
	
	while ( i < 300 )
	{
		if ( oldSize == curr_pos )
			i++;
		else
		{
			oldSize = curr_pos;
			i=0;
		}
		if ( snd.data.size() < curr_pos ) return false;
		if ( snd.data.size() == curr_pos ) break;

		if ( snd.data.size() < ( curr_pos+2 ) ) return false;
		uint16_t serv_fam;
		memcpy ( &serv_fam, &snd.data[curr_pos], sizeof ( serv_fam ) );
		serv_fam=ntohs ( serv_fam );
		server_services_list.push_back ( pair<uint16_t, uint16_t> ( serv_fam, 1 ) );
		curr_pos+=sizeof ( serv_fam );
	}
	
	if ( i < 300 )
		return true;
	else
		return false;	
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::askServices ( void )
{
	logMes_4 ( "ICQKid2::askServices" );
	vector<uint8_t> ask_serv_vec;

// Ask for service only if one presents in server service list
#define TRY_ASK_SERVICE(arg_fam, arg_ver) { \
 bool service_found=false; \
 for (size_t i=0; i<server_services_list.size(); ++i) if (server_services_list[i].first==arg_fam) { service_found=true; break; } \
 if (service_found) \
  { \
  ask_serv_vec.push_back(0x00); \
  ask_serv_vec.push_back(arg_fam); \
  ask_serv_vec.push_back(0x00); \
  ask_serv_vec.push_back(arg_ver); \
  } \
 }
// END OF define TRY_ASK_SERVICE

	TRY_ASK_SERVICE ( 0x01, 0x04 ) // AIM Generic
	TRY_ASK_SERVICE ( 0x13, 0x04 ) // SSI
	TRY_ASK_SERVICE ( 0x02, 0x01 ) // AIM Location
	TRY_ASK_SERVICE ( 0x03, 0x01 ) // AIM BuddyList (BLM)
	TRY_ASK_SERVICE ( 0x15, 0x01 ) // AIM ICQ
	TRY_ASK_SERVICE ( 0x04, 0x01 ) // AIM Messaging (ICBM)
	TRY_ASK_SERVICE ( 0x09, 0x01 ) // AIM BOS, Privacy managment (PRM)
	TRY_ASK_SERVICE ( 0x10, 0x01 ) // AIM Stored Buddy Icons (SST, SSBI)

#undef TRY_ASK_SERVICE

	return ( sendSNAC ( 0x0001, 0x0017, NULL, &ask_serv_vec ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::getServices ( void )
{
	logMes_4 ( "ICQKid2::getServices" );
	SNACData snd;
	snd.service_id=0x0001;
	snd.subtype_id=0x0018;
	snd.req_id=0;
	if ( waitSNAC ( &snd ) !=1 ) return false;

	// Server returns finally SNAC families, whit will be used in our session.
	size_t curr_pos=0;
	server_services_list.clear();
	
	size_t oldSize = 0;
	int i = 0;
	
	while ( i < 300 )
	{
		if ( oldSize == curr_pos)
			i++;
		else
		{
			oldSize = curr_pos;
			i=0;
		}
		
		if ( snd.data.size() <  curr_pos ) return false;
		if ( snd.data.size() == curr_pos ) break;

		if ( snd.data.size() < ( curr_pos+2 ) ) return false;
		uint16_t serv_fam;
		memcpy ( &serv_fam, &snd.data[curr_pos], sizeof ( serv_fam ) );
		serv_fam=ntohs ( serv_fam );
		curr_pos+=sizeof ( serv_fam );

		if ( snd.data.size() < ( curr_pos+2 ) ) return false;
		uint16_t serv_ver;
		memcpy ( &serv_ver, &snd.data[curr_pos], sizeof ( serv_ver ) );
		serv_ver=ntohs ( serv_ver );
		server_services_list.push_back ( pair<uint16_t, uint16_t> ( serv_fam, serv_ver ) );
		curr_pos+=sizeof ( serv_ver );
	}
	
	if (i<300)
		return true;
	else
		return false;	
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::askRateLimits ( uint32_t * sync_id )
{
	logMes_4 ( "ICQKid2::askRateLimits" );
	return ( sendSNAC ( 0x0001, 0x0006, sync_id ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::getRateLimits ( uint32_t sync_id )
{
	logMes_4 ( "ICQKid2::getRateLimits" );
	SNACData snd;
	snd.service_id=0x0001;
	snd.subtype_id=0x0007;
	snd.req_id=sync_id;
	if ( waitSNAC ( &snd ) !=1 ) return false;

	// Server sends to me rate limits.
	// I need to parse and remember it, because i need send
	// back rate groups later
	size_t curr_pos=0;

	if ( snd.data.size() < ( curr_pos+2 ) ) return false;
	uint16_t class_count;
	memcpy ( &class_count, &snd.data[curr_pos], sizeof ( class_count ) );
	class_count=ntohs ( class_count );
	curr_pos+=sizeof ( class_count );

	// Skip rate classes, we dont need it
	if ( snd.data.size() < ( curr_pos+class_count*35 ) ) return false;
	curr_pos+=class_count*35;

	server_rate_groups.clear();
	
	size_t oldSize = 0;
	int i = 0;
	
	while ( i < 300 )
	{
		if ( oldSize == curr_pos)
			i++;
		else
		{
			oldSize = curr_pos;
			i=0;
		}
		if ( snd.data.size() <  curr_pos ) return false;
		if ( snd.data.size() == curr_pos ) break;

		if ( snd.data.size() < ( curr_pos+4 ) ) return false;
		uint16_t group_id, pair_count;
		memcpy ( &group_id, &snd.data[curr_pos], sizeof ( group_id ) );
		group_id=ntohs ( group_id );
		curr_pos+=sizeof ( group_id );
		memcpy ( &pair_count, &snd.data[curr_pos], sizeof ( pair_count ) );
		pair_count=ntohs ( pair_count );
		curr_pos+=sizeof ( pair_count );

		if ( snd.data.size() < ( curr_pos+pair_count*4 ) ) return false;
		curr_pos+=pair_count*4;
		server_rate_groups.push_back ( group_id );
	}
	
	if ( i < 300 )
		return true;
	else
		return false;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::sendRateLimits ( void )
{
	logMes_4 ( "ICQKid2::sendRateLimits" );
	// I send rate groups back to server
	vector<uint8_t> vec ( server_rate_groups.size() *2 );
	size_t curr_pos=0;
	for ( unsigned int i=0; i<server_rate_groups.size(); ++i )
	{
		uint16_t group_id = htons ( server_rate_groups[i] );
		memcpy ( &vec[curr_pos], &group_id, 2 );
		curr_pos+=2;
	}

	return ( sendSNAC ( 0x0001, 0x0008, NULL, &vec ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::askSelfInfo ( uint32_t * sync_id )
{
	logMes_4 ( "ICQKid2::askSelfInfo" );
	return ( sendSNAC ( 0x0001, 0x000e, sync_id ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::getSelfInfo ( uint32_t sync_id )
{
	logMes_4 ( "ICQKid2::getSelfInfo" );
	SNACData snd;
	snd.service_id=0x0001;
	snd.subtype_id=0x000f;
	snd.req_id=sync_id;
	if ( waitSNAC ( &snd ) !=1 ) return false;
	// FIXME - parse selfinfo
	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::askLocationLimit ( uint32_t * sync_id )
{
	logMes_4 ( "ICQKid2::askLocationLimit" );
	return ( sendSNAC ( 0x0002, 0x0002, sync_id ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::getLocationLimit ( uint32_t sync_id )
{
	/*
	We have TLVPack in SNAC data, but dont need in it.
	There are tlvs like [Client max profile lenght],
	[Max capabilities (CLSIDs)] and another, in this pack.
	*/
	logMes_4 ( "ICQKid2::getLocationLimit" );
	SNACData snd;
	snd.service_id=0x0002;
	snd.subtype_id=0x0003;
	snd.req_id=sync_id;
	return ( waitSNAC ( &snd ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::sendLocationInfo ( void )
{
	logMes_4 ( "ICQKid2::sendLocationInfo" );
	uint8_t aim2icq_capa[] =
	{ 
		0x09, 0x46, 0x13, 0x4e, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00,   /* ICQ UTF-8 */
		0x09, 0x46, 0x13, 0x49, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00,   /* ICQ Server relaying */
		0x56, 0x3f, 0xc8, 0x09, 0x0b, 0x6f, 0x41, 0xbd, 0x9f, 0x79, 0x42, 0x26, 0x09, 0xdf, 0xa2, 0xf3,   /* Mini Typing Notification */
		0x1A, 0x09, 0x3C, 0x6C, 0xD7, 0xFD, 0x4E, 0xC5, 0x9D, 0x51, 0xA6, 0x47, 0x4E, 0x34, 0xF5, 0xA0,   /* XtraZ */
		0x09, 0x46, 0x13, 0x4d, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00,   /* AIM_ICQGATE */		
		0x09, 0x46, 0x00, 0x00, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00,   /* tZers */
		//0x09, 0x46, 0x00, 0x00, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00,   /* NEW CAPS */		
		
		//Client ID string
		#if defined(EZX_Z6)
		'z', 'I', 'M', ' ', 'Z', '6', ' ', '0'+VER_MAJOR, '.', '0'+VER_MINOR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		#elif defined(EZX_ZN5)
		'z', 'I', 'M', ' ', 'Z', 'N', '5',' ', '0'+VER_MAJOR, '.', '0'+VER_MINOR, 0x00, 0x00, 0x00, 0x00, 0x00,
		#elif defined(EZX_U9)
		'z', 'I', 'M', ' ', 'U', '9', ' ', '0'+VER_MAJOR, '.', '0'+VER_MINOR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		#elif defined(EZX_E8)
		'z', 'I', 'M', ' ', 'E', '8', ' ', '0'+VER_MAJOR, '.', '0'+VER_MINOR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		#elif defined(EZX_EM30)
		'z', 'I', 'M', ' ', 'E', 'M', '3', '0', ' ', '0'+VER_MAJOR, '.', '0'+VER_MINOR, 0x00, 0x00, 0x00, 0x00,
		#elif defined(EZX_EM35)
		'z', 'I', 'M', ' ', 'E', 'M', '3', '5', ' ', '0'+VER_MAJOR, '.', '0'+VER_MINOR, 0x00, 0x00, 0x00, 0x00,
		#elif defined(EZX_VE66)
		'z', 'I', 'M', ' ', 'V', 'E', '6', '6', ' ', '0'+VER_MAJOR, '.', '0'+VER_MINOR, 0x00, 0x00, 0x00, 0x00,
		#elif defined(EZX_V8)
		'z', 'I', 'M', ' ', 'V', '*', ' ', '0'+VER_MAJOR, '.', '0'+VER_MINOR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		#else
		'z', 'I', 'M', ' ', 'M', 'A', 'G', 'X', ' ', '0'+VER_MAJOR, '.', '0'+VER_MINOR, 0x00, 0x00, 0x00, 0x00,
		#endif
	};

	vector<uint8_t> vec ( aim2icq_capa, aim2icq_capa+sizeof ( aim2icq_capa ) );
	if ( xStatus!=X_STATUS_NONE && xStatus<=X_STATUS_MAX_BOUND )
		vec.insert ( vec.end(), XStatus_arr[xStatus-1], XStatus_arr[xStatus-1]+16 );

	TLVField tlv ( vec, 0x0005 ); // CLSID values
	vec.clear();
	tlv.encode_to ( vec, 0 );

	return ( sendSNAC ( 0x0002, 0x0004, NULL, &vec ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::askBLMLimit ( uint32_t * sync_id )
{
	logMes_4 ( "ICQKid2::askBLMLimit" );
	return ( sendSNAC ( 0x0003, 0x0002, sync_id ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::getBLMLimit ( uint32_t sync_id )
{
	/*
	We have TLVPack in SNAC data, but dont need in it.
	There are tlvs like [Max number of contact list entries],
	[Max number of watcher list entries], [Max online notifications] and another, in this pack.
	*/
	logMes_4 ( "ICQKid2::getBLMLimit" );
	SNACData snd;
	snd.service_id=0x0003;
	snd.subtype_id=0x0003;
	snd.req_id=sync_id;
	return ( waitSNAC ( &snd ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::askICBMParams ( uint32_t * sync_id )
{
	logMes_4 ( "ICQKid2::askICBMParams" );
	return ( sendSNAC ( 0x0004, 0x0004, sync_id ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::getICBMParams ( uint32_t sync_id )
{
// Server sends to me ICBM parameters, i parse it for example.
// Shot form will be simple return waitSNAC(0x0004, 0x0005);
	logMes_4 ( "ICQKid2::getICBMParams" );
	SNACData snd;
	snd.service_id=0x0004;
	snd.subtype_id=0x0005;
	snd.req_id=sync_id;
	if ( waitSNAC ( &snd ) !=1 ) return false;

	ICBMParameters icbm_par;
	if ( !icbm_par.decode_from ( snd.data ) ) return false;

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::sendICBMParams ( void )
{
	logMes_4 ( "ICQKid2::sendICBMParams" );
	// I set my own ICBM parameters for each of channels
	ICBMParameters icbm_par;
	icbm_par.channel=0x0001;
	icbm_par.msg_flags=0x0000000b;	// bit1: messages allowed for specified channel
									// bit2: missed calls notifications enabled for specified channel
									// bit4: client supports typing notifications
	icbm_par.max_snac_size=0x1f40; // 8000 bytes
	icbm_par.max_send_warn_lev=0x03e7; // 999
	icbm_par.max_recv_warn_lev=0x03e7; // 999
	icbm_par.min_msg_interval=0; // msec

	vector<uint8_t> vec;
	icbm_par.encode_to ( vec );

	if ( sendSNAC ( 0x0004, 0x0002, NULL, &vec ) !=1 ) return false;

	vec.clear();
	icbm_par.channel=0x0002;
	icbm_par.msg_flags=0x00000003;
	icbm_par.max_snac_size=0x1f40; // 8000 bytes
	icbm_par.max_send_warn_lev=0x03e7; // 999
	icbm_par.max_recv_warn_lev=0x03e7; // 999
	icbm_par.min_msg_interval=0; // msec
	icbm_par.encode_to ( vec );
	if ( sendSNAC ( 0x0004, 0x0002, NULL, &vec ) !=1 ) return false;

	vec.clear();
	icbm_par.channel=0x0004;
	icbm_par.msg_flags=0x00000003;
	icbm_par.max_snac_size=0x1f40; // 8000 bytes
	icbm_par.max_send_warn_lev=0x03e7; // 999
	icbm_par.max_recv_warn_lev=0x03e7; // 999
	icbm_par.min_msg_interval=0; // msec
	icbm_par.encode_to ( vec );
	if ( sendSNAC ( 0x0004, 0x0002, NULL, &vec ) !=1 ) return false;

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::askPRMLimit ( uint32_t * sync_id )
{
	logMes_4 ( "ICQKid2::askPRMLimit" );
	return ( sendSNAC ( 0x0009, 0x0002, sync_id ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::getPRMLimit ( uint32_t sync_id )
{
	/*
	We have TLVPack in SNAC data, but dont need in it.
	There are tlvs like [Max number of visible list entries] and
	[Max number of invisible list entries], in this pack.
	*/
	logMes_4 ( "ICQKid2::getPRMLimit" );
	SNACData snd;
	snd.service_id=0x0009;
	snd.subtype_id=0x0003;
	snd.req_id=sync_id;
	return ( waitSNAC ( &snd ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::askSSILimit ( uint32_t * sync_id )
{
	logMes_4 ( "ICQKid2::askSSILimit" );
	return ( sendSNAC ( 0x0013, 0x0002, sync_id ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::getSSILimit ( uint32_t sync_id )
{
	/*
	We have TLVPack in SNAC data, but dont need in it.
	There are tlvs like some SSI contact list limitations in this pack.
	*/
	logMes_4 ( "ICQKid2::getSSILimit" );
	SNACData snd;
	snd.service_id=0x0013;
	snd.subtype_id=0x0003;
	snd.req_id=sync_id;
	return ( waitSNAC ( &snd ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::askSSICopy ( uint32_t * sync_id )
{
	logMes_4 ( "ICQKid2::askSSICopy" );
// Not 13,05 - we havent got local copy for update it!
	return ( sendSNAC ( 0x0013, 0x0004, sync_id ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::getSSICopy ( uint32_t sync_id )
{
	logMes_4 ( "start getSSICopy" );
	SNACData snd;
	snd.service_id=0x0013;
	snd.subtype_id=0x0006;
	snd.flags=0x0001;
	snd.req_id=sync_id;

	SSIContactList cl;

	while ( snd.flags&0x0001 ) // Incomplete SNAC, it has continue in next
	{
		if ( waitSNAC ( &snd ) !=1 )
		{
			logMes_4 ( ">>>>>>>>>>>>>Time out" );
			return false;
		}
		if ( !cl.decode_from ( snd.data ) )
		{
			logMes_4 ( ">>>>>>>>>>>>>Decode erorr" );
			return false;
		}
	}

	logMes_4 ( "Clear only SSI contacts from list" );
// Clear only SSI contacts from list

	for ( size_t i=0; i<ContactListUins.size(); /* empty */ )
	{
		if ( !ContactListUins[i].isBLM ) ContactListUins.erase ( ContactListUins.begin() +i );
		else ++i;
	}

	ContactListGroups.clear();
	VisibleList.clear();
	InvisibleList.clear();
	IgnoreList.clear();

	for ( unsigned int i=0; i<cl.items.size(); ++i )
	{
		switch ( cl.items[i].type )
		{
			case 0x0000 : // Buddy record
			{
				SSIUINEntry uen;
				uen.uin=cl.items[i].name;
				uen.groupid=cl.items[i].group_id;
				uen.itemid=cl.items[i].item_id;
				uen.nick="";
				uen.groupname="";
				uen.waitauth=false;
				for ( unsigned int j=0; j<cl.items[i].payload.size(); ++j )
				{
					if ( cl.items[i].payload[j].type==0x0131 ) cl.items[i].payload[j].getAsString ( uen.nick );
					if ( cl.items[i].payload[j].type==0x0066 ) uen.waitauth=true;
				}
				ContactListUins.push_back ( uen );
			}
			break;

			case 0x0001 : // Group record
			{
				if ( cl.items[i].group_id==0x0000 ) break; // We dont need to virtual MasterGroup with group_id=0
				SSIGroupEntry gen;
				gen.id=cl.items[i].group_id;
				gen.name=cl.items[i].name;
				ContactListGroups.push_back ( gen );
			}
			break;

			case 0x0014 : // We have Icon
			break;

			case 0x0004 : // Privacy status
			{
				myPrivSrvStatus_item_id=cl.items[i].item_id;
				for ( unsigned int j=0; j<cl.items[i].payload.size(); ++j )
					if ( cl.items[i].payload[j].type==0x00CA && !cl.items[i].payload[j].data.empty() )
						myPrivSrvStatus=cl.items[i].payload[j].data[0];
			}
			break;

			case 0x0002 : // Visible list item
			{
				SSIUINEntry uen;
				uen.uin=cl.items[i].name;
				uen.groupid=cl.items[i].group_id;
				uen.itemid=cl.items[i].item_id;
				for ( unsigned int j=0; j<cl.items[i].payload.size(); ++j )
					if ( cl.items[i].payload[j].type==0x0131 ) cl.items[i].payload[j].getAsString ( uen.nick );

				VisibleList.push_back ( uen );
			}
			break;

			case 0x0003 : // Invisible list item
			{
				SSIUINEntry uen;
				uen.uin=cl.items[i].name;
				uen.groupid=cl.items[i].group_id;
				uen.itemid=cl.items[i].item_id;
				for ( unsigned int j=0; j<cl.items[i].payload.size(); ++j )
					if ( cl.items[i].payload[j].type==0x0131 ) cl.items[i].payload[j].getAsString ( uen.nick );

				InvisibleList.push_back ( uen );
			}
			break;

			case 0x000E : // Ignore list item
			{
				SSIUINEntry uen;
				uen.uin=cl.items[i].name;
				uen.groupid=cl.items[i].group_id;
				uen.itemid=cl.items[i].item_id;
				for ( unsigned int j=0; j<cl.items[i].payload.size(); ++j )
					if ( cl.items[i].payload[j].type==0x0131 ) cl.items[i].payload[j].getAsString ( uen.nick );

				IgnoreList.push_back ( uen );
			}
			break;

			default :
			{
				// We need to store unknown items for correct work getUnusedItemID()
				unknown_item_ids.insert(cl.items[i].item_id);
			}
		}
	}

	for ( unsigned int i=0; i<ContactListUins.size(); ++i )
	{
		int gr_ind=findCLGroup ( ContactListUins[i].groupid );
		if ( gr_ind>=0 ) ContactListUins[i].groupname=ContactListGroups[gr_ind].name;
	}

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::activateSSI ( void )
{
	logMes_4 ( "ICQKid2::activateSSI" );
	return ( sendSNAC ( 0x0013, 0x0007 ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::sendStatus ( uint32_t astat, bool /*ext*/ )
{
	logMes_4 ( "ICQKid2::sendStatus" );
	vector<uint8_t> vec;
	TLVPack tlvp;

	tlvp.data.push_back ( TLVField ( astat, 0x0006 ) ); // Online status
	
	/*if ( ext ) 
	{
		//tlvp.data.push_back ( TLVField ( ( uint16_t ) 0x0000, 0x0008 ) ); // Unknown

		
		uint8_t dc_info[] =
		{
			0x00, 0x00, 0x00, 0x00,  // Internal IP address
			0x00, 0x00, 0x00, 0x00,  // Internal TCP port 
			//0x04,  					// DC type - DC not possible 
			0x06,  					// DC type - DC not possible 
			0x00, 0x0A,				// DC protocol version - ICQ Lite (9) 
			0x0a, 0x0b, 0x33, 0x45,  // DC Auth cookie 
			0x00, 0x00, 0x0b, 0xf5,  // Web Front Port 
			0x00, 0x00, 0x00, 0x01,  // Client futures 
			0x00, 0x00, 0x00, 0x00,  // Last Info Update 
			0x00, 0x00, 0x00, 0x00,  // Last Extended Info Update (i.e. icqphone status) 
			0x00, 0x00, 0x00, 0x00,  // Last Extended Status Update (i.e. phonebook) 
			0x00, 0x00
		};  //Unknown 
		tlvp.data.push_back ( TLVField ( dc_info, sizeof ( dc_info ), 0x000c ) ); // DC Info
		

		//if ( ext ) 
		//tlvp.data.push_back ( TLVField ( ( uint16_t ) 0x0000, 0x001f ) ); // Unknown		
	}*/

	tlvp.encode_to ( vec );
	if ( sendSNAC ( 0x0001, 0x001e, NULL, &vec ) !=1 ) return false;

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::sendReady ( void )
{
	logMes_4 ( "ICQKid2::sendReady" );
// Send to server SNAC families and versions, and tool ids and versions
	vector<uint8_t> ready_serv_vec;

#define TRY_READY_SERVICE(arg_fam) { \
 bool service_found=false; \
 uint16_t service_version; \
 for (size_t i=0; i<server_services_list.size(); ++i) \
  if (server_services_list[i].first==arg_fam) \
   { \
   service_version=server_services_list[i].second; \
   service_found=true; \
   break; \
   } \
 if (service_found) \
  { \
  ready_serv_vec.push_back(0x00); \
  ready_serv_vec.push_back(arg_fam); \
  ready_serv_vec.push_back(0x00); \
  ready_serv_vec.push_back(service_version); \
  ready_serv_vec.push_back(0x01); \
  ready_serv_vec.push_back(0x10); \
  ready_serv_vec.push_back(0x08); \
  ready_serv_vec.push_back(0xe4); \
  } \
 }
// END OF define TRY_READY_SERVICE

	TRY_READY_SERVICE ( 0x01 ) // AIM Generic
	TRY_READY_SERVICE ( 0x13 ) // SSI
	TRY_READY_SERVICE ( 0x02 ) // AIM Location
	TRY_READY_SERVICE ( 0x03 ) // AIM BuddyList (BLM)
	TRY_READY_SERVICE ( 0x15 ) // AIM ICQ
	TRY_READY_SERVICE ( 0x04 ) // AIM Messaging (ICBM)
	TRY_READY_SERVICE ( 0x09 ) // AIM BOS, Privacy managment (PRM)
	//TRY_READY_SERVICE ( 0x10 ) // AIM Stored Buddy Icons (SST, SSBI) //!!!!!!!!!!!

#undef TRY_READY_SERVICE

	return ( sendSNAC ( 0x0001, 0x0002, NULL, &ready_serv_vec ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
int ICQKid2::directConnect(string ahost, int aport)
{
	logMes_4 ( "directConnect(%s, %d)",ahost.c_str(),aport );

	struct hostent * he;
	sockaddr_in addr;
	int tmp_sock;

	if ((he=gethostbyname(ahost.c_str()))==NULL) return -1;
	if (he->h_addr==NULL) return -1;

	memset(&addr, 0, sizeof(addr));
	memcpy(&(addr.sin_addr.s_addr), he->h_addr, sizeof(addr.sin_addr.s_addr));
	addr.sin_port=htons(aport);
	addr.sin_family=PF_INET;

	if ((tmp_sock=socket(PF_INET, SOCK_STREAM, 0))==-1) return -1;

	logMes_4 ( "bind" );

	__napi_bindsocket2link(tmp_sock);

	logMes_4 ( "Moto connect true" );

	int conn_ret;
	while ((conn_ret=t_connect(tmp_sock, (const struct sockaddr *)&addr, (socklen_t)sizeof(addr), 100))!=0)
		if (conn_ret!=TNETWORK_TIMEOUT)
		{
			CLOSE_SOCK(tmp_sock);
			return -1;
		}

	return tmp_sock;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::waitHello ( void )
{
	logMes_4 ( "ICQKid2::waitHello" );
	FlapPacket fp;
	
	while ( true )
	{
		if ( fp.recv_from ( sock ) ) break;
		if ( fp.t_network_error!=TNETWORK_TIMEOUT ) return false;
	}

	if ( fp.frame_type!=FT_SIGNON ) return false;
	SignOnPacket sp;
	if ( !sp.decode_from ( fp.payload ) ) return false;

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::sendSignOn0 ( void )
{
	logMes_4 ( "ICQKid2::sendSignOn0" );
	FlapPacket fp;
	fp.frame_type=FT_SIGNON;
	fp.seq_number=flap_seq_number++;
	fp.payload.resize ( 4 );
	fp.payload[3]=1;

	int i=0;
	while ( i<500 )
	{
		i++;
		if ( fp.send_to ( sock ) ) return true;
		if ( fp.t_network_error!=TNETWORK_TIMEOUT ) return false;
	}

	return false;	
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::sendMD5saltrequest ( uint32_t * snac_sync )
{
	logMes_4 ( "ICQKid2::sendMD5saltrequest" );
	TLVPack tlv_pack;
	tlv_pack.data.push_back ( TLVField ( myuin, TLV_UID ) );
	tlv_pack.data.push_back ( TLVField ( NULL, 0, 0x004b ) ); // Unknown, empty
	tlv_pack.data.push_back ( TLVField ( NULL, 0, 0x005a ) ); // Unknown, empty

	vector<uint8_t> data_vec;
	tlv_pack.encode_to ( data_vec );

	if ( sendSNAC ( 0x0017, 0x0006, snac_sync, &data_vec ) !=1 ) return false;

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::getMD5salt ( uint32_t snac_sync, vector<uint8_t> & md5_salt )
{
	logMes_4 ( "ICQKid2::getMD5salt" );
	SNACData snd;
	snd.service_id=0x0017;
	snd.subtype_id=0x0000;
	snd.req_id=snac_sync;
	if ( waitSNAC ( &snd ) !=1 ) return false;

	if ( snd.subtype_id==0x0003 ) // Error
	{
		TLVPack tlv_pack;
		if ( tlv_pack.decode_from ( snd.data ) )
		{
			TLVField * err_tlv = tlv_pack.findTLV ( 0x0008 );
			if ( err_tlv!=NULL ) err_tlv->getAsInt16 ( connect_error_code );
			TLVField * err_url_tlv = tlv_pack.findTLV ( 0x0004 );
			if ( err_url_tlv!=NULL ) err_url_tlv->getAsString ( connect_error_url );
		}
		return false;
	}

	if ( snd.subtype_id!=0x0007 ) return false; // WTF?

	if ( snd.data.size() <5 ) return false;
	uint16_t auth_key_len = ntohs ( * ( ( uint16_t* ) ( &snd.data[0] ) ) );
	if ( snd.data.size() < ( auth_key_len+sizeof ( auth_key_len ) ) ) return false;
	md5_salt.assign ( snd.data.begin() +sizeof ( auth_key_len ), snd.data.begin() +sizeof ( auth_key_len ) +auth_key_len );

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::sendMD5authorize ( uint32_t * snac_sync, vector<uint8_t> & md5_salt )
{
	logMes_4 ( "ICQKid2::sendMD5authorize" );
	TLVPack tlv_pack;
	tlv_pack.data.push_back ( TLVField ( myuin, TLV_UID ) );
	tlv_pack.data.push_back ( TLVField ( "zIM", TLV_CLI_STR_ID ) ); // Client ID string

	vector<uint8_t> auth_sum=md5_salt;
	if ( mypassword.length() >8 ) auth_sum.insert ( auth_sum.end(), ( uint8_t* ) mypassword.data(), ( ( uint8_t* ) mypassword.data() ) +8 ); // All passwords were cutted to 8 chars
	else auth_sum.insert ( auth_sum.end(), ( uint8_t* ) mypassword.data(), ( ( uint8_t* ) mypassword.data() ) +mypassword.length() );
	auth_sum.insert ( auth_sum.end(), ( uint8_t* ) AOL_SALT_STR, ( ( uint8_t* ) AOL_SALT_STR ) +sizeof ( AOL_SALT_STR )-1 );

	uint8_t md5_hash[16]={0};
	calculate_md5 ( ( const char* ) ( &auth_sum[0] ), auth_sum.size(), ( char * ) md5_hash );

	tlv_pack.data.push_back ( TLVField ( md5_hash, 16, 0x0025 ) ); // MD5 hash of salt+password+AOL_SALT_STR
	tlv_pack.data.push_back ( TLVField ( ( uint16_t ) 0x010a, TLV_CLI_NUM_ID ) ); // Client ID number
	tlv_pack.data.push_back ( TLVField ( ( uint16_t ) 0x0000, TLV_CLI_VER_MAJOR ) ); // Client major version
	tlv_pack.data.push_back ( TLVField ( ( uint16_t ) 0x0000, TLV_CLI_VER_MINOR ) ); // Client minor version
	tlv_pack.data.push_back ( TLVField ( ( uint16_t ) 0x0000, TLV_CLI_VER_LESSER ) ); // Client lesser version
	tlv_pack.data.push_back ( TLVField ( ( uint16_t ) 0x0000, TLV_CLI_NUM_BUILD ) ); // Client build number
	tlv_pack.data.push_back ( TLVField ( ( uint32_t ) 0x0000043d, TLV_CLI_NUM_DISTR ) ); // Client distribution number
	tlv_pack.data.push_back ( TLVField ( "ru", TLV_CLI_LANG ) ); // Client language
	tlv_pack.data.push_back ( TLVField ( "ru", TLV_CLI_COUNTRY ) ); // Client country
	tlv_pack.data.push_back ( TLVField ( ( uint8_t ) 0x03,  TLV_SSL_FLAG ) );
// skip TLV 0x4a - SSI flag: 1 - SSI only, 0 - family 0x03
// Versions of componets made like icq 5.1, otherwise avatars and xStatus couldn't work

	vector<uint8_t> data_vec;
	tlv_pack.encode_to ( data_vec );
	if ( sendSNAC ( 0x0017, 0x0002, snac_sync, &data_vec ) !=1 ) return false;

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::getBOSSparams ( uint32_t snac_sync, string & boss_host, int & boss_port, vector<uint8_t> & cookie )
{
	logMes_4 ( "ICQKid2::getBOSSparams" );
	SNACData snd;
	snd.service_id=0x0017;
	snd.subtype_id=0x0003;
	snd.req_id=snac_sync;
	if ( waitSNAC ( &snd ) !=1 ) return false;

	TLVPack tlv_pack;
	if ( !tlv_pack.decode_from ( snd.data ) ) return false;
	TLVField * err_tlv = tlv_pack.findTLV ( 0x0008 );
	if ( err_tlv!=NULL ) // Packet contains error code
	{
		err_tlv->getAsInt16 ( connect_error_code );
		TLVField * err_url_tlv = tlv_pack.findTLV ( 0x0004 );
		if ( err_url_tlv!=NULL ) err_url_tlv->getAsString ( connect_error_url );
		return false;
	}

	TLVField * tlv_ptr = tlv_pack.findTLV ( TLV_BOSS_ADDR );
	if ( tlv_ptr==NULL ) return false;
	string boss;
	if ( !tlv_ptr->getAsString ( boss ) ) return false;
	string::size_type i = boss.find ( ":" );
	if ( i==boss.npos || i>= ( boss.length()-1 ) ) return false;
	boss_host.assign ( boss, 0, i );
	boss_port=atoi ( string ( boss, i+1 ).c_str() );
	tlv_ptr = tlv_pack.findTLV ( TLV_AUTH_COOKIE );
	if ( tlv_ptr==NULL ) return false;
	cookie=tlv_ptr->data;

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::sendSignOff ( void )
{
	logMes_4 ( "ICQKid2::sendSignOff" );
	FlapPacket fp;
	fp.frame_type=FT_SIGNOFF;
	fp.seq_number=flap_seq_number++;

	int i=0;
	while ( i<300 )
	{
		i++;
		if ( fp.send_to ( sock ) ) return true;
		if ( fp.t_network_error!=TNETWORK_TIMEOUT ) return false;
	}

	return false;	
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::sendSignOn2 ( vector<uint8_t> & cookie )
{
	logMes_4 ( "ICQKid2::sendSignOn2" );
	FlapPacket fp;
	fp.frame_type=FT_SIGNON;
	fp.seq_number=flap_seq_number++;

	SignOnPacket sp;
	sp.data.push_back ( TLVField ( cookie, TLV_AUTH_COOKIE ) );
	sp.encode_to ( fp.payload );

	int i=0;
	while ( i<500 )
	{
		i++;
		if ( fp.send_to ( sock ) ) return true;
		if ( fp.t_network_error!=TNETWORK_TIMEOUT ) return false;
	}
	
	return false;	
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::addVisInvisIgnor ( vector<SSIUINEntry> & cont, uint16_t it_type, string uin, string nick )
{
	logMes_4 ( "ICQKid2::addVisInvisIgnor" );
	int cont_ind=-1;
	for ( size_t i=0; i<cont.size(); ++i )
		if ( cont[i].uin==uin ) { cont_ind=i; break; }

	if ( cont_ind>=0 && cont[cont_ind].nick==nick ) return true;

	SSIContactListItem cli;
	cli.name=uin;
	cli.group_id=0;
	cli.type=it_type;
	if ( nick!="" ) cli.payload.push_back ( TLVField ( nick, 0x0131 ) );

	uint16_t oper;
	if ( cont_ind<0 ) { cli.item_id=getUnusedItemID(); oper=SSI_ITEM_ADD; }
	else { cli.item_id=cont[cont_ind].itemid; oper=SSI_ITEM_CHANGE; }

	vector<uint8_t> data;
	cli.encode_to ( data, 0 );
	uint32_t sync_id;
	if ( sendSNAC ( 0x0013, oper, &sync_id, &data ) !=1 ) return false;
	uint16_t retflag;
	if ( !getSSIEditAck ( retflag, sync_id ) ) return false;
	if ( retflag!=SSI_EDIT_OK ) return false;

	if ( cont_ind<0 )
	{
		SSIUINEntry uen;
		uen.uin=uin;
		uen.groupid=0;
		uen.itemid=cli.item_id;
		uen.nick=nick;
		cont.push_back ( uen );
	}
	else
	{
		cont[cont_ind].nick=nick;
	}
	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::removeVisInvisIgnor ( vector<SSIUINEntry> & cont, uint16_t it_type, string uin )
{
	logMes_4 ( "ICQKid2::removeVisInvisIgnor" );
	int cont_ind=-1;
	for ( size_t i=0; i<cont.size(); ++i )
		if ( cont[i].uin==uin ) { cont_ind=i; break; }

	if ( cont_ind<0 ) return true;

	SSIContactListItem cli;
	cli.name=uin;
	cli.group_id=0;
	cli.item_id=cont[cont_ind].itemid;
	cli.type=it_type;
	if ( cont[cont_ind].nick!="" ) cli.payload.push_back ( TLVField ( cont[cont_ind].nick, 0x0131 ) );

	vector<uint8_t> data;
	cli.encode_to ( data, 0 );
	uint32_t sync_id;
	if ( sendSNAC ( 0x0013, SSI_ITEM_DELETE, &sync_id, &data ) !=1 ) return false;
	uint16_t retflag;
	if ( !getSSIEditAck ( retflag, sync_id ) ) return false;
	if ( retflag!=SSI_EDIT_OK ) return false;

	for ( size_t i=0; i<cont.size(); /* empty */ )
		if ( cont[i].uin==uin ) cont.erase ( cont.begin() +i );
		else ++i;

	return true;
}

void * startReadSNAC ( void * )
{
	zgui->icq->readSNAC();
	return 0;
}

int ICQKid2::readSNAC()
{
	logMes_4 ( "ICQKid2::readSNAC()" );
	timeb tmstamp1;
	ftime ( &tmstamp1 );

	FlapPacket * fp;

	while ( true )
	{
		logMes_4 ( "Read New snec by soccket" );
		fp = new FlapPacket();
		while ( true )
		{
			if ( network_break_flag )
			{
				logMes_4 ( "Network Break by Flag" );
				return TNETWORK_TIMEOUT;
			}

			if ( fp->recv_from ( sock ) ) break;
		}
		logMes_4 ( "Snac readed by socket" );

		if ( fp->frame_type==FT_SIGNOFF )
		{
			TLVPack tlvp;
			if ( tlvp.decode_from ( fp->payload ) )
			{
				uint16_t err_code=0;
				TLVField * err_tlv = tlvp.findTLV ( 0x0009 );
				if ( err_tlv!=NULL ) err_tlv->getAsInt16 ( err_code );

				string err_url;
				TLVField * url_tlv = tlvp.findTLV ( 0x000b );
				if ( url_tlv!=NULL ) url_tlv->getAsString ( err_url );

				logMes_4 ( "emit onSingOff(err_code, err_url)" );
				emit onSingOff ( err_code, err_url );
			}
			return TNETWORK_CLOSE;
		}

		if ( fp->frame_type!=FT_DATA ) continue;

		SNACData sncd;
		if ( !sncd.decode_from ( fp->payload ) ) continue;
		if ( sncd.flags&0x8000 && sncd.data.size() >=2 )
		{
			uint16_t trash_len;
			memcpy ( &trash_len, &sncd.data[0], sizeof ( trash_len ) );
			trash_len=ntohs ( trash_len );
			if ( sncd.data.size() >= ( unsigned int ) ( 2+trash_len ) )
				sncd.data.erase ( sncd.data.begin(), sncd.data.begin() +2+trash_len );
		}
		if ( stepConnect>39 )
		{
			snac_cache->clearOlderThan ( SNAC_CACHE_LIFETIME );
			if ( pollIncomingEvents ( sncd ) != 1 )
			{
				snac_cache->addEntry ( sncd );
			}
			usleep(delayReadSnec);
		}
		else
		{
			if ( stepConnect==39 )
			{
				snac_cache->addEntry ( sncd );
				pollIncomingEvents ( sncd );
			}
			else
			{
				snac_cache->addEntry ( sncd );
			}
		}
		
		logMes_4 ( "delete" );
		delete fp;
	}

	return 1;
}

int ICQKid2::waitSNAC ( SNACData * snd )
{
	logMes_4 ( "start waitSNAC ["+QString::number ( snd->service_id ) +","+QString::number ( snd->subtype_id ) +"]" );
	time_t startWait = time ( NULL );
	snac_cache->clearOlderThan ( SNAC_CACHE_LIFETIME );
	if ( snac_cache->findEntry ( *snd ) )
	{
		logMes_4 ( "Snec by cash" );
		return 1;
	}
	logMes_4 ( "Not snack in cash" );
	while ( !network_break_flag && ( time ( NULL )-startWait ) <50 ) //time out of 50 sec
	{
		if ( snac_cache->findEntryFromTime ( *snd, startWait ) )
		{
			return 1;
		}
		if ( stepConnect>39 )
		{
			usleep ( timeWaitSnec );
		}
		else
		{
			usleep ( 1 );
		}
	}
	return 0;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
int ICQKid2::sendSNAC ( uint16_t service_id, uint16_t subtype_id, uint32_t * req_id, vector<uint8_t> * data_ptr )
{
	logMes_4 ( "ICQKid2::sendSNAC" );
	FlapPacket fp;
	fp.frame_type=FT_DATA;
	fp.seq_number=flap_seq_number++;

	SNACData sncd;
	sncd.service_id=service_id;
	sncd.subtype_id=subtype_id;
	sncd.flags=0;
	sncd.req_id=snac_seq_number++;

	if ( data_ptr!=NULL ) sncd.data=*data_ptr;
	if ( req_id!=NULL ) *req_id=sncd.req_id;

	sncd.encode_to ( fp.payload );

	timeb tmstamp1, tmstamp2;
	ftime ( &tmstamp1 );
	while ( true )
	{
		if ( network_break_flag )
		{
			network_break_flag=false;
			logMes_4 ( "ICQKid2::sendSNAC end - return timeout" );
			return TNETWORK_TIMEOUT;
		}
		if ( network_timeout>0 )
		{
			ftime ( &tmstamp2 );
			int tmout= ( ( tmstamp2.time-tmstamp1.time ) *1000+tmstamp2.millitm-tmstamp1.millitm );
			if ( tmout>network_timeout )
			{
				logMes_4 ( "ICQKid2::sendSNAC end - return timeout" );
				return TNETWORK_TIMEOUT;
			}
		}
		if ( fp.send_to ( sock ) )
		{
			last_keepalive_timestamp=time ( NULL );
			logMes_4 ( "ICQKid2::sendSNAC end" );
			return 1;
		}
		if ( fp.t_network_error!=TNETWORK_TIMEOUT ) return fp.t_network_error;
	}

	last_keepalive_timestamp=time ( NULL );
	logMes_4 ( "ICQKid2::sendSNAC end" );
	return 1;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
int ICQKid2::sendKeepAlive ( void )
{
	logMes_4 ( "ICQKid2::sendKeepAlive" );
	FlapPacket fp;
	fp.frame_type=FT_KEEP_ALIVE;
	fp.seq_number=flap_seq_number++;

	timeb tmstamp1, tmstamp2;
	ftime ( &tmstamp1 );
	int fixNoEnd=0;
	while ( true )
	{
		if ( network_break_flag )
		{
			network_break_flag=false;
			return TNETWORK_TIMEOUT;
		}
		if ( network_timeout>0 )
		{
			ftime ( &tmstamp2 );
			int tmout= ( ( tmstamp2.time-tmstamp1.time ) *1000+tmstamp2.millitm-tmstamp1.millitm );
			if ( tmout>network_timeout ) return TNETWORK_TIMEOUT;
		}
		if ( fp.send_to ( sock ) )
		{
			last_keepalive_timestamp=time ( NULL );
			return 1;
		}
		if ( fp.t_network_error!=TNETWORK_TIMEOUT ) return fp.t_network_error;

		if ( fixNoEnd > 12 )
		{
			logMes_4("!!!!!!!!!!!ICQKid2::sendKeepAlive - not end while!!!!!!!!!!!");			
			last_keepalive_timestamp=time ( NULL );
			return 1;			
		}
		fixNoEnd++;
	}

	last_keepalive_timestamp=time ( NULL );
	return 1;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::sendAuthResponse ( string uin, string text, uint8_t aflag )
{
	logMes_4 ( "ICQKid2::sendAuthResponse" );
	vector<uint8_t> data ( 1+uin.length() +1+2+text.length() +2 );

	data[0]=uin.length();
	memcpy ( &data[1], uin.data(), uin.length() );
	data[1+uin.length() ]=aflag;
	uint16_t text_len=htons ( text.length() );
	memcpy ( &data[1+uin.length() +1], &text_len, sizeof ( text_len ) );
	memcpy ( &data[1+uin.length() +1+2], text.data(), text.length() );
	memset ( &data[1+uin.length() +1+2+text.length() ], 0, 2 );

	return ( sendSNAC ( 0x0013, 0x001a, NULL, &data ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::sendAuthRequest ( string uin, string text )
{
	logMes_4 ( "ICQKid2::sendAuthRequest" );
	vector<uint8_t> data ( 1+uin.length() +2+text.length() +2 );

	data[0]=uin.length();
	memcpy ( &data[1], uin.data(), uin.length() );
	uint16_t text_len=htons ( text.length() );
	memcpy ( &data[1+uin.length() ], &text_len, sizeof ( text_len ) );
	memcpy ( &data[1+uin.length() +2], text.data(), text.length() );
	memset ( &data[1+uin.length() +2+text.length() ], 0, 2 );

	return ( sendSNAC ( 0x0013, 0x0018, NULL, &data ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::parseIncomingMsg ( ICQKid2Message & msg, vector<uint8_t> & data )
{
	logMes_4 ( "ICQKid2::parseIncomingMsg" );
	msg.timestamp=time ( NULL );

	BaseIncomingMsg bim;
	if ( !bim.decode_from ( data ) ) return false;

	if ( bim.msg_channel==0x0001 ) // Channel 1 message format (plain-text messages)
	{
		SimpleMsg sim;
		if ( !sim.decode_from ( bim.data ) ) return false;
		msg.uin=bim.uin;
		msg.text=sim.message_str;
		switch ( sim.charset )
		{
			case 0 : msg.enc_type=ICQKid2Message::USASCII; break;
			case 2 : msg.enc_type=ICQKid2Message::UCS2BE; break;
			case 3 : msg.enc_type=ICQKid2Message::LOCAL8BIT; break;
			default : return false;
		}
		msg.codepage=sim.sub_charset;
		return true;
	}
	else if ( bim.msg_channel==0x0002 ) // Channel 2 message format (extended messages format)
	{
		ExtendedMsg exm;
		if ( !exm.decode_from ( bim.data ) ) return false;
		msg.uin=bim.uin;
		msg.text=exm.rv_msg.tlv2711.text;
		// $$$ I don't send message of auto response. zIM can not set Status Text 
		if ( exm.rv_msg.tlv2711.msg_type==MSG_TYPE_PLAINTEXT )
		{
			/*
			if ( exm.rv_msg.tlv2711.text_guid=="{0946134E-4C7F-11D1-8222-444553540000}" || \
			        exm.rv_msg.tlv2711.text_guid=="{0946134e-4c7f-11d1-8222-444553540000}" ) msg.enc_type=ICQKid2Message::UTF8;
			else msg.enc_type=ICQKid2Message::LOCAL8BIT;

			memcpy ( msg.text_color, exm.rv_msg.tlv2711.text_color, 4 );
			memcpy ( msg.bg_color, exm.rv_msg.tlv2711.bg_color, 4 );

			( void ) sendMsgAutoResponse ( bim.uin, bim.msg_cookie, MSG_TYPE_PLAINTEXT ); // Plain text
			*/
			return true;
		}
		else if ( exm.rv_msg.tlv2711.msg_type==MSG_TYPE_AUTOAWAY || exm.rv_msg.tlv2711.msg_type==MSG_TYPE_AUTOBUSY || \
		          exm.rv_msg.tlv2711.msg_type==MSG_TYPE_AUTONA || exm.rv_msg.tlv2711.msg_type==MSG_TYPE_AUTODND || \
		          exm.rv_msg.tlv2711.msg_type==MSG_TYPE_AUTOFFC )
		{
			//( void ) sendMsgAutoResponse ( bim.uin, bim.msg_cookie, exm.rv_msg.tlv2711.msg_type );
			return false; // Don't need to use callback onIncomingMsg
		} else 
		if ( exm.rv_msg.tlv2711.msg_type==MSG_TYPE_PLUGIN ) // Plugin message described by text string
		{
			// Process XtraZ Script
			if ( !exm.rv_msg.tlv2711.parseXtrazRequest() ) return false;
			if ( bim.uin!=exm.rv_msg.tlv2711.xstat_sender_id ) return false;
			sendXStatusNotifyAutoResponse ( bim.uin, bim.msg_cookie );
			return false; // Don't need to use callback onIncomingMsg
		}
	}
	else if ( bim.msg_channel==0x0004 ) // Channel 4 message format (typed old-style messages)
	{
		OldStyleMsg osm;
		if ( !osm.decode_from ( bim.data ) ) return false;
		msg.uin=bim.uin;
		msg.text=osm.message_str;

		// Messages types where FE-format presents
		if ( osm.msg_type==0x04 || osm.msg_type==0x06 || osm.msg_type==0x07 || osm.msg_type==0x08 || \
		        osm.msg_type==0x09 || osm.msg_type==0x0C || osm.msg_type==0x0D || osm.msg_type==0x0E )
		{
			string::size_type fe_pos = msg.text.find_last_of ( "\376" ); // 0xfe in oct
			if ( fe_pos!=msg.text.npos ) msg.text.erase ( 0, fe_pos );
		}

		switch ( osm.msg_type )
		{
			case 0x06 : // MTYPE_AUTHREQ
				logMes_4 ( "emit onAuthRequest(msg.uin, msg.text)" );
				emit onAuthRequest ( msg.uin, msg.text );
				return false; // Don't need to use callback onIncomingMsg
				break;
			case 0x07 : // MTYPE_AUTHDENY
				logMes_4 ( "emit onAuthReply(msg.uin, msg.text, AUTH_DECLINED)" );
				emit onAuthReply ( msg.uin, msg.text, AUTH_DECLINED );
				return false; // Don't need to use callback onIncomingMsg
				break;
			case 0x08 : // MTYPE_AUTHOK
				logMes_4 ( "emit onAuthReply(msg.uin, msg.text, AUTH_ACCEPTED)" );
				emit onAuthReply ( msg.uin, msg.text, AUTH_ACCEPTED );
				return false; // Don't need to use callback onIncomingMsg
				break;
			case 0x0C : // MTYPE_ADDED
				logMes_4 ( "emit onWasAdded(msg.uin)" );
				emit onWasAdded ( msg.uin );
				return false; // Don't need to use callback onIncomingMsg
				break;
			case 0x1A : // MTYPE_PLUGIN
			{
				TLV2711 tlv_2711;
				tlv_2711.xtraz_script_text=msg.text;
				if ( !tlv_2711.parseXtrazRequest() ) return false;
				if ( bim.uin!=tlv_2711.xstat_sender_id ) return false;
				logMes_4 ( "sendXStatusNotifyAutoResponse(bim.uin, bim.msg_cookie)" );
				sendXStatusNotifyAutoResponse ( bim.uin, bim.msg_cookie );
				return false; // Don't need to use callback onIncomingMsg
			}
			break;
			default :
				break;
		}
		return true;
	}

	return false; // Unknown message channel
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::parseIncomingMTN ( string & from, uint16_t & type, vector<uint8_t> & data )
{
	logMes_4 ( "ICQKid2::parseIncomingMTN" );
	if ( data.size() <13 ) return false;
	int from_length = data[10];
	if ( data.size() < ( size_t ) ( 13+from_length ) ) return false;
	from.assign ( reinterpret_cast<const char *> ( &data[11] ), from_length );
	memcpy ( &type, &data[11+from_length], sizeof ( type ) );
	type=ntohs ( type );

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::parseAuthRequest ( string & from, string & text, vector<uint8_t> & data )
{
	logMes_4 ( "ICQKid2::parseAuthRequest" );
	if ( data.empty() ) return false;
	uint8_t from_length=data[0];

	if ( data.size() < ( unsigned int ) ( 1+from_length ) ) return false;
	from.assign ( reinterpret_cast<const char *> ( &data[1] ), from_length );

	if ( data.size() < ( unsigned int ) ( 1+from_length+2 ) ) return false;
	uint16_t reason_length;
	memcpy ( &reason_length, &data[1+from_length], sizeof ( reason_length ) );
	reason_length=ntohs ( reason_length );

	if ( data.size() < ( unsigned int ) ( 1+from_length+2+reason_length ) ) return false;
	text.assign ( reinterpret_cast<const char *> ( &data[1+from_length+2] ), reason_length );

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::parseAuthReply ( string & from, string & text, uint8_t & aflag, vector<uint8_t> & data )
{
	logMes_4 ( "ICQKid2::parseAuthReply" );
	if ( data.empty() ) return false;
	uint8_t from_length=data[0];
	if ( data.size() < ( unsigned int ) ( 1+from_length+1 ) ) return false;
	from.assign ( reinterpret_cast<const char *> ( &data[1] ), from_length );
	aflag=data[1+from_length];
	if ( data.size() < ( unsigned int ) ( 1+from_length+1+2 ) ) return false;
	uint16_t reason_length;
	memcpy ( &reason_length, &data[1+from_length+1], sizeof ( reason_length ) );
	reason_length=ntohs ( reason_length );
	if ( data.size() < ( unsigned int ) ( 1+from_length+1+2+reason_length ) ) return false;
	text.assign ( reinterpret_cast<const char *> ( &data[1+from_length+1+2] ), reason_length );

	return true;
}

bool ICQKid2::parseOnlineNotify ( string & uin, uint32_t & stat, vector<uint8_t> & data )
{
	logMes_4 ( "ICQKid2::parseOnlineNotify" );
	size_t curr_pos=0;

	if ( data.empty() ) return false;
	uint8_t uin_length=data[curr_pos++];

	if ( data.size() < ( curr_pos+uin_length ) ) return false;
	uin.assign ( reinterpret_cast<const char *> ( &data[curr_pos] ), uin_length );
	curr_pos+=uin_length;
	curr_pos+=2; // Skip uint16_t warn_level - unused in ICQ

	if ( data.size() < ( curr_pos+2 ) ) return false;
	uint16_t tlv_count;
	memcpy ( &tlv_count, &data[curr_pos], sizeof ( tlv_count ) );
	tlv_count=ntohs ( tlv_count );
	curr_pos+=2;

	stat=STATUS_ONLINE; // default value;

	int uen_ind=findCLUIN ( uin );

	if ( uen_ind>=0 )
	{
		ContactListUins[uen_ind].srv_relay_cap=false;
		ContactListUins[uen_ind].unicode_cap=false;
	}

	for ( int i=0; i<tlv_count; ++i )
	{
		TLVField tlv;
		if ( !tlv.decode_from ( data, curr_pos ) ) break;
		curr_pos+=tlv.data.size() +4; // +2 for TLVtype +2 for TLV length
		if ( tlv.type==0x0006 ) // User status
		{
			if ( !tlv.getAsInt32 ( stat ) ) break;
		}
		else if ( tlv.type==0x0019 && uen_ind>=0 ) // New type capabilities list
		{
			uint8_t cap_templ[16]={0x09, 0x46, 0xFF, 0xFF, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00};
			vector<uint8_t> reconstruct_capas;
			for ( size_t j=0; j<tlv.data.size(); j+=2 )
			{
				cap_templ[2]=tlv.data[j];
				cap_templ[3]=tlv.data[j+1];
				reconstruct_capas.insert ( reconstruct_capas.end(), cap_templ, cap_templ+sizeof ( cap_templ ) );
			}
			if ( !ContactListUins[uen_ind].srv_relay_cap ) ContactListUins[uen_ind].srv_relay_cap=haveSrvRelayCapability ( reconstruct_capas );
			if ( !ContactListUins[uen_ind].unicode_cap ) ContactListUins[uen_ind].unicode_cap=haveUnicodeCapability ( reconstruct_capas );
		}
		else if ( tlv.type==0x000d && uen_ind>=0 ) //Classic capabilities list
		{
			if ( !ContactListUins[uen_ind].srv_relay_cap ) ContactListUins[uen_ind].srv_relay_cap=haveSrvRelayCapability ( tlv.data );
			if ( !ContactListUins[uen_ind].unicode_cap ) ContactListUins[uen_ind].unicode_cap=haveUnicodeCapability ( tlv.data );
			//logMes_4("Num client: %d", haveClientsCapability(tlv.data) );

			size_t ico_num = haveClientsCapability ( tlv.data );
			logMes_4 ( "Num icon: %d", ico_num );
			if ( ContactListUins[uen_ind].clientId != ico_num )
			{
				logMes_4 ( "emit onClientChange(uin, " + QString::number(ico_num) +")" );
				ContactListUins[uen_ind].clientId = ico_num;
				emit onClientChange ( uin, ico_num );
			}

			ico_num = haveXtrazCapability ( tlv.data );
			if ( ico_num > 0 )
			{
				if ( !noAutoXTrazRequest )
				{
					sendXtrazRequest ( uin );
				}
				else
				{
					if ( ico_num<X_STATUS_MAX_BOUND )
					{
						if ( ContactListUins[uen_ind].xStatus != ico_num )
						{
							ContactListUins[uen_ind].xStatus = ico_num;
							ContactListUins[uen_ind].xStatusTitle = "";
							ContactListUins[uen_ind].xStatusDescription = "";
							logMes_4 ( "emit onXstatusChanged(uin, %d, "", "") - [No Auto xTraze Request]",ico_num );
							emit onXstatusChanged ( uin, ico_num, "", "" );
						}
					}
				}
			}
			else
			{
				if ( ContactListUins[uen_ind].xStatus!=X_STATUS_NONE || ContactListUins[uen_ind].xStatusTitle!="" || ContactListUins[uen_ind].xStatusDescription!="" )
				{
					ContactListUins[uen_ind].xStatus=X_STATUS_NONE;
					ContactListUins[uen_ind].xStatusTitle="";
					ContactListUins[uen_ind].xStatusDescription="";
					logMes_4 ( "emit onXstatusChanged(uin, X_STATUS_NONE, "", "")" );
					emit onXstatusChanged ( uin, X_STATUS_NONE, "", "" );
				}
			}
		}
		else if ( tlv.type==0x0003 && uen_ind>=0 && tlv.data.size() >=4 ) // Online since
		{
			ContactListUins[uen_ind].online_since=ntohl ( * ( ( uint32_t* ) &tlv.data[0] ) );
		}
		else if ( tlv.type==0x0004 && uen_ind>=0 && tlv.data.size() >=2 ) // Idle since
		{
			ContactListUins[uen_ind].idle_since=time ( NULL )-60*ntohs ( * ( ( uint16_t* ) &tlv.data[0] ) );
		}
	}

	if ( enabledEye )
	{
		if ( uen_ind>=0 )
		{
			if ( stat==STATUS_OFFLINE )
			{
				itemEye item;
				item.uin = uin;
				item.nick = ContactListUins[uen_ind].nick;
				item.type = 3; //Invisible???
				item.time = QTime::currentTime().toString();
				listEye.push_back ( item );
			}
		}
	}

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::parseOfflineNotify ( string & uin, vector<uint8_t> & data )
{
	logMes_4 ( "ICQKid2::parseOfflineNotify" );
	size_t curr_pos=0;

	if ( data.empty() ) return false;
	uint8_t uin_length=data[curr_pos++];

	if ( data.size() < ( curr_pos+uin_length ) ) return false;
	uin.assign ( reinterpret_cast<const char *> ( &data[curr_pos] ), uin_length );

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::parseSSIAddNotify ( vector<uint8_t> & data )
{
	logMes_4 ( "ICQKid2::parseSSIAddNotify" );
	size_t curr_pos=0;

	size_t oldSize = 0;
	int i = 0;
	
	while ( i < 300 )
	{
		if ( oldSize == curr_pos)
			i++;
		else
		{
			oldSize = curr_pos;
			i=0;
		}
		
		if ( curr_pos>=data.size() ) break;
		SSIContactListItem clit;
		if ( !clit.decode_from ( data, curr_pos ) ) return false;
		curr_pos += 2+clit.name_length+2+2+2+2+clit.payload_length;

		if ( clit.type==0x0000 ) // Buddy record
		{
			SSIUINEntry uen;
			int uen_ind=findCLUIN ( clit.item_id );
			if ( uen_ind>=0 ) uen=ContactListUins[uen_ind];
			uen.uin=clit.name;
			uen.groupid=clit.group_id;
			uen.itemid=clit.item_id;
			uen.nick="";
			uen.groupname="";
			uen.waitauth=false;
			for ( unsigned int j=0; j<clit.payload.size(); ++j )
			{
				if ( clit.payload[j].type==0x0131 ) clit.payload[j].getAsString ( uen.nick );
				if ( clit.payload[j].type==0x0066 ) uen.waitauth=true;
			}

			if ( uen_ind>=0 )
				ContactListUins[uen_ind]=uen;
			else
				ContactListUins.push_back ( uen );
		}

		if ( clit.type==0x0001 && clit.group_id!=0x0000 ) // We dont need to virtual MasterGroup with group_id=0
		{
			SSIGroupEntry gen;
			gen.id=clit.group_id;
			gen.name=clit.name;

			int gen_ind=findCLGroup ( gen.id );
			if ( gen_ind>=0 )
				ContactListGroups[gen_ind]=gen;
			else
				ContactListGroups.push_back ( gen );
		}
	}

	if ( i < 300 )
	{
		for ( unsigned int i=0; i<ContactListUins.size(); ++i )
		{
			int gr_ind=findCLGroup ( ContactListUins[i].groupid );
			if ( gr_ind>=0 ) ContactListUins[i].groupname=ContactListGroups[gr_ind].name;
		}

		return true;
	} else return false;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::parseSSIUpdateNotify ( vector<uint8_t> & data )
{
	logMes_4 ( "ICQKid2::parseSSIUpdateNotify" );
// parseSSIAddNotify is universal method for adding and update, will use it
	return parseSSIAddNotify ( data );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::parseSSIDeleteNotify ( vector<uint8_t> & data )
{
	logMes_4 ( "ICQKid2::parseSSIDeleteNotify" );
	size_t curr_pos=0;

	size_t oldSize = 0;
	int i = 0;
	
	while ( i < 300 )
	{
		if ( oldSize == curr_pos)
			i++;
		else
		{
			oldSize = curr_pos;
			i=0;
		}
		
		if ( curr_pos>=data.size() ) break;
		SSIContactListItem clit;
		if ( !clit.decode_from ( data, curr_pos ) ) return false;
		curr_pos += 2+clit.name_length+2+2+2+2+clit.payload_length;

		if ( clit.type==0x0000 ) // Buddy record
		{
			int uen_ind;
			while ( ( uen_ind=findCLUIN ( clit.item_id ) ) >=0 )
			{
				ContactListUins.erase ( ContactListUins.begin() +uen_ind );
			}
		}

		if ( clit.type==0x0001 && clit.group_id!=0x0000 ) // We dont need to virtual MasterGroup with group_id=0
		{
			// Remove uins from this group
			for ( size_t i=0; i<ContactListUins.size(); /*empty*/ )
			{
				if ( ContactListUins[i].groupid==clit.group_id )
					ContactListUins.erase ( ContactListUins.begin() +i );
				else
					++i;
			}
			int gen_ind;
			while ( ( gen_ind=findCLGroup ( clit.group_id ) ) >=0 )
				ContactListGroups.erase ( ContactListGroups.begin() +gen_ind );
		}
	}
	
	if ( i < 300 )
		return true;
	else
		return false;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::parseWasAdded ( string & from, vector<uint8_t> & data )
{
	logMes_4 ( "ICQKid2::parseWasAdded" );
	if ( data.empty() ) return false;
	uint8_t from_length=data[0];
	if ( data.size() < ( size_t ) ( 1+from_length ) ) return false;
	from.assign ( reinterpret_cast<const char *> ( &data[1] ), from_length );

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::parseXtrazResponse ( string & from, size_t & x_status, string & x_title, string & x_descr, vector<uint8_t> & data )
{
	logMes_4 ( "ICQKid2::parseXtrazResponse" );
	if ( data.size() <11 ) return false;
	if ( data[9]!=2 ) return false; // msg channel
	uint8_t uin_len=data[10];
	if ( data.size() < ( 14+uin_len ) ) return false;

	from.assign ( ( const char* ) &data[11], uin_len );
	if ( data[12+uin_len]!=3 ) return false; // Reason: channel specific

	TLV2711 tlv2711;
	if ( tlv2711.decode_from ( data, 13+uin_len ) ) // return false;
	{
		if ( tlv2711.parseXtrazResponse() ) //return false;
		{
			if ( tlv2711.x_status<X_STATUS_MAX_BOUND ) //return false;
			{
				logMes_4 ( "x_status=%d",tlv2711.x_status-1 );
				logMes_4 ( "x_title="+QString::fromLocal8Bit ( tlv2711.x_title.c_str() ) );
				logMes_4 ( "x_descr="+QString::fromLocal8Bit ( tlv2711.x_descr.c_str() ) );
				x_status=tlv2711.x_status;
				x_title=tlv2711.x_title;
				x_descr=tlv2711.x_descr;
				return true;
			}
		}
	}
	return false;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::parseMsgAutoResponse ( ICQKid2Message & msg, uint8_t & type, vector<uint8_t> & data )
{
	logMes_4 ( "ICQKid2::parseMsgAutoResponse" );
	msg.timestamp=time ( NULL );

	if ( data.size() <11 ) return false;
	if ( data[9]!=2 ) return false; // msg channel
	uint8_t uin_len=data[10];
	if ( data.size() < ( 14+uin_len ) ) return false;
	msg.uin.assign ( ( const char* ) &data[11], uin_len );
	if ( data[12+uin_len]!=3 ) return false; // Reason: channel specific
	TLV2711 tlv2711;
	if ( !tlv2711.decode_from ( data, 13+uin_len ) ) return false;

	type=tlv2711.msg_type;
	if ( type<MSG_TYPE_AUTOAWAY || type>MSG_TYPE_AUTOFFC ) return false;

	msg.text=tlv2711.text;

	if ( tlv2711.text_guid=="{0946134E-4C7F-11D1-8222-444553540000}" || \
	        tlv2711.text_guid=="{0946134e-4c7f-11d1-8222-444553540000}" ) msg.enc_type=ICQKid2Message::UTF8;
	else msg.enc_type=ICQKid2Message::LOCAL8BIT;

	memcpy ( msg.text_color, tlv2711.text_color, 4 );
	memcpy ( msg.bg_color, tlv2711.bg_color, 4 );

	return true;
}

/*
// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::sendMsgAutoResponse ( string touin, uint8_t * msg_cookie, uint8_t type )
{
	logMes_4 ( "ICQKid2::sendMsgAutoResponse" );
	vector<uint8_t> data;
	size_t pos=0;

	if ( enabledEye )
	{
		int uen_ind=findCLUIN ( touin );
		itemEye item;
		item.uin = touin;
		if ( uen_ind>=0 )
			item.nick = ContactListUins[uen_ind].nick;
		item.type = 2; //Read Status
		item.time = QTime::currentTime().toString();
		listEye.push_back ( item );
	}

	data.resize ( pos+8 );
	memcpy ( &data[pos], msg_cookie, 8 );
	pos+=8;

	data.resize ( pos+3 );
	data[pos]=0;
	data[pos+1]=2; // message channel
	data[pos+2]=touin.length();
	pos+=3;

	data.resize ( pos+touin.length() );
	memcpy ( &data[pos], touin.data(), touin.length() );
	pos+=touin.length();

	data.resize ( pos+2 );
	data[pos]=0;
	data[pos+1]=3; // reason code - channel specific
	pos+=2;

	TLV2711 tlv_2711;
	tlv_2711.version=9;
	tlv_2711.cookie=0;
	tlv_2711.msg_type=type;
	tlv_2711.status=0;
	tlv_2711.priority=0;
	switch ( type )
	{
		case MSG_TYPE_PLAINTEXT :
			tlv_2711.flags=0;
			tlv_2711.text="";
			break;

		case MSG_TYPE_AUTOAWAY  :
			tlv_2711.flags=0x03; // +Normal msg, +auto msg, -multi msg
			tlv_2711.text=AutoAwayMessageText;
			break;

		case MSG_TYPE_AUTOBUSY  :
			tlv_2711.flags=0x03; // +Normal msg, +auto msg, -multi msg
			tlv_2711.text=AutoBusyMessageText;
			break;

		case MSG_TYPE_AUTONA    :
			tlv_2711.flags=0x03; // +Normal msg, +auto msg, -multi msg
			tlv_2711.text=AutoNotAvailableMessageText;
			break;

		case MSG_TYPE_AUTODND   :
			tlv_2711.flags=0x03; // +Normal msg, +auto msg, -multi msg
			tlv_2711.text=AutoDoNotDisturbMessageText;
			break;

		case MSG_TYPE_AUTOFFC   :
			tlv_2711.flags=0x03; // +Normal msg, +auto msg, -multi msg
			tlv_2711.text=AutoFreeForChatMessageText;
			break;
	}
	tlv_2711.encode_to ( data, pos );

	return ( sendSNAC ( 0x0004, 0x000b, NULL, &data ) ==1 );
}
*/

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::sengMsgAutoRequest ( string touin, uint8_t type )
{
	logMes_4 ( "ICQKid2::sengMsgAutoRequest" );
	BaseOutgoingMsg bom;
	bom.msg_channel=0x0002;
	bom.uin=touin;
	memset ( bom.msg_cookie, 0xAB, 8 );

	ExtendedMsg ex_msg;
	ex_msg.user_class=0x0050;
	ex_msg.user_status= ( online_status|STATUS_DCDISABLED );
	ex_msg.online_time=0;
	ex_msg.create_time=0;

	uint8_t icq_serv_relay[] = {0x09, 0x46, 0x13, 0x49, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00};
	ex_msg.rv_msg.type=0; // request
	memcpy ( ex_msg.rv_msg.capability, icq_serv_relay, 16 );
	ex_msg.rv_msg.external_ip=0;
	ex_msg.rv_msg.external_port=0;
	memcpy ( ex_msg.rv_msg.cookie, bom.msg_cookie, 8 );

	ex_msg.rv_msg.tlv2711.version=9;
	ex_msg.rv_msg.tlv2711.cookie=0;
	ex_msg.rv_msg.tlv2711.msg_type=type;
	ex_msg.rv_msg.tlv2711.flags=0x03; // +Normal msg, +auto msg, -multi msg
	ex_msg.rv_msg.tlv2711.status=0;
	ex_msg.rv_msg.tlv2711.priority=0;
	ex_msg.rv_msg.tlv2711.text="";

	ex_msg.encode_to ( bom.data );
	vector<uint8_t> data;
	bom.encode_to ( data );

	sendSNAC ( 0x0004, 0x0006, NULL, &data );
	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::askOfflMsgAction ( uint8_t flag, uint32_t * sync_id )
{
	logMes_4 ( "ICQKid2::askOfflMsgAction" );
	uint8_t templ[10] = {0x08, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF}; //0xFF - variable fields
	vector<uint8_t> vec ( templ, &templ[10] );
	uint32_t int_uin = atol ( myuin.c_str() );
	memcpy ( &vec[2], &int_uin, sizeof ( int_uin ) ); // LITTLE ENDIAN!!!
	memcpy ( &vec[8], &flap_seq_number, sizeof ( flap_seq_number ) ); // LITTLE ENDIAN!!!
	vec[6]=flag;

	TLVField tlv ( vec, 0x01 );
	vec.clear();
	tlv.encode_to ( vec, 0 );

	return ( sendSNAC ( 0x0015, 0x0002, sync_id, &vec ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::getOfflMsg ( vector<ICQKid2Message> & offl_vec, uint32_t sync_id )
{
	logMes_4 ( "ICQKid2::getOfflMsg" );
	offl_vec.clear();

	while ( true )
	{
		SNACData snd;
		snd.service_id=0x0015;
		snd.subtype_id=0x0003;
		snd.req_id=sync_id;
		if ( waitSNAC ( &snd ) !=1 ) return false;

		TLVField tlv;
		if ( !tlv.decode_from ( snd.data, 0 ) ) return false;
		if ( tlv.type!=1 ) continue;
		if ( tlv.data.size() <10 ) return false;

		uint16_t offl_type;
		memcpy ( &offl_type, &tlv.data[6], sizeof ( offl_type ) ); //LITTLE ENDIAN

		if ( offl_type==0x0042 ) break; // End of offline messages
		if ( offl_type!=0x0041 ) continue; // If not end, must be 0x0041

		if ( tlv.data.size() <25 ) return false;

		uint32_t sender_uin;
		memcpy ( &sender_uin, &tlv.data[10], sizeof ( sender_uin ) ); //LITLE ENDIAN

		uint16_t year;
		memcpy ( &year, &tlv.data[14], sizeof ( year ) ); //LITLE ENDIAN

		struct tm tm_time;
		tm_time.tm_isdst=0;
		tm_time.tm_year=year-1900;
		tm_time.tm_mon=tlv.data[16]-1;
		tm_time.tm_mday=tlv.data[17];
		tm_time.tm_hour=tlv.data[18];
		tm_time.tm_min=tlv.data[19];
		tm_time.tm_sec=0;

		uint16_t str_len;
		memcpy ( &str_len, &tlv.data[22], sizeof ( str_len ) ); //LITLE ENDIAN

		if ( tlv.data.size() < ( unsigned int ) ( 24+str_len ) || str_len<1 ) return false;

		string text ( reinterpret_cast<const char*> ( &tlv.data[24] ), str_len-1 );
		ostringstream ss;
		ss << sender_uin;

		ICQKid2Message msg ( ss.str(), text, ICQKid2Message::USASCII, 0, true );
		msg.timestamp=mktime ( &tm_time )-timezone;

		offl_vec.push_back ( msg );
	}

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::askShortInfo ( string uin, uint32_t * sync_id )
{
	logMes_4 ( "ICQKid2::askShortInfo" );
	uint8_t templ[16] = {0x0e, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xd0, 0x07, 0xFF, 0xFF, 0xba, 0x04, 0xFF, 0xFF, 0xFF, 0xFF};
	vector<uint8_t> vec ( templ, &templ[16] );
	uint32_t int_myuin = atol ( myuin.c_str() );
	memcpy ( &vec[2], &int_myuin, sizeof ( int_myuin ) ); // LITTLE ENDIAN!!!
	uint32_t int_uin = atol ( uin.c_str() );
	memcpy ( &vec[12], &int_uin, sizeof ( int_uin ) ); // LITTLE ENDIAN!!!
	memcpy ( &vec[8], &flap_seq_number, sizeof ( flap_seq_number ) ); // LITTLE ENDIAN!!!

	TLVField tlv ( vec, 0x01 );
	vec.clear();
	tlv.encode_to ( vec, 0 );

	return ( sendSNAC ( 0x0015, 0x0002, sync_id, &vec ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::getShortInfo ( ICQKidShortUserInfo & info, uint32_t sync_id )
{
	logMes_4 ( "ICQKid2::getShortInfo" );
	SNACData snd;
	snd.service_id=0x0015;
	snd.subtype_id=0x0003;
	snd.req_id=sync_id;
	if ( waitSNAC ( &snd ) !=1 ) return false;

	TLVField tlv;
	if ( !tlv.decode_from ( snd.data, 0 ) ) return false;
	if ( tlv.type!=1 ) return false;
	if ( tlv.data.size() <15 ) return false;
	if ( tlv.data[12]!=0x0a ) return false;

	size_t curr_pos=13;

	uint16_t nick_length;
	memcpy ( &nick_length, &tlv.data[curr_pos], sizeof ( nick_length ) ); // LITTLE ENDIAN!!!
	if ( nick_length<1 ) return false;
	curr_pos += 2;
	if ( tlv.data.size() < ( curr_pos+nick_length ) ) return false;
	info.Nickname.assign ( reinterpret_cast<const char *> ( &tlv.data[curr_pos] ), nick_length-1 );
	curr_pos += nick_length;

	if ( tlv.data.size() < ( curr_pos+2 ) ) return false;
	uint16_t fname_length;
	memcpy ( &fname_length, &tlv.data[curr_pos], sizeof ( fname_length ) ); // LITTLE ENDIAN!!!
	if ( fname_length<1 ) return false;
	curr_pos += 2;
	if ( tlv.data.size() < ( curr_pos+fname_length ) ) return false;
	info.Firstname.assign ( reinterpret_cast<const char *> ( &tlv.data[curr_pos] ), fname_length-1 );
	curr_pos += fname_length;

	if ( tlv.data.size() < ( curr_pos+2 ) ) return false;
	uint16_t lname_length;
	memcpy ( &lname_length, &tlv.data[curr_pos], sizeof ( lname_length ) ); // LITTLE ENDIAN!!!
	if ( lname_length<1 ) return false;
	curr_pos += 2;
	if ( tlv.data.size() < ( curr_pos+lname_length ) ) return false;
	info.Lastname.assign ( reinterpret_cast<const char *> ( &tlv.data[curr_pos] ), lname_length-1 );
	curr_pos += lname_length;

	if ( tlv.data.size() < ( curr_pos+2 ) ) return false;
	uint16_t email_length;
	memcpy ( &email_length, &tlv.data[curr_pos], sizeof ( email_length ) ); // LITTLE ENDIAN!!!
	if ( email_length<1 ) return false;
	curr_pos += 2;
	if ( tlv.data.size() < ( curr_pos+email_length ) ) return false;
	info.Email.assign ( reinterpret_cast<const char *> ( &tlv.data[curr_pos] ), email_length-1 );
	curr_pos += email_length;

	if ( tlv.data.size() < ( curr_pos+1 ) ) return false;
	if ( tlv.data[curr_pos]!=0 ) info.Needauth=true;
	else info.Needauth=false;
	curr_pos += 2; // Skip unknown char 00

	if ( tlv.data.size() < ( curr_pos+1 ) ) return false;
	info.Gender = tlv.data[curr_pos];

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::askFullInfo ( string uin, uint32_t * sync_id )
{
	logMes_4 ( "ICQKid2::askFullInfo" );
	uint8_t data_templ[] = {0x00, 0x01, /* TLV.Type(1) - encapsulated META_DATA */
	                        0x00, 0x10, /* TLV Length */
	                        0x0E, 0x00, /* data chunk size LE */
	                        0xFF, 0xFF, 0xFF, 0xFF, /* request owner uin in LE integer form */
	                        0xD0, 0x07, /* data type: META_DATA_REQ */
	                        0x01, 0x00, /* request sequence number */
	                        0xFF, 0xFF, /* data subtype */
	                        0xFF, 0xFF, 0xFF, 0xFF
	                       }; /* uin to search in LE integer form */

	uint32_t my_int_uin=atoi ( myuin.c_str() );
	memcpy ( data_templ+6, &my_int_uin, sizeof ( my_int_uin ) );
	if ( uin==myuin ) { data_templ[14]=0xb2; data_templ[15]=0x04; } // Selfinfo request
	else { data_templ[14]=0xd0; data_templ[15]=0x04; } // Foreign info request
	uint32_t int_uin=atoi ( uin.c_str() );
	memcpy ( data_templ+16, &int_uin, sizeof ( int_uin ) );

	vector<uint8_t> data ( data_templ, data_templ+sizeof ( data_templ ) );

	return ( sendSNAC ( 0x0015, 0x0002, sync_id, &data ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::getFullInfo ( ICQKidFullUserInfo & info, uint32_t sync_id )
{
	logMes_4 ( "ICQKid2::getFullInfo" );
	SNACData snd;
	snd.service_id=0x0015;
	snd.subtype_id=0x0003;
	snd.req_id=sync_id;
	snd.flags=0x0001;

	uint32_t my_int_uin=atoi ( myuin.c_str() );

	while ( snd.flags&0x0001 )
	{
		if ( waitSNAC ( &snd ) !=1 ) return false;
		if ( snd.data.size() <17 ) return false;
		if ( snd.data[0]!=0 || snd.data[1]!=1 ) return false; // TLV.Type(1) - encapsulated META_DATA
		if ( memcmp ( &snd.data[6], &my_int_uin, sizeof ( my_int_uin ) ) !=0 ) return false;
		if ( snd.data[10]!=0xDA || snd.data[11]!=0x07 ) return false; // data type: META_DATA

		if ( snd.data[14]==0xC8 && snd.data[15]==0x00 ) // data subtype: META_BASIC_USERINFO
		{
			parseBasicUserInfo ( snd.data, info );
		}
		else if ( snd.data[14]==0xDC && snd.data[15]==0x00 ) // data subtype: META_MORE_USERINFO
		{
			parseMoreUserInfo ( snd.data, info );
		}
		else if ( snd.data[14]==0xEB && snd.data[15]==0x00 ) // data subtype: META_EMAIL_USERINFO
		{
			parseEmailUserInfo ( snd.data, info );
		}
		else if ( snd.data[14]==0x0E && snd.data[15]==0x01 ) // data subtype: META_HPAGECAT_USERINFO
		{
			parseHpagecatUserInfo ( snd.data, info );
		}
		else if ( snd.data[14]==0xD2 && snd.data[15]==0x00 ) // data subtype: META_WORK_USERINFO
		{
			parseWorkUserInfo ( snd.data, info );
		}
		else if ( snd.data[14]==0xE6 && snd.data[15]==0x00 ) // data subtype: META_NOTES_USERINFO
		{
			parseNotesUserInfo ( snd.data, info );
		}
		else if ( snd.data[14]==0xF0 && snd.data[15]==0x00 ) // data subtype: META_INTERESTS_USERINFO
		{
			parseInterestsUserInfo ( snd.data, info );
		}
		else if ( snd.data[14]==0xFA && snd.data[15]==0x00 ) // data subtype: META_AFFILIATIONS_USERINFO
		{
			parseAffiliationsUserInfo ( snd.data, info );
		}
	}

	return true;
}

#define UNPACK_LE(arg) \
 if (data.size()<(pos+sizeof(arg))) return; \
 memcpy(&arg, &data[pos], sizeof(arg)); \
 pos+=sizeof(arg);

#define UNPACK_ASCIIZ(str) \
 { \
 uint16_t strz_len; \
 UNPACK_LE(strz_len) \
 if (data.size()<(pos+strz_len)) return; \
 if (strz_len>1) str.assign(reinterpret_cast<const char *>(&data[pos]), strz_len-1); \
 else str=""; \
 pos+=strz_len; \
 }


// ----------------=========ooooOOOOOOOOOoooo=========----------------
void ICQKid2::parseBasicUserInfo ( vector<uint8_t> & data, ICQKidFullUserInfo & info )
{
	logMes_4 ( "ICQKid2::parseBasicUserInfo" );
	size_t pos=16;
	if ( data[pos++]!=0x0a ) return;

	UNPACK_ASCIIZ ( info.Nickname )
	UNPACK_ASCIIZ ( info.Firstname )
	UNPACK_ASCIIZ ( info.Lastname )
	UNPACK_ASCIIZ ( info.Email )
	UNPACK_ASCIIZ ( info.Homecity )
	UNPACK_ASCIIZ ( info.Homestate )
	UNPACK_ASCIIZ ( info.Homephone )
	UNPACK_ASCIIZ ( info.Homefax )
	UNPACK_ASCIIZ ( info.Homeaddress )
	UNPACK_ASCIIZ ( info.Cellphone )
	UNPACK_ASCIIZ ( info.Homezip )
	UNPACK_LE ( info.Homecountry )
	UNPACK_LE ( info.GMToffset )
	UNPACK_LE ( info.Auth )
	UNPACK_LE ( info.Webaware )
	UNPACK_LE ( info.DCpermissions )
	UNPACK_LE ( info.Pubprimarymail )
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
void ICQKid2::parseMoreUserInfo ( vector<uint8_t> & data, ICQKidFullUserInfo & info )
{
	logMes_4 ( "ICQKid2::parseMoreUserInfo" );
	size_t pos=16;
	if ( data[pos++]!=0x0a ) return;

	UNPACK_LE ( info.Age )
	UNPACK_LE ( info.Gender )
	UNPACK_ASCIIZ ( info.Homepage )
	UNPACK_LE ( info.Birthyear )
	UNPACK_LE ( info.Birthmonth )
	UNPACK_LE ( info.Birthday )
	UNPACK_LE ( info.Language1 )
	UNPACK_LE ( info.Language2 )
	UNPACK_LE ( info.Language3 )
	pos+=2;
	UNPACK_ASCIIZ ( info.Origcity )
	UNPACK_ASCIIZ ( info.Origstate )
	UNPACK_LE ( info.Origcountry )
	UNPACK_LE ( info.Maritalstatus )
	UNPACK_LE ( info.Allowspam )
	UNPACK_LE ( info.Codepage )
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
void ICQKid2::parseEmailUserInfo ( vector<uint8_t> & data, ICQKidFullUserInfo & info )
{
	logMes_4 ( "ICQKid2::parseEmailUserInfo" );
	info.Emails.clear();

	size_t pos=16;
	if ( data[pos++]!=0x0a ) return;

	uint8_t emails_count;
	UNPACK_LE ( emails_count )

	string email;
	uint8_t pub_flag;

	for ( size_t i=0; i<emails_count; ++i )
	{
		UNPACK_LE ( pub_flag );
		UNPACK_ASCIIZ ( email );
		info.Emails.push_back ( pair<string, uint8_t> ( email, pub_flag ) );
	}
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
void ICQKid2::parseHpagecatUserInfo ( vector<uint8_t> & data, ICQKidFullUserInfo & info )
{
	logMes_4 ( "ICQKid2::parseHpagecatUserInfo" );
	size_t pos=16;
	if ( data[pos++]!=0x0a ) return;

	UNPACK_LE ( info.Homepageenabled )
	UNPACK_LE ( info.Homepagecategory )
	UNPACK_ASCIIZ ( info.Homepagekeywords )
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
void ICQKid2::parseWorkUserInfo ( vector<uint8_t> & data, ICQKidFullUserInfo & info )
{
	logMes_4 ( "ICQKid2::parseWorkUserInfo" );
	size_t pos=16;
	if ( data[pos++]!=0x0a ) return;

	UNPACK_ASCIIZ ( info.Workcity )
	UNPACK_ASCIIZ ( info.Workstate )
	UNPACK_ASCIIZ ( info.Workphone )
	UNPACK_ASCIIZ ( info.Workfax )
	UNPACK_ASCIIZ ( info.Workaddress )
	UNPACK_ASCIIZ ( info.Workzip )
	UNPACK_LE ( info.Workcountry )
	UNPACK_ASCIIZ ( info.Workcompany )
	UNPACK_ASCIIZ ( info.Workdepartment )
	UNPACK_ASCIIZ ( info.Workposition )
	UNPACK_LE ( info.Workoccupation )
	UNPACK_ASCIIZ ( info.Workwebpage )
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
void ICQKid2::parseNotesUserInfo ( vector<uint8_t> & data, ICQKidFullUserInfo & info )
{
	logMes_4 ( "ICQKid2::parseNotesUserInfo" );
	size_t pos=16;
	if ( data[pos++]!=0x0a ) return;

	UNPACK_ASCIIZ ( info.Notes )
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
void ICQKid2::parseInterestsUserInfo ( vector<uint8_t> & data, ICQKidFullUserInfo & info )
{
	logMes_4 ( "ICQKid2::parseInterestsUserInfo" );
	size_t pos=16;
	if ( data[pos++]!=0x0a ) return;

	pos++;
	UNPACK_LE ( info.Interest1category )
	UNPACK_ASCIIZ ( info.Interest1 )
	UNPACK_LE ( info.Interest2category )
	UNPACK_ASCIIZ ( info.Interest2 )
	UNPACK_LE ( info.Interest3category )
	UNPACK_ASCIIZ ( info.Interest3 )
	UNPACK_LE ( info.Interest4category )
	UNPACK_ASCIIZ ( info.Interest4 )
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
void ICQKid2::parseAffiliationsUserInfo ( vector<uint8_t> & data, ICQKidFullUserInfo & info )
{
	logMes_4 ( "ICQKid2::parseAffiliationsUserInfo" );
	size_t pos=16;
	if ( data[pos++]!=0x0a ) return;

	pos++;
	UNPACK_LE ( info.Past1category )
	UNPACK_ASCIIZ ( info.Past1 )
	UNPACK_LE ( info.Past2category )
	UNPACK_ASCIIZ ( info.Past2 )
	UNPACK_LE ( info.Past3category )
	UNPACK_ASCIIZ ( info.Past3 )
	pos++;
	UNPACK_LE ( info.Affiliation1category )
	UNPACK_ASCIIZ ( info.Affiliation1 )
	UNPACK_LE ( info.Affiliation2category )
	UNPACK_ASCIIZ ( info.Affiliation2 )
	UNPACK_LE ( info.Affiliation3category )
	UNPACK_ASCIIZ ( info.Affiliation3 )
}

#undef UNPACK_LE
#undef UNPACK_ASCIIZ

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::getSearchAnswer ( vector<ICQKidShortUserInfo> & result_vec, uint32_t snac_sync )
{
	logMes_4 ( "ICQKid2::getSearchAnswer" );
	TLVField tlv;
	uint32_t int_uin;

	SNACData snd;
	snd.service_id=0x0015;
	snd.subtype_id=0x0003;
	snd.req_id=snac_sync;

	result_vec.clear();
	while ( true )
	{
		if ( waitSNAC ( &snd ) !=1 ) return false;
		if ( !tlv.decode_from ( snd.data, 0 ) ) return false;
		if ( tlv.type!=0x01 ) return false;
		if ( tlv.data.size() <13 ) return false;
		if ( tlv.data[6]!=0xda || tlv.data[7]!=0x07 || tlv.data[11]!=0x01 ) return false;
		if ( tlv.data[12]==0x0a ) // Success flag
		{
			ICQKidShortUserInfo suinfo;
			size_t curr_pos=15;
			if ( tlv.data.size() < ( curr_pos+4 ) ) return false;
			memcpy ( &int_uin, &tlv.data[curr_pos], sizeof ( int_uin ) ); // Little Endian!!!
			curr_pos += sizeof ( int_uin );
			ostringstream ss;
			ss << int_uin;
			suinfo.Uin=ss.str();

			uint16_t nick_length;
			if ( tlv.data.size() < ( curr_pos+2 ) ) return false;
			memcpy ( &nick_length, &tlv.data[curr_pos], sizeof ( nick_length ) ); // Little Endian!!!
			curr_pos += sizeof ( nick_length );
			if ( tlv.data.size() < ( curr_pos+nick_length ) ) return false;
			if ( nick_length>1 ) suinfo.Nickname.assign ( reinterpret_cast<const char *> ( &tlv.data[curr_pos] ), nick_length-1 );
			curr_pos += nick_length;

			uint16_t fname_length;
			if ( tlv.data.size() < ( curr_pos+2 ) ) return false;
			memcpy ( &fname_length, &tlv.data[curr_pos], sizeof ( fname_length ) ); // Little Endian!!!
			curr_pos += sizeof ( fname_length );
			if ( tlv.data.size() < ( curr_pos+fname_length ) ) return false;
			if ( fname_length>1 ) suinfo.Firstname.assign ( reinterpret_cast<const char *> ( &tlv.data[curr_pos] ), fname_length-1 );
			curr_pos += fname_length;

			uint16_t lname_length;
			if ( tlv.data.size() < ( curr_pos+2 ) ) return false;
			memcpy ( &lname_length, &tlv.data[curr_pos], sizeof ( lname_length ) ); // Little Endian!!!
			curr_pos += sizeof ( lname_length );
			if ( tlv.data.size() < ( curr_pos+lname_length ) ) return false;
			if ( lname_length>1 ) suinfo.Lastname.assign ( reinterpret_cast<const char *> ( &tlv.data[curr_pos] ), lname_length-1 );
			curr_pos += lname_length;

			uint16_t email_length;
			if ( tlv.data.size() < ( curr_pos+2 ) ) return false;
			memcpy ( &email_length, &tlv.data[curr_pos], sizeof ( email_length ) ); // Little Endian!!!
			curr_pos += sizeof ( email_length );
			if ( tlv.data.size() < ( curr_pos+email_length ) ) return false;
			if ( email_length>1 ) suinfo.Email.assign ( reinterpret_cast<const char *> ( &tlv.data[curr_pos] ), email_length-1 );
			curr_pos += email_length;

			if ( tlv.data.size() < ( curr_pos+1 ) ) return false;
			if ( tlv.data[curr_pos]==1 ) suinfo.Needauth=true;
			else suinfo.Needauth=false;
			++curr_pos;

			if ( tlv.data.size() < ( curr_pos+2 ) ) return false;
			if ( tlv.data[curr_pos]==1 ) suinfo.Status=STATUS_ONLINE;
			else suinfo.Status=STATUS_OFFLINE;
			curr_pos += 2;

			if ( tlv.data.size() < ( curr_pos+1 ) ) return false;
			else suinfo.Gender=tlv.data[curr_pos];
			++curr_pos;

			if ( tlv.data.size() < ( curr_pos+2 ) ) return false;
			else memcpy ( & ( suinfo.Age ), &tlv.data[curr_pos], sizeof ( suinfo.Age ) ); // Little Endian!!!

			result_vec.push_back ( suinfo );
		}
		if ( tlv.data[10]==0xae ) break; // It was last packet
		if ( tlv.data[10]!=0xa4 ) return false; // If not last and not A4 - It's error
	}

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
uint16_t ICQKid2::getUnusedItemID ( void )
{
	logMes_4 ( "ICQKid2::getUnusedItemID" );
	for ( uint16_t i=0x0001; i<0xfffe; ++i )
	{
		if ( myPrivSrvStatus_item_id==i ) continue;

		bool has_collis=false;
		for ( size_t j=0; j<ContactListUins.size(); ++j )
			if ( ContactListUins[j].itemid==i )
			{
				has_collis=true;
				break;
			}
		if ( has_collis ) continue;
		for ( size_t j=0; j<VisibleList.size(); ++j )
			if ( VisibleList[j].itemid==i )
			{
				has_collis=true;
				break;
			}
		if ( has_collis ) continue;

		for ( size_t j=0; j<InvisibleList.size(); ++j )
			if ( InvisibleList[j].itemid==i )
			{
				has_collis=true;
				break;
			}
		if ( has_collis ) continue;

		for ( size_t j=0; j<IgnoreList.size(); ++j )
			if ( IgnoreList[j].itemid==i )
			{
				has_collis=true;
				break;
			}
		if ( has_collis ) continue;

		for (set<uint16_t>::iterator iter=unknown_item_ids.begin(); iter!=unknown_item_ids.end(); ++iter)
			if (*iter==i)
			{
				has_collis=true;
				break;
			}
		if (has_collis) continue;

		return i;
	}

	return 0;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
uint16_t ICQKid2::getUnusedGroupID ( void )
{
	logMes_4 ( "ICQKid2::getUnusedGroupID" );
	for ( uint16_t i=0x0001; i<0xfffe; ++i )
	{
		bool has_collis=false;
		for ( size_t j=0; j<ContactListGroups.size(); ++j )
			if ( ContactListGroups[j].id==i )
			{
				has_collis=true;
				break;
			}
		if ( !has_collis ) return i;
	}

	return 0;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::startSSITransact ( void )
{
	logMes_4 ( "ICQKid2::startSSITransact" );
	return ( sendSNAC ( 0x0013, 0x0011 ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::endSSITransact ( void )
{
	logMes_4 ( "ICQKid2::endSSITransact" );
	return ( sendSNAC ( 0x0013, 0x0012 ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::getSSIEditAck ( uint16_t & retflag, uint32_t sync_id )
{
	logMes_4 ( "ICQKid2::getSSIEditAck" );
	SNACData snd;
	snd.service_id=0x0013;
	snd.subtype_id=0x000e;
	snd.req_id=sync_id;

	if ( waitSNAC ( &snd ) !=1 ) return false;
	for ( unsigned int i=0; i< ( snd.data.size() /2 ); ++i )
	{
		memcpy ( &retflag, &snd.data[i*2], sizeof ( retflag ) );
		retflag=ntohs ( retflag );
		if ( retflag!=SSI_EDIT_OK ) break;
	}

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::addDeleSSIUIN ( SSIUINEntry uen, uint16_t flag, uint32_t * sync_id )
{
	logMes_4 ( "ICQKid2::addDeleSSIUIN" );
	SSIContactListItem cli;
	cli.name=uen.uin;
	cli.group_id=uen.groupid;
	cli.item_id=uen.itemid;
	cli.type=0x0000; // Buddy record

	if ( uen.nick!="" )
	{
		TLVField tlv1 ( uen.nick, 0x0131 );
		cli.payload.push_back ( tlv1 );
	}

	if ( uen.waitauth )
	{
		TLVField tlv2 ( "", 0x0066 );
		cli.payload.push_back ( tlv2 );
	}

	vector<uint8_t> data;
	cli.encode_to ( data, 0 );

	return ( sendSNAC ( 0x0013, flag, sync_id, &data ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::addDeleSSIGroup ( SSIGroupEntry gen, uint16_t flag, uint32_t * sync_id )
{
	logMes_4 ( "ICQKid2::addDeleSSIGroup" );
	SSIContactListItem cli;
	cli.name=gen.name;
	cli.group_id=gen.id;
	cli.item_id=0x0000;
	cli.type=0x0001; // Group

	vector<uint8_t> data;
	cli.encode_to ( data, 0 );

	return ( sendSNAC ( 0x0013, flag, sync_id, &data ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::updateSSIGroupContent ( uint16_t group_id, uint32_t * sync_id )
{
	logMes_4 ( "ICQKid2::updateSSIGroupContent" );
	SSIContactListItem cli;
	cli.group_id=group_id;
	cli.item_id=0x0000;
	cli.type=0x0001; // Group
	if ( group_id!=0 )
	{
		int gr_ind=findCLGroup ( group_id );
		if ( gr_ind<0 ) return false;
		cli.name=ContactListGroups[gr_ind].name;
	}

	TLVField tlv ( NULL, 0, 0x00c8 ); // Group members

#define PACK_16(arg) \
 { \
 uint16_t tmp_arg=htons(arg); \
 tlv.data.insert(tlv.data.end(), (uint8_t*)(&tmp_arg), ((uint8_t*)(&tmp_arg))+sizeof(tmp_arg)); \
 }

	if ( group_id!=0 )
	{
		for ( size_t i=0; i<ContactListUins.size(); ++i )
			if ( ContactListUins[i].groupid==group_id )
				PACK_16 ( ContactListUins[i].itemid )
			}
	else
	{
		for ( size_t i=0; i<ContactListGroups.size(); ++i )
			PACK_16 ( ContactListGroups[i].id )
		}

	if ( !tlv.data.empty() ) cli.payload.push_back ( tlv );

#undef PACK_16

	vector<uint8_t> data;
	cli.encode_to ( data, 0 );
	return ( sendSNAC ( 0x0013, SSI_ITEM_CHANGE, sync_id, &data ) ==1 );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::getOfflineMessages ( void )
{
	logMes_4 ( "ICQKid2::getOfflineMessages" );
	uint32_t sync_id;
	if ( !askOfflMsgAction ( OFFLINE_RECIEVE, &sync_id ) )
	{
		logMes_4 ( "Error on askOfflMsgAction(OFFLINE_RECIEVE, &sync_id)" );
		return false;
	}
	logMes_4 ( "get off 1" );
	sleep ( 3 );
	logMes_4 ( "get off 2" );
	vector<ICQKid2Message> offl_vec;
	logMes_4 ( "get off 3" );
	if ( !getOfflMsg ( offl_vec, sync_id ) )
	{
		logMes_4 ( "Error on getOfflMsg(offl_vec, sync_id)" );
		return false;
	}
	else
	{
		logMes_4 ( "Offline message count = %d",offl_vec.size() );
	}

	if ( !askOfflMsgAction ( OFFLINE_DELETE, &sync_id ) )
	{
		logMes_4 ( "Error on askOfflMsgAction(OFFLINE_DELETE, &sync_id)" );
		return false;
	}
	for ( unsigned int i=0; i<offl_vec.size(); ++i )
	{
		logMes_4 ( "emit onIncomingMsg(offl_vec[i])" );
		emit onIncomingMsg ( offl_vec[i] );
	}

	return true;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
///int ICQKid2::pollIncomingEvents(int tmout)
int ICQKid2::pollIncomingEvents ( SNACData & snd )
{
	logMes_4 ( "ICQKid2::pollIncomingEvents" );

	logMes_4 ( "snd.service_id=%d,snd.subtype_id=%d",snd.service_id,snd.subtype_id );

	if ( snd.service_id==0x0004 && snd.subtype_id==0x0007 ) // Incoming message
	{
		ICQKid2Message msg;
		if ( !parseIncomingMsg ( msg, snd.data ) ) return TNETWORK_TIMEOUT;
		logMes_4 ( "emit onIncomingMsg(msg)" );
		emit onIncomingMsg ( msg );
	}
	else
		if ( snd.service_id==0x0004 && snd.subtype_id==0x000b ) // Incoming client autoresponse, may be autostatus message
		{
			ICQKid2Message msg;
			uint8_t type;
			if ( !parseMsgAutoResponse ( msg, type, snd.data ) ) //return TNETWORK_TIMEOUT;
			{
				string from;
				size_t x_status;
				string x_title;
				string x_descr;
				logMes_4 ( "parseXtrazResponse" );
				if ( !parseXtrazResponse ( from, x_status, x_title, x_descr, snd.data ) ) return TNETWORK_TIMEOUT;
				int uen_ind=findCLUIN ( from );
				if ( uen_ind<0 )
				{
					logMes_4 ( "emit onXstatusChanged(from, x_status, x_title, x_descr)" );
					emit onXstatusChanged ( from, x_status, x_title, x_descr );
				}
				else if ( ( ContactListUins[uen_ind].xStatus!=x_status && x_status != 0 ) || ContactListUins[uen_ind].xStatusTitle!=x_title || ContactListUins[uen_ind].xStatusDescription!=x_descr )
				{
					if ( x_status != 0 )
					{
						ContactListUins[uen_ind].xStatus=x_status;
					}

					ContactListUins[uen_ind].xStatusTitle=x_title;
					ContactListUins[uen_ind].xStatusDescription=x_descr;

					logMes_4 ( "emit onXstatusChanged(from, x_status, x_title, x_descr)" );
					emit onXstatusChanged ( from, ContactListUins[uen_ind].xStatus, x_title, x_descr );
				}
			}
			else
			{
				int uen_ind=findCLUIN ( msg.uin );
				if ( uen_ind>=0 )
				{
					switch ( type )
					{
						case MSG_TYPE_AUTOAWAY  :
							if ( ContactListUins[uen_ind].AutoAwayMessageText!=msg.text )
							{
								ContactListUins[uen_ind].AutoAwayMessageText=msg.text;
								onIncomingAutoStatusMsg ( msg, type );
							}
							break;

						case MSG_TYPE_AUTOBUSY  :
							if ( ContactListUins[uen_ind].AutoBusyMessageText!=msg.text )
							{
								ContactListUins[uen_ind].AutoBusyMessageText=msg.text;
								onIncomingAutoStatusMsg ( msg, type );
							}
							break;

						case MSG_TYPE_AUTONA    :
							if ( ContactListUins[uen_ind].AutoNotAvailableMessageText!=msg.text )
							{
								ContactListUins[uen_ind].AutoNotAvailableMessageText=msg.text;
								onIncomingAutoStatusMsg ( msg, type );
							}
							break;

						case MSG_TYPE_AUTODND   :
							if ( ContactListUins[uen_ind].AutoDoNotDisturbMessageText!=msg.text )
							{
								ContactListUins[uen_ind].AutoDoNotDisturbMessageText=msg.text;
								onIncomingAutoStatusMsg ( msg, type );
							}
							break;

						case MSG_TYPE_AUTOFFC   :
							if ( ContactListUins[uen_ind].AutoFreeForChatMessageText!=msg.text )
							{
								ContactListUins[uen_ind].AutoFreeForChatMessageText=msg.text;
								onIncomingAutoStatusMsg ( msg, type );
							}
							break;
					}
				}
				else
				{
					onIncomingAutoStatusMsg ( msg, type );
				}
			} //
		}
		else
			if ( snd.service_id==0x0013 && snd.subtype_id==0x0019 ) // Authorize request
			{
				string from, text;
				if ( !parseAuthRequest ( from, text, snd.data ) ) return TNETWORK_TIMEOUT;
				logMes_4 ( "emit onAuthRequest(from, text)" );
				emit onAuthRequest ( from, text );
			}
			else
				if ( snd.service_id==0x0013 && snd.subtype_id==0x001b ) // Authorize reply
				{
					string from, text;
					uint8_t aflag;
					if ( !parseAuthReply ( from, text, aflag, snd.data ) ) return TNETWORK_TIMEOUT;
					logMes_4 ( "emit onAuthReply(from, text, aflag)" );
					emit onAuthReply ( from, text, aflag );
				}
				else
					if ( snd.service_id==0x0003 && snd.subtype_id==0x000b ) // Online notification
					{
						string from;
						uint32_t stat;
						if ( !parseOnlineNotify ( from, stat, snd.data ) )
						{
							logMes_4 ( "Not online notifule" );
							return TNETWORK_TIMEOUT;
						}

						uint32_t stat1=stat&0xffff0000;
						uint32_t stat2=stat&0x0000ffff;
						bool invis_flag=false;

						if ( stat2&STATUS_INVISIBLE )
						{
							invis_flag=true;
							stat2&=0xfffff0ff;
						}

						if ( stat2==FSTATUS_NA ) stat2=STATUS_NA;
						if ( stat2==FSTATUS_OCCUPIED ) stat2=STATUS_OCCUPIED;
						if ( stat2==FSTATUS_DND ) stat2=STATUS_DND;

						if ( !noAutoMsgRequest )
						{
							switch ( stat2 )
							{
								case STATUS_AWAY      : sengMsgAutoRequest ( from, MSG_TYPE_AUTOAWAY ); break;
								case STATUS_OCCUPIED  : sengMsgAutoRequest ( from, MSG_TYPE_AUTOBUSY ); break;
								case STATUS_NA        : sengMsgAutoRequest ( from, MSG_TYPE_AUTONA ); break;
								case STATUS_DND       : sengMsgAutoRequest ( from, MSG_TYPE_AUTODND ); break;
								case STATUS_FREE4CHAT : sengMsgAutoRequest ( from, MSG_TYPE_AUTOFFC ); break;
							}
						}

						logMes_4 ( "pollIncomingEvents::Set status" );
						int uen_ind=findCLUIN ( from );

						if ( uen_ind<0 || ContactListUins[uen_ind].status_modifiers!=stat1 || ContactListUins[uen_ind].online_status!=stat2 || ContactListUins[uen_ind].invisible!=invis_flag )
						{
							if ( uen_ind>=0 )
							{
								ContactListUins[uen_ind].status_modifiers=stat1;
								ContactListUins[uen_ind].online_status=stat2;
								ContactListUins[uen_ind].invisible=invis_flag;
							}
							logMes_4 ( "emit onUserNotify(from, stat1, stat2, invis_flag)" );
							emit onUserNotify ( from, stat1, stat2, invis_flag );
						}
					}
					else if ( snd.service_id==0x0003 && snd.subtype_id==0x000c ) // Offline notification
					{
						string from;
						if ( !parseOfflineNotify ( from, snd.data ) ) return TNETWORK_TIMEOUT;
						int uen_ind=findCLUIN ( from );
						if ( uen_ind<0 )
						{
							logMes_4 ( "emit onUserNotify(from, 0, STATUS_OFFLINE, false)" );
							emit onUserNotify ( from, 0, STATUS_OFFLINE, false );
						}
						else
							if ( ContactListUins[uen_ind].status_modifiers!=0 || ContactListUins[uen_ind].online_status!=STATUS_OFFLINE || ContactListUins[uen_ind].invisible!=false )
							{
								ContactListUins[uen_ind].status_modifiers=0;
								ContactListUins[uen_ind].online_status=STATUS_OFFLINE;
								ContactListUins[uen_ind].invisible=false;
								ContactListUins[uen_ind].srv_relay_cap=false;
								ContactListUins[uen_ind].unicode_cap=false;
								logMes_4 ( "emit onUserNotify(from, 0, STATUS_OFFLINE, false)" );
								emit onUserNotify ( from, 0, STATUS_OFFLINE, false );
							}
					}
					else if ( snd.service_id==0x0013 && snd.subtype_id==0x0008 ) // SSI add item notification
					{
						if ( !parseSSIAddNotify ( snd.data ) ) return TNETWORK_TIMEOUT;
						logMes_4 ( "emit onContactListChanged()" );
						emit onContactListChanged();
					}
					else
						if ( snd.service_id==0x0013 && snd.subtype_id==0x0009 ) // SSI update item notification
						{
							if ( !parseSSIUpdateNotify ( snd.data ) ) return TNETWORK_TIMEOUT;
							logMes_4 ( "emit onContactListChanged()" );
							emit onContactListChanged();
						}
						else
							if ( snd.service_id==0x0013 && snd.subtype_id==0x000a ) // SSI delete item notification
							{
								if ( !parseSSIDeleteNotify ( snd.data ) ) return TNETWORK_TIMEOUT;
								logMes_4 ( "emit onContactListChanged()" );
								emit onContactListChanged();
							}
							else
								if ( snd.service_id==0x0013 && snd.subtype_id==0x001c ) // You were added notification
								{
									string from;
									if ( !parseWasAdded ( from, snd.data ) ) return TNETWORK_TIMEOUT;
									logMes_4 ( "emit onWasAdded(from)" );
									emit onWasAdded ( from );
								}
								else
									if ( snd.service_id==0x0004 && snd.subtype_id==0x0014 ) // Incoming MTN
									{
										string from;
										uint16_t type;
										if ( !parseIncomingMTN ( from, type, snd.data ) ) return TNETWORK_TIMEOUT;
										logMes_4 ( "emit onIncomingMTN(from, type)" );
										emit onIncomingMTN ( from, type );
									}
									else
									{
										logMes_4 ( "ICQKid2::pollIncomingEvents end" );
										return TNETWORK_TIMEOUT;
									}

	logMes_4 ( "ICQKid2::pollIncomingEvents end" );
	return 1;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
void ICQKid2::sendXStatusNotifyAutoResponse ( string touin, uint8_t * msg_cookie )
{
	logMes_4 ( "ICQKid2::sendXStatusNotifyAutoResponse" );
	if ( xStatus==X_STATUS_NONE || xStatus>X_STATUS_MAX_BOUND ) return;
	vector<uint8_t> data;
	size_t pos=0;

	if ( enabledEye )
	{
		int uen_ind=findCLUIN ( touin );
		itemEye item;
		item.uin = touin;
		if ( uen_ind>=0 )
		{
			item.nick = ContactListUins[uen_ind].nick;
		}
		item.type = 1; //Read xStatus
		item.time = QTime::currentTime().toString();
		listEye.push_back ( item );
	}

	data.resize ( pos+8 );
	memcpy ( &data[pos], msg_cookie, 8 );
	pos+=8;

	data.resize ( pos+3 );
	data[pos]=0;
	data[pos+1]=2; // message channel
	data[pos+2]=touin.length();
	pos+=3;

	data.resize ( pos+touin.length() );
	memcpy ( &data[pos], touin.data(), touin.length() );
	pos+=touin.length();

	data.resize ( pos+2 );
	data[pos]=0;
	data[pos+1]=3; // reason code - channel specific
	pos+=2;

	ostringstream plug_msg;
	plug_msg << "<NR><RES>";
	plug_msg <<  "&lt;ret event='OnRemoteNotification'&gt;";
	plug_msg <<   "&lt;srv&gt;";
	plug_msg <<    "&lt;id&gt;&lt;/id&gt;";
	plug_msg <<    "&lt;val srv_id=''&gt;undefined&lt;/val&gt;";
	plug_msg <<   "&lt;/srv&gt;";
	plug_msg <<   "&lt;srv&gt;";
	plug_msg <<    "&lt;id&gt;cAwaySrv&lt;/id&gt;";
	plug_msg <<    "&lt;val srv_id='cAwaySrv'&gt;";
	plug_msg <<     "&lt;Root&gt;";
	plug_msg <<      "&lt;CASXtraSetAwayMessage&gt;&lt;/CASXtraSetAwayMessage&gt;";
	plug_msg <<      "&lt;uin&gt;" << myuin << "&lt;/uin&gt;";
	plug_msg <<      "&lt;index&gt;" << xStatus << "&lt;/index&gt;";
	plug_msg <<      "&lt;title&gt;" << xStatusTitle << "&lt;/title&gt;";
	plug_msg <<      "&lt;desc&gt;" << xStatusDescription << "&lt;/desc&gt;";
	plug_msg <<     "&lt;/Root&gt;";
	plug_msg <<    "&lt;/val&gt;";
	plug_msg <<   "&lt;/srv&gt;";
	plug_msg <<   "&lt;srv&gt;";
	plug_msg <<    "&lt;id&gt;cRandomizerSrv&lt;/id&gt;";
	plug_msg <<    "&lt;val srv_id='cRandomizerSrv'&gt;undefined&lt;/val&gt;";
	plug_msg <<   "&lt;/srv&gt;";
	plug_msg <<  "&lt;/ret&gt;";
	plug_msg << "</RES></NR>\r\n";

	uint8_t xtraz_script_guid[] = {0x3b, 0x60, 0xb3, 0xef, 0xd8, 0x2a, 0x6c, 0x45, 0xa4, 0xe0, 0x9c, 0x5a, 0x5e, 0x67, 0xe8, 0x65};

	TLV2711 tlv_2711;
	tlv_2711.version=9;
	tlv_2711.cookie=0;
	tlv_2711.msg_type=0x1a; // Message type - Plugin message described by text string
	tlv_2711.flags=0;
	tlv_2711.status=0;
	tlv_2711.priority=0;
	tlv_2711.text="";
	memcpy ( tlv_2711.plug_guid, xtraz_script_guid, 16 );
	tlv_2711.plug_func_id=8; // Script notify
	tlv_2711.plug_name="Script Plug-in: Remote Notification Arrive";
	tlv_2711.xtraz_script_text=plug_msg.str();
	tlv_2711.encode_to ( data, pos );

	( void ) sendSNAC ( 0x0004, 0x000b, NULL, &data );
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::haveSrvRelayCapability ( vector<uint8_t> & data )
{
	logMes_4 ( "ICQKid2::haveSrvRelayCapability" );
	uint8_t srv_rel_arr[16]={ 0x09, 0x46, 0x13, 0x49, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 };   // ICQ Server relaying

	size_t caps_num=data.size() /16;
	for ( size_t i=0; i<caps_num; ++i )
		if ( memcmp ( &data[i*16], srv_rel_arr, 16 ) ==0 ) return true;

	return false;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
bool ICQKid2::haveUnicodeCapability ( vector<uint8_t> & data )
{
	logMes_4 ( "ICQKid2::haveUnicodeCapability" );
	uint8_t unicode_arr[16]={ 0x09, 0x46, 0x13, 0x4e, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 };  // ICQ UTF-8
// uint8_t old_unicode_arr[16]={ 0x2e, 0x7a, 0x64, 0x75, 0xfa, 0xdf, 0x4d, 0xc8, 0x88, 0x6f, 0xea, 0x35, 0x95, 0xfd, 0xb6, 0xdf }; // Old ICQ UTF-8

	size_t caps_num=data.size() /16;
	for ( size_t i=0; i<caps_num; ++i )
	{
		if ( memcmp ( &data[i*16], unicode_arr, 16 ) ==0 ) return true;
//  if (memcmp(&data[i*16], old_unicode_arr, 16)==0) return true;
	}

	return false;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
size_t ICQKid2::haveXtrazCapability ( vector<uint8_t> & data )
{
	logMes_4 ( "ICQKid2::haveXtrazCapability" );
	size_t caps_num=data.size() /16;
	for ( size_t i=0; i<caps_num; ++i )
		for ( size_t j=0; j<X_STATUS_MAX_BOUND; ++j )
			if ( memcmp ( &data[i*16], XStatus_arr[j], 16 ) ==0 )
				return j+1;

	return 0;
}

// ----------------=========ooooOOOOOOOOOoooo=========----------------
void ICQKid2::sendXtrazRequest ( string uin )
{
	logMes_4 ( "sendXtrazRequest" );
	BaseOutgoingMsg bom;
	bom.msg_channel=0x0002;
	bom.uin=uin;
	memset ( bom.msg_cookie, 0xAB, 8 );

	ExtendedMsg ex_msg;
	ex_msg.user_class=0x0050;
	ex_msg.user_status= ( online_status|STATUS_DCDISABLED );
	ex_msg.online_time=0;
	ex_msg.create_time=0;

	uint8_t icq_serv_relay[] = {0x09, 0x46, 0x13, 0x49, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00};
	ex_msg.rv_msg.type=0; // request
	memcpy ( ex_msg.rv_msg.capability, icq_serv_relay, 16 );
	ex_msg.rv_msg.external_ip=0;
	ex_msg.rv_msg.external_port=0;
	memcpy ( ex_msg.rv_msg.cookie, bom.msg_cookie, 8 );

	string plug_msg1 = "<N><QUERY>&lt;Q&gt;&lt;PluginID&gt;srvMng&lt;/PluginID&gt;&lt;/Q&gt;</QUERY><NOTIFY>&lt;srv&gt;&lt;id&gt;cAwaySrv&lt;/id&gt;&lt;req&gt;&lt;id&gt;AwayStat&lt;/id&gt;&lt;trans&gt;1&lt;/trans&gt;&lt;senderId&gt;";
	string plug_msg2 = "&lt;/senderId&gt;&lt;/req&gt;&lt;/srv&gt;</NOTIFY></N>";

	uint8_t xtraz_script_guid[] = {0x3b, 0x60, 0xb3, 0xef, 0xd8, 0x2a, 0x6c, 0x45, 0xa4, 0xe0, 0x9c, 0x5a, 0x5e, 0x67, 0xe8, 0x65};

	ex_msg.rv_msg.tlv2711.version=9;
	ex_msg.rv_msg.tlv2711.cookie=0;
	ex_msg.rv_msg.tlv2711.msg_type=0x1a; // Message type - Plugin message described by text string
	ex_msg.rv_msg.tlv2711.flags=0;
	ex_msg.rv_msg.tlv2711.status=0;
	ex_msg.rv_msg.tlv2711.priority=0;
	ex_msg.rv_msg.tlv2711.text="";
	memcpy ( ex_msg.rv_msg.tlv2711.plug_guid, xtraz_script_guid, 16 );
	ex_msg.rv_msg.tlv2711.plug_func_id=8; // Script notify
	ex_msg.rv_msg.tlv2711.plug_name="Script Plug-in: Remote Notification Arrive";
	ex_msg.rv_msg.tlv2711.xtraz_script_text=plug_msg1+myuin+plug_msg2;

	ex_msg.encode_to ( bom.data );
	vector<uint8_t> data;
	bom.encode_to ( data );

	sendSNAC ( 0x0004, 0x0006, NULL, &data );
}

size_t ICQKid2::haveClientsCapability ( vector<uint8_t> & data )
{
	QMutexLocker locker ( &mutexClientData );
	logMes_4 ( "ICQKid2::haveClientsCapability" );
	size_t caps_num=data.size() /16;
	for ( size_t j=1; j<ICQ_CLIENTS_COUNT; ++j )
		for ( size_t i=0; i<caps_num; ++i )
		{
			if ( memcmp ( &data[i*16], icq_clients[j].data, icq_clients[j].len ) ==0 )
				return j;
		}

	return 0;
}

QString ICQKid2::getClientName ( size_t id )
{
	QMutexLocker locker ( &mutexClientData );
	logMes_4 ( "ICQKid2::getClientName - " + QString::number(id) );
	if ( id >= ICQ_CLIENTS_COUNT )
		return "n/a";
	return icq_clients[id].name;
}

QString ICQKid2::getClientImageNeme ( size_t id )
{
	QMutexLocker locker ( &mutexClientData );
	logMes_4 ( "ICQKid2::getClientImageNeme" );
	if ( id >= ICQ_CLIENTS_COUNT )
		return "";	
	return icq_clients[id].filename;
}
