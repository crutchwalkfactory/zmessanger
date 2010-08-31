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

#include "zgui.h"
#include "GUI_Define.h"
#include "BaseDlg.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <resolv.h> // For using inet on USB

#include <ZApplication.h>
#include <ZListBox.h>
#include <ZConfig.h>
#include <ZMessageDlg.h>

#include <qcopchannel_qws.h>
#include <qtextcodec.h>
#include <qtimer.h>
#include <qdir.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qdatetime.h>
#include <qtextstream.h>
#include <qclipboard.h>


#include "ZAboutDlg.h"
#include "ZEye.h"
#include "ZSettings.h"

#include <ZSingleCaptureDlg.h>
#include <ZSingleSelectDlg.h>

#include "ZIMethod.h"

#ifdef _XMPP
#include <gloox.h>
#endif

#include "zDefs.h"

#define MENU_MY_STATUS_ID 2
#define MENU_MY_XSTATUS_ID 3
#define MENU_MY_PSTATUS_ID 4

#ifdef CUTED_PLATFORM
#define insertItem(a,b,c,d,e,f) insertItem(QString("   ")+a,b,c,d,e,f)
#endif

static NAPILink *napi_link;

extern "C" void __napi_bindsocket2link(int fd) 
{
	if(napi_link)
		napi_link->bindSocket(fd);
}

void * slot_onClickMenu( void * status);

ZGui::ZGui ( QWidget* parent, const char* name, WFlags fl )
    :ZKbMainWidget ( ZHeader::MAINDISPLAY_HEADER, parent, name, fl )
{
	getProgramDir();
	QCopChannel* channel = new QCopChannel(ZMESSANGER_CHENEL,this,"zMessanger");
	connect( channel, SIGNAL(received(const QCString&, const QByteArray&)), this, SLOT( siganalReceived(const QCString&, const QByteArray&) ) );
	CreateWindow ( parent );
}

void ZGui::siganalReceived( const QCString &msg, const QByteArray & data )
{
	if ( msg == "openChat(ZContactItem*)" )
	{
		QDataStream s( data, IO_ReadOnly ); 
		int item;
		s >> item;
		openChat((ZContactItem *)item);
		return;
	} else
	if ( msg == "showError(str,str,int)" )
	{
		QDataStream s( data, IO_ReadOnly ); 
		QString title, mes;
		int ico;
		s >> title;
		s >> mes;
		s >> ico;
		ZMessageDlg * dlg = new ZMessageDlg(title, mes, ZMessageDlg::TypeOK, 0, myWidget);
		switch ( ico )
		{
			case 1:
				dlg->setMsgIcon(ICON_DLG_ERROR);
				break;
		}
		dlg->exec();
		delete dlg;
		dlg = NULL;
		return;
	} else
	if ( msg == "onConnectChange(int,bool)" )
	{
		QDataStream s( data, IO_ReadOnly ); 
		int protocol;
		int online;
		s >> protocol;
		s >> online;
		#ifdef _XMPP		
		if ( protocol == PROT_ICQ )
		{
			if ( online )
			{
				if ( !xmppSet || xmpp->connected )
				{
					closeProc();
					createMenuConected();
					ZHeader::changeStatus(ZHeader::IM, 1);
				}
			} else
			{
				icqSet = false;
				if ( !xmpp->connected && xmppSet )
				{
					//Not action
				} else
				if ( xmpp->connected )
				{
					closeProc();
					createMenuConected();
					ZHeader::changeStatus(ZHeader::IM, 1);
				} 
				else if ( !xmppSet )
				{
					closeProc();
					showProfileList();
					createMenuDisconected();
					ZHeader::changeStatus(ZHeader::IM, 0);
				}
			}
	
		}else 
		if ( protocol == PROT_JABBER )
		{
			if ( online )
			{
				if ( !icqSet || icq->connected )
				{
					closeProc();
					createMenuConected();
					ZHeader::changeStatus(ZHeader::IM, 1);
				}
			} else
			{
				xmppSet = false;
				if ( !icq->connected && icqSet )
				{
					//Not action
				} else
				if (  icq->connected )
				{
					closeProc();
					createMenuConected();
					ZHeader::changeStatus(ZHeader::IM, 1);
				} else
				if ( !icqSet )
				{
					closeProc();
					showProfileList();
					createMenuDisconected();
					ZHeader::changeStatus(ZHeader::IM, 0);
				}			
			}
		}
		#else
			if ( online )
			{
				closeProc();
				createMenuConected();
				ZHeader::changeStatus(ZHeader::IM, 1);
			} else
			{
				closeProc();
				showProfileList();
				createMenuDisconected();
				ZHeader::changeStatus(ZHeader::IM, 0);
			}
		#endif
	
		return;
	}	
	if ( msg == "showMainWin()" )
	{
		slot_Raise();
		return;
	}
}

ZGui::~ZGui()
{
	qApp->removeEventFilter( this );
	
	ZHeader::changeStatus(ZHeader::IM, 5);
	ZHeader::unregisterInfo(ZHeader::IM);
	
	if (m_pHeader != NULL)
	{
		delete m_pHeader;
		m_pHeader = NULL;
	}

	saveHistory();

	if ( icq->connected )
	{
		icq->Disconnect();
	}
	delete icq;
	#ifdef _XMPP
	delete xmpp;
	#endif
	delete myWidget;
}

void ZGui::CreateWindow ( QWidget* )
{	
	logMes_2("CreateWindow: Start create window");
	setMainWidgetTitle ( "zMessanger" );
	
	logMes_3("CreateWindow: Create napi");
	napi = new NAPIManager();
	link = NULL;
	napi_link = 0;
	QObject::connect(napi, SIGNAL(connected()), this, SLOT(slot_internetConnected()));
	
	logMes_3("CreateWindow: Create ZWidget");
	myWidget = new ZWidget ( this, NULL, 0, ( ZSkinService::WidgetClsID ) 40 );

  	menustatus = NULL;
	menuContact = NULL;
	menuCL = NULL;
	menu = NULL;
	menuProfile = NULL;
	
	logMes_3("CreateWindow: Create ZMyListBox");
	lbContact = new ZMyListBox ( QString( CONTACT_ITEM_TYPE ), myWidget, 0);
	#ifdef CUTED_PLATFORM
	ZFormContainer *form = new ZFormContainer(this, 0, ZSkinService::clsZFormContainer);
	form->addChild(lbContact);
	setContentWidget(form);
	#else
	setContentWidget(lbContact);
	#endif

	lbContact->setFixedWidth ( SCREEN_WIDTH ); 

	logMes_3("CreateWindow: Create ZSoftKey");
	softKey = new ZSoftKey("CST_2A", this, this);

	QRect rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	logMes_3("CreateWindow: Create Menu");
	menu = new ZOptionsMenu ( rect, softKey, NULL, 0); 
	menu->setItemSpacing(10);
	menu->setSpacing(0);

	menustatus = new ZOptionsMenu ( rect, softKey, NULL, 0); 
	menustatus->setItemSpacing(10);
	menustatus->setSpacing(0);	

	menuProfile = new ZOptionsMenu ( rect, softKey, NULL, 0); 
	menuProfile->setItemSpacing(10);
	menuProfile->setSpacing(0);

	menuCL = new ZOptionsMenu ( rect, softKey, NULL, 0); 
	menuCL->setItemSpacing(10);
	menuCL->setSpacing(0);

	menuContact = new ZOptionsMenu ( rect, softKey, NULL, 0); 
	menuContact->setItemSpacing(10);
	menuContact->setSpacing(0);

	menuPrivatStatus = new ZOptionsMenu ( rect, softKey, NULL, 0); 
	menuPrivatStatus->setItemSpacing(10);
	menuPrivatStatus->setSpacing(0);


	#ifdef _MainMenuFix
	connect( menu, SIGNAL( canceled() ), this, SLOT( slotFixMenuBag() ) );
	connect( menu, SIGNAL( activated( int ) ), this, SLOT( slotFixMenuBag( int ) ) );		
	connect( menuCL, SIGNAL( activated( int ) ), this, SLOT( slotFixMenuBag( int ) ) );	
	connect( menuPrivatStatus, SIGNAL( activated( int ) ), this, SLOT( slotFixMenuBag( int ) ) );	
	connect( menustatus, SIGNAL( activated( int ) ), this, SLOT( slotFixMenuBag( int ) ) );		
	#endif

	softKey->setOptMenu ( ZSoftKey::LEFT, menu );
	softKey->setText ( ZSoftKey::LEFT, LNG_MENU );
	softKey->setTextForOptMenuHide( LNG_MENU );
	setSoftKey( softKey );
	
	createMenuDisconected();	

	logMes_3("CreateWindow: connect slot");

	//***********for taskman***
	connect( qApp, SIGNAL(signalRaise()), this, SLOT(slot_Raise()) );
	connect( qApp, SIGNAL(askReturnToIdle(int)), this, SLOT(slot_ReturnToIdle(int)) );
		
	//*************************

	connect ( lbContact, SIGNAL ( selected ( int ) ), this, SLOT ( lbContactSel ( int ) ) );

	logMes_3("CreateWindow: create lib");

	icq = new zICQ();
	icq->ProgDir = ProgDir;
	icqSet=false;
	#ifdef _XMPP
	xmpp = new zXMPP();
	xmppSet=false;
	#endif
	  
	{
	logMes_3("CreateWindow: connect lib slot");
	slotActiv=false;
	activSlot( true );
	connect( icq, SIGNAL( onErrorConnect(QString) ), this, SLOT( slot_onErrorConnectICQ(QString) ) );
	connect( icq, SIGNAL( onConnected(bool) ), this, SLOT( slot_onConnectedICQ(bool) ) );
	connect( icq, SIGNAL( onWasAdded(string) ), this, SLOT( slot_onWasAdded(string) ) );		
	connect( icq, SIGNAL( onIncomingMsg(ICQKid2Message) ), this, SLOT( slot_onIncomingMsg(ICQKid2Message) ) );
	connect( icq, SIGNAL( onAuthRequest(string, string) ), this, SLOT( slot_onAuthRequest(string, string) ) );
	connect( icq, SIGNAL( onAuthReply(string, string, uint8_t) ), this, SLOT( slot_onAuthReply(string, string, uint8_t) ) );	
	#ifdef _XMPP
	connect( xmpp, SIGNAL( onConnected(bool) ), this, SLOT( slot_onConnectedXMPP(bool) ) );
	connect( xmpp, SIGNAL( onErrorConnect(QString) ), this, SLOT( slot_onErrorConnectXMPP(QString) ) );
	#endif		
	}

	showProfileList();

	zSmile = new zEmotIcons(ProgDir);

	logMes_3("CreateWindow: Read config");
	ZConfig cfg(ProgDir+"/zMessanger.cfg");
	
	string MyServ  = cfg.readEntry(QString("Inet"), QString("Server"), "login.icq.com").latin1();
	int MyPort  = cfg.readNumEntry(QString("Inet"), QString("Port"), 5190 );
	icq->setLoginHost(MyServ, MyPort);
	cfg_inetProfile = cfg.readEntry(QString("Inet"), QString("ProfileName"), "");
	cfg_timeKeepConnect = cfg.readNumEntry(QString("Inet"), QString("TimeKeepConnect"), 500); 
	icq->delayReadSnec = cfg.readNumEntry(QString("Inet"), QString("delayReadSnec"), 2000);
	QString codePage = cfg.readEntry(QString("Message"), QString("CodePage"), "CP1251");
	icq->noAutoXTrazRequest = cfg.readBoolEntry(QString("Message"), QString("noAutoXTrazRequest"), true);
	icq->noAutoMsgRequest = cfg.readBoolEntry(QString("Message"), QString("noAutoMsgRequest"), true);
	cfg_alertVibr = cfg.readBoolEntry(QString("Alert"), QString("Vibrate"), true);
	cfg_alertRing = cfg.readBoolEntry(QString("Alert"), QString("Ring"), false);
	cfg_alertRingVol = cfg.readBoolEntry(QString("Alert"), QString("Volume"), 2);
	cfg_alertPath = cfg.readEntry(QString("Alert"), QString("Path"), "");
	cfg_dontShowOffLine = cfg.readBoolEntry(QString("ContactList"), QString("dontShowOffLine"), true);
	cfg_dontShowGroup = cfg.readBoolEntry(QString("ContactList"), QString("dontShowGroup"), false);
	cfg_rigthAlignXStatus = cfg.readBoolEntry(QString("ContactList"), QString("rigthAlignXStatus"), true);
	cfg_sortType = cfg.readNumEntry(QString("ContactList"), QString("sortType"), 2);
	cfg_chatFontSize = cfg.readNumEntry(QString("Chat"), QString("chatFontSize"), 11);
	cfg_mesFontSize = cfg.readNumEntry(QString("Chat"), QString("writeMesFontSize"), 11);
	cfg_maxNumLines = cfg.readNumEntry(QString("Chat"), QString("maxNumLines"), 40);
	cfg_notSendTypeMes = cfg.readBoolEntry(QString("Chat"), QString("notSendTypeMes"), false);
	cfg_sendByCenter = cfg.readBoolEntry(QString("Chat"), QString("sendByCenter"), false);
	cfg_noShowStatusBarInChat = cfg.readBoolEntry(QString("Chat"), QString("noShowStatusBarInChat"), false);
	zSmile->setSmilePack(cfg.readEntry(QString("Chat"), QString("smilePack"), "kolobok"));
	icq->enabledEye = cfg.readBoolEntry(QString("Othe"), QString("enabledEye"), true);
	#ifdef _SupportZPlayer
	cfg_nowPlaying = cfg.readEntry(QString("Status"), QString("nowPlaying"), "%artist% - %title%");
	#endif
	icq->loadOldStatusPostConnect( cfg.readBoolEntry(QString("Status"), QString("saveStausCanDisconect"), true) );
	#ifndef _NoUSBNetOption
	cfg_InetOnUSB = cfg.readBoolEntry(QString("Inet"), QString("inetOnUSB"), false);
	#else
	cfg_InetOnUSB = false;	
	#endif
	
	dlgChat = NULL;
	dlgStat = NULL;
	
	codec = NULL;
	if ( !codePage.isEmpty() )
		codec = QTextCodec::codecForName( codePage );
	if ( !codec )
		codec = QTextCodec::codecForName( "CP1251" );
	
	isShown = true;

	lbContact->setShowGroup(!cfg_dontShowGroup);

	qApp->installEventFilter( this );

	logMes_3("CreateWindow: Add icon to 'tray'");

	m_pHeader = new ZHeader(ZHeader::MAINDISPLAY_HEADER, NULL);
	
	ZHeader::AppStatusInfo_S as;
	{
		as.status = (int)ZHeader::IM;
		as.numStatus = 6;
		as.priority = 3;
		as.appStatus = new ZHeader::AppStatus_S[6];
		
		as.appStatus[0].status = 0;
		as.appStatus[0].strResID[0] = "im_icq_cli4.k";
		as.appStatus[0].strResID[1] = "im_icq_cli4.k";
		as.appStatus[0].strResID[2] = "im_icq_idle5.k";
		
		as.appStatus[1].status = 1;
		as.appStatus[1].strResID[0] = "im_icq_online_cli4.k";
		as.appStatus[1].strResID[1] = "im_icq_online_cli4.k";
		as.appStatus[1].strResID[2] = "im_icq_online_idle5.k";
		as.appStatus[1].strResID[3] = "";
		
		as.appStatus[2].status = 2;
		as.appStatus[2].strResID[0] = "im_icq_invisible_cli4.k";
		as.appStatus[2].strResID[1] = "im_icq_invisible_cli4.k";
		as.appStatus[2].strResID[2] = "im_icq_invisible_idle5.k";
		as.appStatus[2].strResID[3] = "";
		
		as.appStatus[3].status = 3;
		as.appStatus[3].strResID[0] = "im_icq_away_cli4.k";
		as.appStatus[3].strResID[1] = "im_icq_away_cli4.k";
		as.appStatus[3].strResID[2] = "im_icq_away_idle5.k";
		as.appStatus[3].strResID[3] = "";
		
		as.appStatus[4].status = 4;
		as.appStatus[4].strResID[0] = "new_im_msn_cli5.k";
		as.appStatus[4].strResID[1] = "new_im_msn_cli5.k";
		as.appStatus[4].strResID[2] = "new_im_msn_idle5.k";
		as.appStatus[4].strResID[3] = "new_im_msn_hed2.k";
		
		as.appStatus[5].status = 5;
		as.appStatus[5].strResID[0] = "";
		as.appStatus[5].strResID[1] = "";
		as.appStatus[5].strResID[2] = "";
		as.appStatus[5].strResID[3] = "";
	}
	ZHeader::registerInfo(as);
	ZHeader::changeStatus(ZHeader::IM, 0);

	trayMes = false;
	showedChat = false;
	#ifdef _SupportZPlayer
	chenelZPlayer = 0;
	#endif
	logMes_2("CreateWindow: End create form");
}

void ZGui::activSlot( bool con )
{
	if (slotActiv == con) return;
	slotActiv = con;
	if ( con )
	{
	connect( icq, SIGNAL( onUserNotify(string, uint32_t, uint32_t, bool) ), this, SLOT( slot_onUserNotify(string, uint32_t, uint32_t, bool) ) );
	connect( icq, SIGNAL( onIncomingMTN(string, uint16_t) ), this, SLOT( slot_onIncomingMTN(string, uint16_t) ) );
	connect( icq, SIGNAL( onContactListChanged() ), this, SLOT( slot_onContactListChanged() ) );
	connect( icq, SIGNAL( onXstatusChanged(string, size_t, string, string) ), this, SLOT( slot_onXstatusChanged(string, size_t, string, string) ) );
	connect( icq, SIGNAL( onClientChange(string, size_t) ), this, SLOT( slot_onClientChange(string, size_t) ) );
	} else
	{
	disconnect( icq, SIGNAL( onUserNotify(string, uint32_t, uint32_t, bool) ), this, SLOT( slot_onUserNotify(string, uint32_t, uint32_t, bool) ) );
	disconnect( icq, SIGNAL( onIncomingMTN(string, uint16_t) ), this, SLOT( slot_onIncomingMTN(string, uint16_t) ) );
	disconnect( icq, SIGNAL( onContactListChanged() ), this, SLOT( slot_onContactListChanged() ) );
	disconnect( icq, SIGNAL( onXstatusChanged(string, size_t, string, string) ), this, SLOT( slot_onXstatusChanged(string, size_t, string, string) ) );
	disconnect( icq, SIGNAL( onClientChange(string, size_t) ), this, SLOT( slot_onClientChange(string, size_t) ) );		
	}
}

int ZGui::strtoint(string str)
{
	return std::atoi( str.c_str() );
}

void ZGui::createMenuConected()
{
	logMes_2("createMenuConected: start");
	
	menuProfile->clear();
	menustatus->clear();
	menuCL->clear();
	menuContact->clear();
	menu->clear();
	menuPrivatStatus->clear();
	
	QPixmap* pm  = new QPixmap();

	if ( icq->connected )
	{
		pm->load(ProgDir+ "/status/icq/online.png");
		menustatus->insertItem ( LNG_ONLINE , NULL, pm, true, 0, 0);
		pm->load(ProgDir+ "/status/icq/ffc.png");
		menustatus->insertItem ( LNG_FFC , NULL, pm, true, 1, 1);
		pm->load(ProgDir+ "/status/icq/away.png");
		menustatus->insertItem ( LNG_AWAY , NULL, pm, true, 2, 2);
		pm->load(ProgDir+ "/status/icq/na.png");
		menustatus->insertItem ( LNG_NA , NULL, pm, true, 3, 3);
		pm->load(ProgDir+ "/status/icq/occupied.png");
		menustatus->insertItem ( LNG_OCCUPED , NULL, pm, true, 4, 4 );
		pm->load(ProgDir+ "/status/icq/dnd.png");
		menustatus->insertItem ( LNG_DND , NULL, pm, true, 5, 5 );
		pm->load(ProgDir+ "/status/icq/invisible.png");
		menustatus->insertItem ( LNG_INVISIBLE , NULL, pm, true, 6, 6 );
		pm->load(ProgDir+ "/status/icq/evil.png");
		menustatus->insertItem ( LNG_EVIL , NULL, pm, true, 7, 7 );
		pm->load(ProgDir+ "/status/icq/depression.png");
		menustatus->insertItem ( LNG_DEPRESSION , NULL, pm, true, 8, 8);
		pm->load(ProgDir+ "/status/icq/athome.png");
		menustatus->insertItem ( LNG_ATHOME , NULL, pm, true, 9, 9 );
		pm->load(ProgDir+ "/status/icq/atwork.png");
		menustatus->insertItem ( LNG_ATWORCK , NULL, pm, true, 10, 10);
		pm->load(ProgDir+ "/status/icq/lunch.png");
		menustatus->insertItem ( LNG_LANCH , NULL, pm, true, 11, 11 );
		
		connect( menustatus, SIGNAL( activated( int ) ), this, SLOT( menu_statusChange( int ) ) );
		
		pm->load(ProgDir+ "/status/private/1.png");
		menuPrivatStatus->insertItem ( LNG_ALL_CAN_SEE , NULL, pm, true, 1, 1);
		pm->load(ProgDir+ "/status/private/2.png");
		menuPrivatStatus->insertItem ( LNG_NOBODY_CAN_SEE , NULL, pm, true, 2, 2);	
		pm->load(ProgDir+ "/status/private/3.png");
		menuPrivatStatus->insertItem ( LNG_VISLIST_CAN_SEE , NULL, pm, true, 3, 3);		
		pm->load(ProgDir+ "/status/private/4.png");
		menuPrivatStatus->insertItem ( LNG_INVISLIST_CANNOT_SEE , NULL, pm, true, 4, 4);
		pm->load(ProgDir+ "/status/private/5.png");
		menuPrivatStatus->insertItem ( LNG_CONTACTLIST_CAN_SEE , NULL, pm, true, 5, 5);		
		
		connect( menuPrivatStatus, SIGNAL( activated( int ) ), this, SLOT( menu_privStatusChange( int ) ) );					
	}

	pm->load(ProgDir+ "/menu/user_add.png");
	menuCL->insertItem ( LNG_ADDCONTACT , NULL, pm, true, 0, 0);	
	pm->load(ProgDir+ "/menu/group_add.png");
	menuCL->insertItem ( LNG_ADDGRPUP , NULL, pm, true, 1, 1);	
	pm->load(ProgDir+ "/menu/group_remove.png");
	menuCL->insertItem ( LNG_REMOVEGRPUP , NULL, pm, true, 2, 2);	
	pm->load(ProgDir+ "/menu/user_remove.png");
	menuCL->insertItem ( LNG_REMOVECONTACT , NULL, pm, true, 3, 3);	
	
	menuCL->connectItem ( 0, this, SLOT ( menu_addContact() ) );
	menuCL->connectItem ( 3, this, SLOT ( menu_removeContact() ) );
	menuCL->connectItem ( 1, this, SLOT ( menu_addGroup() ) );
	menuCL->connectItem ( 2, this, SLOT ( menu_removeGroup() ) );

	pm->load(ProgDir+ "/menu/disconnect.png");
	menu->insertItem ( LNG_DISCONNECT , NULL, pm, true, 0, 0 );	
  	menu->insertSeparator(1, 1);

	if ( icq->connected )
	{
		QString ico;
		switch( icq->getMyOnlineStatus() )
		{
			case STATUS_ONLINE:
				ico = "online.png"; break;
			case STATUS_AWAY:
				ico = "away.png"; break;
			case STATUS_DND:
				ico = "dnd.png"; break;
			case STATUS_NA:
				ico = "na.png"; break;
			case STATUS_OCCUPIED:
				ico = "occupied.png"; break;
			case STATUS_FREE4CHAT:
				ico = "ffc.png"; break;
			case STATUS_EVIL:
				ico = "evil.png"; break;
			case STATUS_DEPRESSION:
				ico = "depression.png"; break;
			case STATUS_HOME:
				ico = "athome.png"; break;
			case STATUS_WORK:
				ico = "atwork.png"; break;
			case STATUS_LUNCH: 
				ico = "lunch.png"; break;
			case STATUS_INVISIBLE:
				ico = "invisible.png"; break;
		}
		pm->load(ProgDir+ "/status/icq/"+ico);
		menu->insertItem ( LNG_STATUS , menustatus, pm, true, MENU_MY_STATUS_ID, MENU_MY_STATUS_ID );
		
		if ( icq->getMyXStatus() == X_STATUS_NONE )
			pm->load(ProgDir+ "/menu/xStatus.png");	
		else
			pm->load( ProgDir + QString ( "/status/icq/x/icq_xstatus" ) + QString::number(icq->getMyXStatus()-1) + ".png");
		menu->insertItem ( LNG_XSTATUS , NULL, pm, true, MENU_MY_XSTATUS_ID, MENU_MY_XSTATUS_ID );
		
		pm->load( ProgDir + QString ( "/status/private/" ) + QString::number(icq->getMyPrivacyStatus()) + ".png");
		menu->insertItem ( LNG_PRIVATE_STATUS , menuPrivatStatus, pm, true, MENU_MY_PSTATUS_ID, MENU_MY_PSTATUS_ID);	
		
		menu->insertSeparator(5, 5); 
	}

  	pm  = new QPixmap();
	menu->insertItem ( LNG_CONTACTLIST , menuCL, pm, true, 6, 6 );
	menu->insertSeparator(7, 7); 
	pm->load(ProgDir+ "/menu/settings.png");
	menu->insertItem ( LNG_SETTING , NULL, pm, true, 8, 8 );
	pm->load(ProgDir+ "/menu/about.png");	
	menu->insertItem ( LNG_ABOUT , NULL, pm, true, 9, 9 );
  	menu->insertSeparator(10, 10);
  	pm->load(ProgDir+ "/menu/hide.png");
  	menu->insertItem ( LNG_MINIMIZE , NULL, pm, true, 11,11 );
  	pm->load(ProgDir+ "/menu/exit.png");
	menu->insertItem ( LNG_EXIT , NULL, pm, true, 12, 12 );	

	menu->connectItem ( 0, this, SLOT ( menu_connect() ) );
	menu->connectItem ( 3, this, SLOT ( menu_setXStatus() ) );
	menu->connectItem ( 8, this, SLOT ( menu_settings() ) );
	menu->connectItem ( 9, this, SLOT ( menu_about() ) );	
	menu->connectItem ( 11, this, SLOT ( menu_minimize() ) );
	menu->connectItem ( 12, qApp, SLOT ( quit() ) );
	
	pm->load(ProgDir+ "/menu/user_chat.png");
	menuContact->insertItem ( LNG_OPENCHAT , NULL, pm, true, 0, 0);
	pm->load(ProgDir+ "/menu/xStatus.png");	
	menuContact->insertItem ( LNG_STATUSS , NULL, pm, true, 1, 1);
	pm->load(ProgDir+ "/menu/user_info.png");
	menuContact->insertItem ( LNG_MENU_USERINFO , NULL, pm, true, 2, 2);
	pm->load(ProgDir+ "/menu/copy.png");
	menuContact->insertItem ( LNG_MENU_COPYUIN , NULL, pm, true, 3, 3);
	menuContact->connectItem ( 3, this, SLOT ( menu_copyUin() ) );
	menuContact->connectItem ( 0, this, SLOT ( menu_showChat() ) );
	menuContact->connectItem ( 1, this, SLOT ( menu_showXStatus() ) );
	menuContact->connectItem ( 2, this, SLOT ( menu_userInfo() ) );
	
	softKey->setOptMenu ( ZSoftKey::RIGHT, menuContact );
	softKey->setText ( ZSoftKey::RIGHT, LNG_MENUCONTACT, ( ZSoftKey::TEXT_PRIORITY ) 0 );
}

void ZGui::createMenuDisconected()
{
	logMes_2("createMenuDisconected: run");
	menuProfile->clear();
	menustatus->clear();
	menuCL->clear();
	menuContact->clear();
	menu->clear();
	menuPrivatStatus->clear();
	
	QPixmap* pm  = new QPixmap();
	
	pm->load(ProgDir+ "/menu/connect.png");
	menu->insertItem ( LNG_CONNECT , NULL, pm, true, 0, 0 );	
	menu->insertSeparator(1, 1);
	#ifdef _VersionTest
	pm  = new QPixmap();
	menu->insertItem ( "Open chat" , NULL, pm, true, 2, 2 );		
	menu->insertSeparator(3, 3); 
	#endif
	pm->load(ProgDir+ "/menu/settings.png");
	menu->insertItem ( LNG_SETTING , NULL, pm, true, 4, 4 );	
	pm->load(ProgDir+ "/menu/about.png");
	menu->insertItem ( LNG_ABOUT , NULL, pm, true, 5, 5 );
	menu->insertSeparator(6, 6); 
	pm->load(ProgDir+ "/menu/hide.png");
	menu->insertItem ( LNG_MINIMIZE , NULL, pm, true, 7, 7 );	
	pm->load(ProgDir+ "/menu/exit.png");  	
	menu->insertItem ( LNG_EXIT , NULL, pm, true, 8, 8 );
	
	menuProfile->clear();
	
	pm->load(ProgDir+ "/menu/profile_add.png");
	menuProfile->insertItem ( LNG_ADD , NULL, pm, true, 0, 0 );
	pm->load(ProgDir+ "/menu/profile_del.png");
	menuProfile->insertItem ( LNG_REMOVE , NULL, pm, true, 1, 1 );
	pm->load(ProgDir+ "/menu/profile_edit.png");
	menuProfile->insertItem ( LNG_CHANGE , NULL, pm, true, 2, 2 );	
	pm->load(ProgDir+ "/menu/auth_acepted.png");
	menuProfile->insertItem ( LNG_SET_DEFS , NULL, pm, true, 3, 3 );
	
	menu->connectItem ( 0, this, SLOT ( menu_connect() ) );
	#ifdef _VersionTest	
	menu->connectItem ( 2, this, SLOT ( openChatTest() ) );
	#endif
	menu->connectItem ( 4, this, SLOT ( menu_settings() ) );
	menu->connectItem ( 5, this, SLOT ( menu_about() ) );	
	menu->connectItem ( 7, this, SLOT ( menu_minimize() ) );
	menu->connectItem ( 8, qApp, SLOT ( quit() ) );

	menuProfile->connectItem ( 0, this, SLOT ( menu_profile_add() ) );
	menuProfile->connectItem ( 1, this, SLOT ( menu_profile_del() ) );
	menuProfile->connectItem ( 2, this, SLOT ( menu_profile_change() ) );
	menuProfile->connectItem ( 3, this, SLOT ( menu_profile_setdef() ) );	
	
	softKey->setOptMenu ( ZSoftKey::RIGHT, menuProfile );
	softKey->setText ( ZSoftKey::RIGHT, LNG_MENU_PROFILES, ( ZSoftKey::TEXT_PRIORITY )0 );
}

void ZGui::alert()
{
	if (cfg_alertRing)
	{
		system( QFile::encodeName( "/usr/SYSqtapp/phone/alertprocess -playMode 1 -playvol "+QString::number(cfg_alertRingVol)+" -playfile '"+cfg_alertPath+"' &" ) );
	}
	if (cfg_alertVibr)
	{
		system( QFile::encodeName( "/usr/SYSqtapp/phone/vibrateprocess &" ) );
	}
}

void * slot_onClickMenu( void * status)
{
	logMes_1("slot_onClickMenu: status chenge to "+QString::number((int)status));
	
	QPixmap* pm  = new QPixmap();
	
	QString ico;
	uint st = 0;
	switch( (int) status)
	{
		case 0:
			st = STATUS_ONLINE; ico = "online.png";
			if (!zgui->trayMes) ZHeader::changeStatus(ZHeader::IM, 1); break;
		case 2:
			st = STATUS_AWAY; ico = "away.png";
			if (!zgui->trayMes) ZHeader::changeStatus(ZHeader::IM, 3); break;
		case 5:
			st =  STATUS_DND; ico = "dnd.png";
			if (!zgui->trayMes) ZHeader::changeStatus(ZHeader::IM, 3); break;
		case 3:
			st =  STATUS_NA; ico = "na.png";
			if (!zgui->trayMes) ZHeader::changeStatus(ZHeader::IM, 3); break;
		case 4:
			st =  STATUS_OCCUPIED; ico = "occupied.png";
			if (!zgui->trayMes) ZHeader::changeStatus(ZHeader::IM, 3); break;
		case 1:
			st =  STATUS_FREE4CHAT; ico = "ffc.png";
			if (!zgui->trayMes) ZHeader::changeStatus(ZHeader::IM, 1); break;
		case 7:
			st =  STATUS_EVIL; ico = "evil.png";
			if (!zgui->trayMes) ZHeader::changeStatus(ZHeader::IM, 1); break;
		case 8:
			st =  STATUS_DEPRESSION; ico = "depression.png";
			if (!zgui->trayMes) ZHeader::changeStatus(ZHeader::IM, 1); break;
		case 9:
			st =  STATUS_HOME; ico = "athome.png";
			if (!zgui->trayMes) ZHeader::changeStatus(ZHeader::IM, 1); break;
		case 10:
			st =  STATUS_WORK; ico = "atwork.png";
			if (!zgui->trayMes) ZHeader::changeStatus(ZHeader::IM, 1); break;
		case 11: 
			st =  STATUS_LUNCH; ico = "lunch.png";
			if (!zgui->trayMes) ZHeader::changeStatus(ZHeader::IM, 1); break;
		case 6:
			st =  STATUS_INVISIBLE; ico = "invisible.png";
			if (!zgui->trayMes) ZHeader::changeStatus(ZHeader::IM, 2);
	}

	if ( zgui->icq->setStatus(st) )
	{
		pm->load(ProgDir+ "/status/icq/"+ ico);
		zgui->menu->changeItem( MENU_MY_STATUS_ID, LNG_STATUS, pm );
	}
	pthread_exit(NULL);
}

void ZGui::menu_statusChange(int status)
{
	menustatus->hide();
	menu->hide();

	pthread_attr_t attr;
	pthread_t thread;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&thread, &attr, slot_onClickMenu, (void *)status);
	pthread_attr_destroy(&attr);	
}

void ZGui::menu_privStatusChange(int status)
{
	if (icq->setMyPrivacyStatus(status))
		menu->changeItem( MENU_MY_PSTATUS_ID, LNG_PRIVATE_STATUS, new QPixmap(ProgDir+ "/status/private/"+QString::number(status)+".png") );	
}

void ZGui::slot_onConnectedICQ(bool conect)
{
	onConnectChange(PROT_ICQ, conect); 
}

#ifdef _XMPP
void ZGui::slot_onConnectedXMPP(bool conect)
{
	onConnectChange(PROT_JABBER, conect);
}
#endif

void ZGui::onConnectChange (int protocol, bool online) 
{
	QByteArray data;
	QDataStream s( data, IO_WriteOnly ); 
	s << protocol;
	s << (int)online;
	QCopChannel::send( ZMESSANGER_CHENEL, "onConnectChange(int,bool)", data );
}

void ZGui::slot_onUserNotify(string uin, uint32_t stat1, uint32_t stat2, bool invis_flag)
{
	logMes_3("ZGui::slot_onUserNotify");
	if ( stat2 == STATUS_OFFLINE && cfg_dontShowOffLine )
	{
		logMes_3("delete contact");
		lbContact->contactRemove(uin);
	} else
	{	
		ZContactItem * contact = getICQContact(uin);
		if (contact != NULL)
		{
			contact->setStatus(invis_flag?STATUS_INVISIBLE:stat2);
			if ( STATUS_BIRTHDAY & stat1 )
				contact->setBirthday(true);
		}
	}
	if (stat2 == STATUS_OFFLINE && dlgChat != NULL)
	{
		if ( dlgChat->uin == uin )
			dlgChat->setContOff();
	}
}

void ZGui::slot_onIncomingMsg(ICQKid2Message msg)
{
	QString mes;
	switch (msg.enc_type)
	{
		case ICQKid2Message::USASCII :
			logMes_3("Coded: US-ASCII encoded");
			mes = strtoqstr( msg.text.c_str() );
			break;
		case ICQKid2Message::LOCAL8BIT :
			logMes_3("Coded: 8 bit encoded with "+QString::number(msg.codepage)+" codepage");
			mes = codec->toUnicode( msg.text.c_str() );
			break;
		case ICQKid2Message::UCS2BE :
			logMes_3("Coded: UCS-2 Big endian encoded");
			mes=icq->unicodeToUtf8( msg.text );
			break;
		case ICQKid2Message::UTF8 :
			logMes_3("Coded: UTF-8 encoded");
			mes = QString::fromUtf8( msg.text.c_str() );
			break;
	}
	
	int uin_ind = icq->findContact( msg.uin );
	if ( uin_ind < 0 ) // if not exist contact
		uin_ind = icq->addContactToNoCLList(QString(msg.uin.c_str()),QString(msg.uin.c_str()));
	icq->setMesIcon(uin_ind, true);
	
	QDateTime time;
	time.setTime_t(msg.timestamp);	
	
	newMes( QString::fromLocal8Bit( msg.uin.c_str() ),
		QString::fromLocal8Bit(icq->getNick(uin_ind).c_str() ),
		mes, time, TYPE_MESSAGE );

	alert();
}

void ZGui::showProfileList()
{
	logMes_2("showProfileList: start");
	lbContact->clear();

	ZConfig cfg(ProgDir+"/zMessanger.cfg");

	QString uin;
	int icqCount=0;
	int checkId = cfg.readNumEntry(QString("Login"), QString("DefaultICQ"), 0);
	for (int i=1;i<20;i++)
	{
		uin = cfg.readEntry(QString("Login"), QString("UIN"+QString::number(i)), "");

		if (uin != "")
		{
			ZContactItem* listitem = new ZContactItem(lbContact, ITEM_PROFILE );
			logMes_2("showProfileList: "+uin);
			listitem->setNick( uin );
			listitem->setProtocol( PROT_ICQ );
			listitem->setStatus( STATUS_ONLINE );
			listitem->setReservedData ( i );
			if ( i == checkId )
				lbContact->checkItem(listitem,true);
			lbContact->insertItemInList ( listitem, -1, true );	
			icqCount++;
		} else
		{
			break;
		}
	}
	
	#ifdef _XMPP
	checkId = cfg.readNumEntry(QString("Login"), QString("DefaultJ"), 0);
	for (int i=1;i<20;i++)
	{
		uin = cfg.readEntry(QString("Login"), QString("JID"+QString::number(i)), "");

		if (uin != "")
		{
			ZContactItem* listitem = new ZContactItem(lbContact, ITEM_PROFILE );
			logMes_2("showProfileList: "+uin);
			listitem->setNick( uin );
			listitem->setProtocol( PROT_JABBER );
			lbContact->insertItem ( listitem,-1,true );
			listitem->setReservedData ( i );
			if ( i == checkId )
				lbContact->checkItem(listitem,true);
		} else
		{
			break;
		}
	}
	#endif
	
	logMes_2("showProfileList: end");
}

void ZGui::saveHistory()
{
	logMes_3("saveHistory: start");
	//Save history to file
	QDir dirHist;
	QString histPath = ProgDir+"history/";
	logMes_3("saveHistory: "+histPath);
	dirHist.setPath(histPath);
	if ( !dirHist.exists() )
	{
		logMes("Not found folder");
		QString str = QString ( "mkdir -p %1" ).arg ( histPath );
		system ( str.utf8() );
	}

	logMes_3("saveHistory: Print history");
	mesList messanges;
	QString str;
	for ( tHistory::Iterator it = messageList.begin( ); it != messageList.end( ); it++) 
	{ 
		logMes_3("saveHistory: save to - "+histPath+QString::number(it.key())+".txt");
		QFile file( histPath+QString::number(it.key())+".txt" );
		if ( file.open( IO_WriteOnly | IO_Append ) ) 
		{
			logMes_3("saveHistory: file opened");
			QTextStream stream( &file );
			stream.setEncoding(QTextStream::UnicodeUTF8);
			messanges = it.data();
			logMes_3("saveHistory: print message");
			for ( uint n = 0; n < messanges.count(); n++ )
			{	
				str = bdMesToText(  *messanges.at(n) , true  );
				if (str != "")
					stream << str << "\n";
			}
			logMes_3("saveHistory: end print");
		}
		file.close();
	}
	logMes_3("saveHistory: end");
}

void ZGui::slot_onIncomingMTN(string from, uint16_t atype)
{
	ZContactItem * contact = getICQContact(from);
	if (contact == NULL)
		return;	
		
	switch(atype)
	{
	case MTN_BEGIN:
		contact->setType(true);
		break;	
	case MTN_TYPED:
		contact->setType(false);
		break;	
	case MTN_FINISH:
		contact->setType(false);
		break;
	}
	
	if (dlgChat != NULL)
	{
		if ( dlgChat->uin == from )
		{
			switch(atype)
			{
			case MTN_BEGIN :
				dlgChat->setType(LNG_TYPE);
				break;		
			case MTN_TYPED : 
				break;	
			case MTN_FINISH :
				dlgChat->setType("");
				break;
			//default : 
			}
		}
	}
}

#ifdef _XMPP
void ZGui::slot_onChangeTextChat(std::string jid, QString mes)
{
	logMes_2("slot_onChangeTextChat: newMes to conferense");
	if ( dlgChat != NULL )
	{
		logMes_3("slot_onChangeTextChat: Add New Message to Chat");
		if ( dlgChat->uin == jid )
		{
			dlgChat->addMes( mes );
			logMes_3("slot_onChangeTextChat: Added mes");
			return;
		}	
	}
}
#endif

void ZGui::slot_onAuthReply(string from, string text, uint8_t aflag)
{
	logMes_3("slot_onAuthReply: run");
	int uin_ind = icq->findContact(from);

	newMes( QString::fromLocal8Bit( from.c_str() ),
		strtoqstr( icq->getNick(uin_ind) ),
		QString::fromLocal8Bit( text.c_str() ),
		QDateTime::currentDateTime(), ((aflag==AUTH_DECLINED) ? TYPE_AUTH_DECLINED : TYPE_AUTH_ACCEPTED) );

	alert();
}

void ZGui::slot_onAuthRequest(string from, string text)
{
	logMes_3("slot_onAuthRequest: run");

	int uin_ind = icq->findCLUIN(from);
	QString qtext;
	if (uin_ind<0) 
	{
		qtext = LNG_AUTHREQUEST1 + QString::fromLocal8Bit( from.c_str() ) +"\n"+ 
 			strtoqstr( text.c_str() ) + LNG_AUTHREQUEST2;
 	} else
 	{
		qtext =  LNG_AUTHREQUEST1 + strtoqstr( icq->getNick(uin_ind).c_str() )+ 
		" ("+ QString::fromLocal8Bit( from.c_str() )+")\n"+ strtoqstr( text.c_str() ) + LNG_AUTHREQUEST2;
	}

	newMes( QString::fromLocal8Bit( from.c_str() ),
		QString::fromLocal8Bit( from.c_str() ),
		qtext,
		QDateTime::currentDateTime(), TYPE_AUTH_REQUEST);

	uin_ind = icq->findContact( from );	
	icq->setAskAuth(uin_ind, true);

	if ( dlgChat && dlgChat->uin == from )		
		dlgChat->buildMenu();

	alert();

	updateMyIcon();
}

void ZGui::slot_onContactListChanged(void)
{
	clearList();
	printContact();
	#ifdef _XMPP
	printContactXMPP();
	#endif
}

void ZGui::slot_onSingOff(uint16_t err_code, string err_url)
{
	logMes_1("lot_onSingOff: run");
	icq->connected = false;

	onConnectChange(PROT_ICQ, false);

	QString error = LNG_ERROR+" ";
	switch(err_code)
	{
		case SIGNOFF_OTHER_LOCATION :
			error = error+LNG_UINUSES;
			break;
		default:
			error = error+LNG_ERRCODE+QString::number(err_code);
	}
	if ( err_url.c_str() != "" )
	{
		error = error+"\n"+LNG_URLDESCRIPTION+"\n"+QString::fromLocal8Bit(err_url.c_str());
	}

	showMesDlg(LNG_ERRORCONNECT, error, 1);
}

void ZGui::slot_onWasAdded(string from)
{	
	int uin_ind = icq->findContact(from);

	newMes( QString::fromLocal8Bit(from.c_str() ),
		QString::fromLocal8Bit(icq->getNick(uin_ind).c_str() ),
		LNG_YOUWASADDED, QDateTime::currentDateTime(), TYPE_WAY_ADDED);

	alert();
}

void ZGui::slot_onXstatusChanged(string uin, size_t x_status, string x_title, string x_descr)
{
	if (dlgStat != NULL)
	{
		logMes_1("Add xStatus to Dlg xSatatus Viwe");
		if ( uin == dlgStat->id )
		{
			dlgStat->setTitle( strtoqstr( x_title.c_str() ) );
			dlgStat->setDesc( strtoqstr( x_descr.c_str() ) );
		}
	}
	
	ZContactItem * contact = getICQContact(uin);
	if (contact == NULL)
		return;
	contact->setStatusX(x_status);
}

QString ZGui::getProgramDir()
{
	ProgDir = QString ( qApp->argv() [0] ) ;
	int i = ProgDir.findRev ( "/" );
	ProgDir.remove ( i+1, ProgDir.length() - i );
	return ProgDir;
}

void ZGui::updateStatus()
{
	setProc(icq->getConnectPercentage());
}

void ZGui::cancellICQ()
{
	logMes("cancellICQ: run");
	icq->breakNetworkOperation();
}

void ZGui::menu_profile_del()
{
	if ( lbContact->currentItem() < 0 )
		return;	
	
	ZContactItem* listitem = lbContact->item ( lbContact->currentItem() );

	int n = listitem->getReservedData();
	
	ZConfig cfg(ProgDir+"/zMessanger.cfg");

	if ( listitem->getProtocol() == PROT_ICQ )
	{
		cfg.writeEntry(QString("Login"), QString("UIN")+QString::number(n), "");
		cfg.writeEntry(QString("Login"), QString("PAS")+QString::number(n), "");
	}
	#ifdef _XMPP
	if ( listitem->getProtocol() == PROT_JABBER )
	{
		cfg.writeEntry(QString("Login"), QString("JID")+QString::number(n), "");
		cfg.writeEntry(QString("Login"), QString("JPAS")+QString::number(n), "");
	}
	#endif
	
	cfg.flush();
	
	lbContact->takeItem( listitem );
}

void ZGui::menu_profile_add()
{
	qApp->removeEventFilter( this );
	
	ZAddProfile* dlgAddProfile = new ZAddProfile( );
	dlgAddProfile->exec();
	if (dlgAddProfile->result() == 1)
	{
		ZContactItem* listitem = new ZContactItem(lbContact, ITEM_PROFILE );
		
		listitem->setNick( dlgAddProfile->getNewId() );
		int id = dlgAddProfile->getNewNum();
		#ifdef _XMPP
		if ( dlgAddProfile->getNewProt() == 0 )
		#endif
			listitem->setProtocol( PROT_ICQ );
		#ifdef _XMPP
		else 
			listitem->setProtocol( PROT_JABBER );
		#endif
		
		listitem->setReservedData ( id );
		lbContact->insertItemInList( listitem,-1,true );
	}
	
	qApp->installEventFilter( this );
}

void ZGui::menu_profile_setdef()
{
	if ( lbContact->currentItem() < 0 )
		return;	
	
	ZContactItem* listitem = lbContact->item ( lbContact->currentItem() );

	ZConfig cfg(ProgDir+"/zMessanger.cfg");
	
	if ( listitem->getProtocol() == PROT_ICQ )
		cfg.writeEntry(QString("Login"), QString("DefaultICQ"), listitem->getReservedData());
	#ifdef _XMPP
	if ( listitem->getProtocol() == PROT_JABBER )
		cfg.writeEntry(QString("Login"), QString("DefaultJ"), listitem->getReservedData());
	#endif
	
	cfg.flush();

	for (int n=0; n<lbContact->count(); n++ )
	{
		if ( lbContact->itemChecked(n) )
		{
			ZContactItem* listitem2 = lbContact->item( n );
			if ( ( listitem2->getProtocol() == PROT_ICQ && listitem->getProtocol() == PROT_ICQ )
				#ifdef _XMPP
				||(listitem2->getProtocol() == PROT_JABBER && listitem->getProtocol() == PROT_JABBER ) 
				#endif
				)
				lbContact->checkItem(n, false);
		}
	}
	
	lbContact->checkItem(listitem, true);
}

void ZGui::menu_profile_change()
{
	if ( lbContact->currentItem() < 0 )
		return;	
		
	qApp->removeEventFilter( this );
	
	ZContactItem* listitem = lbContact->item ( lbContact->currentItem() );
	
	int data = listitem->getReservedData();
	
	ZConfig cfg(ProgDir+"/zMessanger.cfg");
	
	int protInt=-1;
	
	QString ID;
	QString PAS;
	
	if ( listitem->getProtocol() == PROT_ICQ )
	{
		ID = cfg.readEntry(QString("Login"), QString("UIN")+QString::number(data), "");
		PAS = cfg.readEntry(QString("Login"), QString("PAS")+QString::number(data), "");
		protInt = 0;
	}
	#ifdef _XMPP
	if ( listitem->getProtocol() == PROT_JABBER )
	{
		ID = cfg.readEntry(QString("Login"), QString("JID")+QString::number(data), "");
		PAS = cfg.readEntry(QString("Login"), QString("JPAS")+QString::number(data), "");
		protInt = 1;
	}
	#endif
	
	ZAddProfile* dlgAddProfile = new ZAddProfile();
	dlgAddProfile->setEditProfile(ID, PAS, data, protInt);

	dlgAddProfile->exec();
	if (dlgAddProfile->result() == 1)
		listitem->setNick(dlgAddProfile->getNewId() );
		
	qApp->installEventFilter( this );
}

void ZGui::menu_showXStatus()
{
	ZContactItem* listitem = lbContact->item ( lbContact->currentItem() );
	if ( listitem->getProtocol() != PROT_ICQ && !listitem->isGroup() )
		return;
	
	qApp->removeEventFilter( this );
	dlgStat  = new ZStatusInfo( listitem->getUID(), listitem->getProtocol() );
	dlgStat->exec();
	delete dlgStat;
	dlgStat = NULL;
	qApp->installEventFilter( this );
}

void ZGui::menu_userInfo()
{
	ZContactItem* listitem = lbContact->item ( lbContact->currentItem() );
	if ( listitem->getProtocol() != PROT_ICQ && !listitem->isGroup() )
		return;

	qApp->removeEventFilter( this );
	ZUserInfo * dlgUserInfo  = new ZUserInfo( listitem->getUID() );
	dlgUserInfo->exec();
	delete dlgUserInfo;	
	qApp->installEventFilter( this );
}

void ZGui::slotFixMenuBag()
{
	softKey->eventFilter( myWidget, &QKeyEvent(QEvent::KeyPress,Z6KEY_LSK,0,1));
	softKey->eventFilter( myWidget, &QKeyEvent(QEvent::KeyRelease,Z6KEY_LSK,0,0));
}

void ZGui::menu_connect()
{	
	logMes_2("menu_connect");
	
	#ifdef _XMPP
	xmppSet = false;
	#endif
	icqSet = false;

	if ( !icq->connected 
	#ifdef _XMPP
		&& !xmpp->connected 
	#endif
					)
	{
		logMes_2("menu_connect 2");
		
		ZConfig cfg(ProgDir+"/zMessanger.cfg");

		bool inetConnec = false;
		if (!cfg_InetOnUSB)
		{
			if ( napi_link == 0)
			{
				inetConnec = true;
				link = napi->openLink( cfg_inetProfile );
				napi_link = link;
			}
		} else
		{
			if ((_res.options & RES_INIT) == 0 && res_init() == -1) 
			{
				perror("NET USB: res_init error");
			}
			struct sockaddr_in *paddr = (struct sockaddr_in*) _res.nsaddr_list;
			paddr->sin_family = AF_INET;
			paddr->sin_port = htons(53);
			paddr->sin_addr.s_addr = inet_addr("192.168.16.1");
			system("route del default; route add default gw 192.168.16.1");
		}

		logMes_2("menu_connect 3");

		string MyUin;
		string MyPassword;
		QString MyJID;
		QString MYJPassword;
		int id;
		ZContactItem* listitem;

		for (int i=0;i<lbContact->count();i++)
			if ( lbContact->itemChecked(i) )
			{
				logMes_2("menu_connect 4");
				listitem = lbContact->item( i );
				
				id  = listitem->getReservedData();
				if ( !icqSet && listitem->getProtocol() == PROT_ICQ )
				{
					logMes_2("menu_connect 5");
					
					MyUin = cfg.readEntry(QString("Login"), QString("UIN")+QString::number(id), "123").latin1() ;
					MyPassword = cfg.readEntry(QString("Login"), 	QString("PAS")+QString::number(id), "123").latin1();
					icq->setUIN(MyUin);
					icq->setPassword(MyPassword);
					icqSet = true;
					#ifndef _XMPP
					break;
					#endif
				}
				#ifdef _XMPP
				if ( !xmppSet && listitem->getProtocol() == PROT_JABBER )
				{
					MyJID = cfg.readEntry(QString("Login"), QString("JID")+QString::number(id), "123");
					MYJPassword = cfg.readEntry(QString("Login"), 	QString("JPAS")+QString::number(id), "123");
					xmpp->setProfile(MyJID , MYJPassword );
					xmppSet = true;
				}
				if ( xmppSet && icqSet )
					break;
				#endif
			}
		cfg.flush();

		#ifdef _XMPP
		if ( !xmppSet && !icqSet )
		#else
		if ( !icqSet )
		#endif	
			return;

		showProc(LNG_CONNECTTTT);
		
		timer = NULL;
		
		timer = new QTimer();
		if ( icqSet )
		{
			connect( timer, SIGNAL(timeout()), SLOT( updateStatus() ) );
			connect( pProgressDialog, SIGNAL(cancelled()), SLOT( cancellICQ() ) );
			timer->start( 1000, false );
		}

		if ( !inetConnec )
			slot_internetConnected();	
	} else
	{
		if ( icq->connected )
		{
			icq->Disconnect();
		}
		#ifdef _XMPP
		if ( xmpp->connected )
		{
			xmpp->Disconnect();
		}
		#endif
	}
}

void ZGui::clearList()
{
	lbContact->clear();
	
	ZContactItem* listitem = new ZContactItem( lbContact, ITEM_SPLITER );
	listitem->setProtocol(PROT_SPLIT1);
	QPixmap pm;
	pm.load( ProgDir + QString ( "/split/icq.png" ) );
	listitem->setPixmap ( 0, pm );
	lbContact->protAdd ( listitem );

	#ifdef _XMPP
	listitem = new ZContactItem( lbContact, ITEM_SPLITER );
	listitem->setProtocol(PROT_SPLIT2);
	pm.load( ProgDir + QString ( "/split/jabber.png" ) );
	listitem->setPixmap ( 0, pm );
	lbContact->protAdd ( listitem );
	#endif
}

void ZGui::slot_internetConnected()
{	
	clearList();
	#ifdef _XMPP
	if ( xmppSet )
		xmpp->Connect();
	#endif
	if ( icqSet )
	{
		struct hostent * he;
 		if ((he=gethostbyname( icq->getHost().c_str() ))!=NULL)
		{
			if (he->h_addr!=NULL)
			{	
				struct   sockaddr_in   adr;
				memcpy(&adr.sin_addr,he->h_addr_list[0],he->h_length);
				logMes("ICQ ip: " + QString(inet_ntoa(adr.sin_addr)));
				QString host = QString(inet_ntoa(adr.sin_addr));
				icq->setHost(host.latin1());
			}
		}
		icq->startConnect();
	}
	
}

void ZGui::addContactToList(QString uin)
{
	ICQKidShortUserInfo uinfo;
	
	if (icq->getUserInfo(uin.latin1(), uinfo)) 
	{
		QString strGender;
    	switch (uinfo.Gender)
    	{
			case 0: strGender=""; break;
			case 1: strGender=LNG_GANDER1; break;
			case 2: strGender=LNG_GANDER2; break;
    	}
    	
    	ZMessageDlg* dlg = new ZMessageDlg( LNG_ADDCONTACT_TITLE,"UIN:" + strtoqstr(uinfo.Uin) + "\n"+LNG_NICK + strtoqstr(uinfo.Nickname) + "\n"+LNG_NAME + strtoqstr(uinfo.Firstname) + "\n"+LNG_LASTNAME+strtoqstr(uinfo.Lastname)+"\n"+LNG_GANDER+strGender, ZMessageDlg::TypeChoose, 0, myWidget);
    	dlg->setMsgIcon(ICON_DLG_QUESTION);
    	
		if ( dlg->exec() )
		{
			ZSingleSelectDlg * selgroup = new ZSingleSelectDlg(LNG_SELECTGROUP, LNG_ADDTOGROUP, myWidget);
			for (int i=0; i<icq->getCountGroup(); ++i)
			{
				QStringList item;
				item.append(strtoqstr( icq->getGroupName(i).c_str() ));
				selgroup->addItemsList(item);
			}
			if ( selgroup->exec() > 0 )
			{
				if ( strtoqstr(uinfo.Nickname) != "" )
				{
					icq->addContact(uin.latin1(), uinfo.Nickname,icq->getGroupName(selgroup->getCheckedItemIndex()));
				} else
				{
					icq->addContact(uin.latin1(), uin.latin1(),icq->getGroupName(selgroup->getCheckedItemIndex()) );
				}
			}
			delete selgroup;
			selgroup = NULL;	
		}
		delete dlg;
		dlg = NULL;
	}
}

void ZGui::menu_addContact()
{
	qApp->removeEventFilter( this );
	
	ZSingleCaptureDlg* zscd = new ZSingleCaptureDlg(LNG_ADDCONTACT, "UIN:", ZSingleCaptureDlg::TypeLineEdit, myWidget, "", true, 0, 0);
	ZLineEdit* zle = (ZLineEdit*)zscd->getLineEdit();

	QValueList<QUuid> flist1;
	flist1.append( ZKB_INPUT_NUMERIC );
	setInputMethods((QWidget*)zle, ZKB_INPUT_NUMERIC, flist1);
	
	if ( zscd->exec() == 1 )
	{
		QString uin = zle->text().stripWhiteSpace();
		addContactToList(uin);
	}
	delete zle;
	delete zscd;
	
	qApp->installEventFilter( this );
}

void ZGui::menu_removeContact()
{
	ZContactItem* listitem = lbContact->item ( lbContact->currentItem() );
	
	if ( listitem->isGroup() || listitem->getProtocol() != PROT_ICQ || listitem->isNoCL() )
		return;
	
	qApp->removeEventFilter( this );
	
	string suin;
	QString nick;

	suin = listitem->getUID();
	int n = icq->findContact(suin);
	nick = strtoqstr( icq->getNick(n).c_str() );

	ZMessageDlg * dlg = new ZMessageDlg(LNG_REMOVECONTACT_QUESTION, "UIN:"+strtoqstr(suin)+"\n"+LNG_NICK+nick, ZMessageDlg::TypeChoose, 0, myWidget);
	dlg->setMsgIcon(ICON_DLG_QUESTION);
	
	if ( dlg->exec() )
	{
		icq->removeContact(suin);
	}
	
	delete dlg;
	dlg = NULL;
	
	qApp->installEventFilter( this );	
}

void ZGui::menu_addGroup()
{
	qApp->removeEventFilter( this );
	
	ZSingleCaptureDlg* zscd = new ZSingleCaptureDlg(LNG_ADDGRPUP, LNG_GROUPNAME, ZSingleCaptureDlg::TypeLineEdit, myWidget, "", true, 0, 0);
	ZLineEdit* zle = (ZLineEdit*)zscd->getLineEdit();

	if (zscd->exec() == 1)
	{
		QString nameGroup = zle->text().stripWhiteSpace();
		icq->addGroup(nameGroup.utf8().data());	
	}
	
	delete zle;
	delete zscd;
	
	qApp->installEventFilter( this );	
}

void ZGui::menu_removeGroup()
{
	ZContactItem* listitem = lbContact->item ( lbContact->currentItem() );
	
	if ( !listitem->isGroup()  || listitem->isNoCL() || listitem->getProtocol() != PROT_ICQ )
		return;

	qApp->removeEventFilter( this );

	ZMessageDlg * dlg = new ZMessageDlg(LNG_REMOVEGROUP_QUESTION,LNG_GROUPNAME+strtoqstr( icq->getGroupName( listitem->getReservedData() ).c_str() ), ZMessageDlg::TypeChoose, 0, myWidget);
	dlg->setMsgIcon(ICON_DLG_QUESTION);

	if ( dlg->exec() )
	{
		icq->removeGroup( icq->getGroupName( listitem->getReservedData() ) );
	}
	
	delete dlg;
	dlg = NULL;
	
	qApp->installEventFilter( this );
}

void ZGui::menu_showChat()
{
	lbContactSel(lbContact->currentItem());
}

void ZGui::menu_settings()
{
	qApp->removeEventFilter( this );
	ZSettingsDlg * dlgSettings  = new ZSettingsDlg();
	dlgSettings->exec();
	delete dlgSettings;
	qApp->installEventFilter( this );
}

void ZGui::menu_copyUin()
{
	ZContactItem* listitem = lbContact->item ( lbContact->currentItem() );

	QClipboard *cb = QApplication::clipboard();
	cb->setText( strtoqstr( listitem->getUID() ) );
}

void ZGui::menu_about()
{
	qApp->removeEventFilter( this );	
	
	ZAboutDialog * dlgAbout  = new ZAboutDialog( );
	dlgAbout->exec();
	delete dlgAbout;
	dlgAbout = NULL;
	
	qApp->installEventFilter( this );
}

void ZGui::menu_minimize()
{
	slot_ReturnToIdle(0);
}

void ZGui::menu_setXStatus()
{
	qApp->removeEventFilter( this );
	
	ZImgeSelect * dlgXStatus  = new ZImgeSelect(false);
	dlgXStatus->exec();
	delete dlgXStatus;
	
	QPixmap * pm = new QPixmap(); 
	if ( icq->getMyXStatus() == X_STATUS_NONE )
		pm->load(ProgDir+ "/menu/xStatus.png");	
	else
		pm->load( ProgDir + QString ( "/status/icq/x/icq_xstatus" ) + QString::number(icq->getMyXStatus()-1) + ".png");
	menu->changeItem( MENU_MY_XSTATUS_ID, LNG_XSTATUS, pm );
	
	qApp->installEventFilter( this );
}

void ZGui::slot_Raise()
{
	this->show();
	#ifdef _XMPP
	if ( (icq->connected || xmpp->connected ) && !isShown )
	#else
	if ( icq->connected && !isShown )
	#endif
	{
		clearList();
		if ( icq->connected )
			printContact(false);
		#ifdef _XMPP
		if ( xmpp->connected )
			xmppPrintContact(false);
		#endif
	}
	isShown = true;
	activSlot(true);
}

void ZGui::slot_ReturnToIdle( int )
{
	isShown = false;
	if (icq->connected)
		lbContact->clear();
	this->hide();
	activSlot(false);
}

int ZGui::icqAddGroup( QString Name, int groupId )
{
	QPixmap pm;
	pm.load( ProgDir + "/CL/group.png" );

	ZContactItem* listitem = new ZContactItem ( lbContact, ITEM_GROUP );
	listitem->setGroup( true );
	listitem->setNick( Name.simplifyWhiteSpace() );
	listitem->setGroupId( groupId );

	lbContact->groupAdd(listitem, groupId );
	return -1;
}

#ifdef _XMPP
int ZGui::xmppAddGroup( QString Name, int groupId )
{
	QPixmap pm;
	pm.load( ProgDir + "/CL/group.png" );

	ZContactItem* listitem = new ZContactItem ( lbContact, ITEM_GROUP );
	
	listitem->setGroup( true );
	listitem->setProtocol(PROT_JABBER);
	listitem->appendSubItem ( 1, Name.simplifyWhiteSpace() , true );
	listitem->setGroupId( groupId );

	lbContact->groupAdd(listitem, groupId );
	return -1;
}
#endif

int ZGui::icqAddUser(QString Name, string uin, int Status, int xStatus, int clientId, int id, int groupId, bool auth, bool mes, bool invisible)
{
	logMes_2("icqAddUser: "+Name);
	QString ico;
	QPixmap pm;
	
	ZContactItem* listitem = new ZContactItem ( lbContact, ITEM_CONACT );
	
	listitem->setNick( Name.simplifyWhiteSpace() );	
	listitem->setUID(uin);
	listitem->setStatus(invisible?STATUS_INVISIBLE:Status, true);
	listitem->setStatusX(xStatus);
	listitem->setClient(clientId);
	listitem->setReservedData(id);
	listitem->setGroupId(groupId);
	listitem->setNoCL(id>=1000);
	if ( auth )
		listitem->setWaitAuth( auth );
	if ( mes )
		listitem->setNewMes( mes );		

	lbContact->contactAdd( listitem );
	return -1;
}

#ifdef _XMPP
int ZGui::xmppAddUser(string name, int Status, int id, int clientId, int groupId, bool auth, bool mes)
{
	logMes_2("xmppAddUser: "+strtoqstr(name));
	QString ico;
	QPixmap pm;
	
	ZContactItem* listitem = new ZContactItem ( lbContact, ITEM_CONACT );
	listitem->setNick( QString::fromUtf8( name.c_str() ).simplifyWhiteSpace() );	
	listitem->setProtocol( PROT_JABBER );
	listitem->setUID( name );
	listitem->setStatus(Status);
	listitem->setClient(clientId);
	listitem->setGroupId(groupId);
	if ( auth )
		listitem->setWaitAuth( auth );
	if ( mes )
		listitem->setNewMes( mes );		

	lbContact->contactAdd( listitem );
	return -1;
}

void ZGui::xmppUpdateStatus(string jid, int contactId, int Status, int clientId, bool mes, bool typing, bool ignorAll)
{
	// FIX ME
}

ZContactItem * ZGui::getXMPPContact(string jid)
{
	logMes_2("getXMPPContact: "+strtoqstr(jid));
	if ( !isShown ) // if minimizing - dont update CL
	{
		return NULL;
	}
		
	ZContactItem * listitem = lbContact->getConact(jid);
	if ( listitem == NULL )
	{
		int n = xmpp->findContact(jid);
		if ( n > -1 ) // Contact in CL
		{
			n = xmppAddUser(xmpp->getNick(n), xmpp->getStatus(n), n, xmpp->getClientId(n) , xmpp->getGroupId(n), false, xmpp->isMesIcon(n));
		}
		ZContactItem * listitem = lbContact->getConact(jid);
		if ( listitem == NULL )
			return NULL;	
	}
	return listitem;
}

#endif

ZContactItem * ZGui::getICQContact(string uin)
{
	logMes_3("getICQContact: " + strtoqstr(uin) );
	if ( !isShown ) // if minimizing - dont update CL
	{
		return NULL;
	}
		
	ZContactItem * listitem = lbContact->getConact(uin);
	if ( listitem == NULL )
	{
		int n = icq->findContact(uin);
		if ( n > -1 )
		{
			icqAddUser(strtoqstr( icq->getNick(n).c_str() ), uin, icq->getStatus(n), icq->getXStatus(n), icq->getClientId(n), n, icq->getGroupId(n), icq->isWaitAuth(n), icq->isMesIcon(n), false);
		} else
		{
			logMes_3("getICQContact: new contact");
			n = icq->addContactToNoCLList(QString(uin.c_str()),QString(uin.c_str()));
			n = icqAddUser(strtoqstr( icq->getNick(n).c_str() ), uin, icq->getStatus(n), icq->getXStatus(n),	icq->getClientId(n), n, icq->getGroupId(n), icq->isWaitAuth(n), icq->isMesIcon(n), false);
		}
		ZContactItem * listitem = lbContact->getConact(uin);
		if ( listitem == NULL )
			return NULL;	
	}
	return listitem;
}

void ZGui::printContact(bool Clear)
{
	logMes_3("printContact: start");
	
	if (Clear)
		lbContact->dellAllContactWithProtocol(PROT_ICQ);

	logMes_1("Group in list: "+QString::number(icq->getCountGroup()));
	logMes_1("Contact in list: "+QString::number(icq->getCountCL()));

	for (int i=0; i<icq->getCountGroup(); ++i)
		icqAddGroup( strtoqstr( icq->getGroupName(i).c_str() ), icq->getGroupItemId(i) );
	for (int j=0; j<icq->getCountCL(); ++j)
		if ( !cfg_dontShowOffLine || (cfg_dontShowOffLine &&  icq->getStatus(j) != STATUS_OFFLINE) || icq->isMesIcon(j) )
			icqAddUser(strtoqstr( icq->getNick(j).c_str() ), icq->getUIN(j), icq->getStatus(j), icq->getXStatus(j), icq->getClientId(j), j, icq->getGroupId(j),  icq->isWaitAuth(j), icq->isMesIcon(j) );


	// Insert split
	QPixmap pm;
	pm.load( ProgDir + "/CL/group.png" );

	ZContactItem* listitem = new ZContactItem ( lbContact, ITEM_GROUP );
	
	listitem->setGroup( true );
	listitem->setNick ( "No in CL" );
	listitem->setGroupId( ICQ_NOT_IN_LIST_GROUP );
	listitem->setNoCL( true );
	
	lbContact->groupAdd(listitem, ICQ_NOT_IN_LIST_GROUP );
	
	//Add no list contact
	for (uint j=1000; j<1000+icq->NoContactListUins.size(); ++j)
		icqAddUser(strtoqstr( icq->getNick(j).c_str() ), icq->getUIN(j), icq->getStatus(j), icq->getXStatus(j), icq->getClientId(j), j, icq->getGroupId(j),  icq->isWaitAuth(j), icq->isMesIcon(j));

	lbContact->UpdateList();
}

#ifdef _XMPP
void ZGui::xmppPrintContact(bool Clear)
{
	logMes_2("xmppPrintContact: run");

	if (Clear)
		lbContact->dellAllContactWithProtocol(PROT_JABBER);

	logMes_3("Group in list: "+QString::number(xmpp->getGroupCount());
	logMes_3("Group in list: "+QString::number(xmpp->getContactCount());
	
	for (int i=0; i<xmpp->getGroupCount(); ++i)
	{
		if ( i < xmpp->getGroupCount()-1 )
		{
			xmppAddGroup(strtoqstr( xmpp->getGroupName(i).c_str() ), i);
			for (int j=0; j<xmpp->getContactCount(); ++j)
				if ( xmpp->getContactGroup(j) == xmpp->getGroupName(i) )
				{
					if ( !cfg_dontShowOffLine || (cfg_dontShowOffLine &&  xmpp->getStatus(j) != XMPP_OFFLINE) || xmpp->isMesIcon(j) )
						xmppAddUser( xmpp->getNick(j), xmpp->getStatus(j), j, xmpp->getClientId(j), -1, xmpp->isWaitAuth(j), xmpp->isMesIcon(j), true);
				}
		} else
		{
			/* FIX ME: NOT ADD CONFERENCE 
			logMes_2("Group: %s", xmpp->getGroupName(i).c_str() );
			xmppAddGroup(strtoqstr( xmpp->getGroupName(i).c_str() ), i);
			for (int j=0; j<xmpp->getConferenceCount(); j++)
			{
				t = j;
				SET_JABBER(t);
				SET_CONFERENT(t);
				xmppAddUser( strtoqstr( xmpp->getConferenceJID(t).c_str() ), XMPP_ONLINE, t, 0, -1, false, false, true);
			}
			*/
		}
  	}
}
#endif

void ZGui::setProc(int i)
{
	pProgressDialog->setProgress(i);
	qApp->processEvents();	
}

void ZGui::setOperat(QString text, int max)
{
	pProgressDialog->setProgress(0);	
	pProgressDialog->setInstructText(text);
	if (max>0)
		pProgressDialog->setProgressTotalStep(max);
	qApp->processEvents();	
}

void ZGui::showProc(QString text)
{
	pProgressDialog = new ZProgressDlg("zMessanger", text, 100, 100, this);
	pProgressDialog->setProgress(0);
	pProgressDialog->raise();
	pProgressDialog->show();
	qApp->processEvents();
}

void ZGui::closeProc()
{
	if (pProgressDialog == NULL )
		return;
	if ( timer != NULL)
	{
		timer->stop();
		delete timer;
		timer = NULL;
	}
	pProgressDialog->close();
	delete pProgressDialog;
	pProgressDialog = NULL;
}

bool ZGui::showMesDlg(QString title, QString mes, int ico)
{
	QByteArray data;
	QDataStream s( data, IO_WriteOnly ); 
	s << title;
	s << mes;
	s << ico;
	return QCopChannel::send( ZMESSANGER_CHENEL, "showError(str,str,int)", data );
}

void ZGui::slot_onErrorConnectICQ(QString mes)
{
	showMesDlg(LNG_ERRORCONNECT, mes, 1);
	onConnectChange(PROT_ICQ, false);
}

#ifdef _XMPP
void ZGui::slot_onErrorConnectXMPP(QString mes)
{
	showMesDlg(LNG_ERRORCONNECT, mes, 1);
	onConnectChange(PROT_JABBER, false);
}
#endif

void ZGui::slot_onClientChange( string uin, size_t clientId )
{
	ZContactItem * contact = getICQContact(uin);
	if (contact == NULL)
		return;
	contact->setClient(clientId);
	
	if (dlgChat != NULL)
	{
		if ( dlgChat->uin == uin )
		{
			dlgChat->setClient( icq->getClientName(clientId) );
		}
	}
}

void ZGui::newMes(QString uin, QString title, QString mes, QDateTime time, int type)
{
	logMes_2("newMes: "+uin);
	messageList[uin.toUInt()].append( BDMes(title, mes, time, type) );
	if ( dlgChat && dlgChat->uin == uin.latin1() )
	{
		logMes_2("newMes: Add New Message to Chat");		
		dlgChat->addMes( bdMesToText( BDMes(title, mes, time, type )) );
		logMes_2("newMes: Added mes");
		return;
	}

	trayMes = true;
	ZHeader::changeStatus(ZHeader::IM, 4);

	if ( !isShown )
		return;

	ZContactItem * contact = getICQContact(uin.latin1());
	if (contact == NULL)
		return;
	contact->setNewMes(true);
}

#ifdef _XMPP
void ZGui::xmppNewMes(string jid, QString title, QString mes, QDateTime time, int type)
{
	logMes_2("xmppNewMes: %s", jid.c_str());
	if ( type == TYPE_AUTH_REQUEST )
	{
		mes = LNG_AUTHREQUEST1+title+LNG_AUTHREQUEST2+"\n"+mes;
	}
	xmpp->messageList[jid].append( BDMes(title, mes, time, type) );
	if ( dlgChat != NULL )
	{
		logMes_2("xmppNewMes: Add New Message to Chat");
		if ( dlgChat->uin == jid )
		{
			dlgChat->addMes( bdMesToText( BDMes(title, mes, time, type )) );
			logMes_2("xmppNewMes: Added mes");
			return;
		}
	}

	trayMes = true;
	ZHeader::changeStatus(ZHeader::IM, 4);

	if ( !isShown )
		return;	

	ZContactItem * contact = getXMPPContact(jid);
	if (contact == NULL)
		return;
	contact->setNewMes(true);
}
#endif

void ZGui::openChatTest()
{
	qApp->removeEventFilter( this );
	zChatForm *chat  = new zChatForm("","", -1);
	chat->exec();
	delete chat;
	chat = NULL;
	qApp->installEventFilter( this );
}

QString ZGui::bdMesToText(BDMes mesFild, bool forHistory)
{
	QString mes = "";
	if (!forHistory)
	{
		switch ( mesFild.type() )
		{
			case TYPE_MESSAGE:
			case TYPE_MY_MESSAGE:
				mes = "[*MES*]";
				break;
			case TYPE_WAY_ADDED:
				mes = "[*WADD*]";
				break;
			case TYPE_AUTH_REQUEST:
				mes = "[*WADD*]";
				break;
			case TYPE_AUTH_DECLINED:
				mes = "[*AUTH*]";
				break;
			case TYPE_AUTH_ACCEPTED:
				mes = "[*AUTH*]";
				break;
			default:
				mes = "";
		}
	}

	if (!forHistory)
	{	
		if ( mesFild.type() == TYPE_MY_MESSAGE)
		{
			return mes+" $3%$"+mesFild.title()+" ["+mesFild.time().time().toString()+"]#\n"+mesFild.mes();//+"\n"	
		} else
		{
			return mes+" $1%$"+mesFild.title()+" ["+mesFild.time().time().toString()+"]#\n"+mesFild.mes();//+"\n"				
		}
	} else
	{
		return mes+mesFild.title()+" ["+mesFild.time().toString()+"]\n"+mesFild.mes();//+"\n"	
	}
}

void ZGui::updateMyIcon()
{
	if (trayMes)
	{
		#ifdef _XMPP
		int status;
		if (icq->connected)
		{
			status = icq->getMyOnlineStatus();
		} else
		if (xmpp->connected) 
		{
			status = 1;
		}
		switch( status )
		#else
		switch( icq->getMyOnlineStatus() )
		#endif
		{
			case 0:
				ZHeader::changeStatus(ZHeader::IM, 1); break;
			case 2:
				ZHeader::changeStatus(ZHeader::IM, 3); break;
			case 5:
				ZHeader::changeStatus(ZHeader::IM, 3); break;
			case 3:
				ZHeader::changeStatus(ZHeader::IM, 3); break;
			case 4:
				ZHeader::changeStatus(ZHeader::IM, 3); break;
			case 1:
				ZHeader::changeStatus(ZHeader::IM, 1); break;
			case 7:
				ZHeader::changeStatus(ZHeader::IM, 1); break;
			case 8:
				ZHeader::changeStatus(ZHeader::IM, 1); break;
			case 9:
				ZHeader::changeStatus(ZHeader::IM, 1); break;
			case 10:
				ZHeader::changeStatus(ZHeader::IM, 1); break;
			case 11: 
				ZHeader::changeStatus(ZHeader::IM, 1); break;
			case 6:
				ZHeader::changeStatus(ZHeader::IM, 2);
		}
		trayMes = false;
	}
}

void ZGui::openChat(ZContactItem* listitem)
{
	if (showedChat)
		return;	

	logMes_2("openChat: start");

	showedChat = true;

	mesList messanges;
	QString textMes = "";

	if ( listitem->getProtocol() == PROT_ICQ )
	{
		messanges = messageList[QString( listitem->getUID().c_str() ).toUInt()];
		icq->setMesIcon(listitem->getReservedData(), false);
	} else
	#ifdef _XMPP
	if ( listitem->getProtocol() == PROT_JABBER )
	{
		if ( listitem->isConference() )
		{
			textMes = "Need code fix";//xmpp->getConferenceText(data);
			connect( xmpp->zXMPPConf, SIGNAL(onChangeTextChat(std::string, QString)), this, SLOT(slot_onChangeTextChat(std::string, QString)) );
		} else
		{
			ZContactItem * contact = getXMPPContact(listitem->getUID());
			if (contact == NULL)
				return;
			
			contact->setNewMes(false);	
					
			updateMyIcon();
			
			messanges = xmpp->messageList[listitem->getUID()];
		}

	} else
	#endif
	{
		return;
	}
	
	listitem->setNewMes( false );
	listitem->setType( false );

	updateMyIcon();

	logMes_3("openChat: create chat dlg");

	qApp->removeEventFilter( this );

	dlgChat = new zChatForm(listitem->getUID(), listitem->getNick(), listitem->getProtocol());
	
	logMes_3("openChat: Add message to chat");

	dlgChat->eeChat->setText("");
	uint n;

	logMes_3("openChat: Print mes");
	if (textMes == "") 
	{
		for ( n = max( (int)0, (int)messanges.count() - 10); n < messanges.count(); n++ )
			dlgChat->eeChat->addText( bdMesToText( ( *messanges.at(n) ) ) );
	} else
	{
		dlgChat->eeChat->setText( textMes );
	}

	logMes_3("openChat: End add mes");

	dlgChat->eeChat->toEnd();

	#ifdef _XMPP
	if ( listitem->getProtocol() == PROT_ICQ )
	{
	#endif	
		if ( listitem->intStat() != 13 /*OFFLINE*/)
		{
			dlgChat->setClient( icq->getClientName( listitem->getClient() ) );
		} else
		{
			dlgChat->setContOff();		
		}
	#ifdef _XMPP	
	} 
	else if ( listitem->getProtocol() == PROT_JABBER )
	{
		if ( listitem->isConference() )
		{
			//Not info
		} else
		{
			int id = xmpp->findContact(listitem->getUID());
			if ( xmpp->getStatus(id) != XMPP_OFFLINE)
			{
				dlgChat->setClient( xmpp->getClientNeme(xmpp->getClientId(id)) );
			} else
			{
				dlgChat->setContOff();
			}
		}
	}
	#endif

	logMes_3("openChat: Show dlg");

	dlgChat->exec();
	delete dlgChat;
	dlgChat = NULL;
	
	#ifdef _XMPP
	if ( listitem->getProtocol() == PROT_JABBER && listitem->isConference() )
			disconnect( xmpp->zXMPPConf, SIGNAL(onChangeTextChat(std::string, QString)), this, SLOT(slot_onChangeTextChat(std::string, QString)) );
	#endif

	showedChat = false;
	
	qApp->installEventFilter( this );

	logMes_2("openChat: end");
}

void ZGui::lbContactSel( int i)
{
	logMes_2("lbContactSel: start");

	ZContactItem* listitem = lbContact->item ( lbContact->currentItem() );

	if ( !icq->connected 
		#ifdef _XMPP
		&& !xmpp->connected 
		#endif
					)
	{
		logMes_3("lbContactSel: select profile");
		#ifdef _XMPP
		int protInt=0;
		if ( listitem->getProtocol() == PROT_ICQ )
			protInt = 1;
		if ( listitem->getProtocol() == PROT_JABBER )
			protInt = 2;
		#endif
		for (int n=0; n<lbContact->count(); n++ )
			#ifdef _XMPP
			if ( lbContact->itemChecked(n) && n != i )
			{
				listitem = lbContact->item( n );
				if ( ( listitem->getProtocol()==PROT_ICQ && protInt == 1 ) || (  listitem->getProtocol()==PROT_JABBER && protInt == 2 ) )
			#endif
			if ( n != i ) {
					lbContact->checkItem(n, false);
			}
		return;
	}

	if ( listitem->isGroup() )
	{
		logMes_3("lbContactSel: Show/Hide group!");
		lbContact->showHideGroup( listitem->getGroupId() );
		return;
	} else
	{
		QByteArray data;
		QDataStream s( data, IO_WriteOnly ); 
		s << (int)listitem;
		QCopChannel::send( ZMESSANGER_CHENEL, "openChat(ZContactItem*)", data );
	}
	
	logMes_3("lbContactSel: end");
}

void ZGui::sendMessange(string id, QString mes, int protocol )
{
	logMes_2("sendMessange: run");
	#ifdef _XMPP
	if ( protocol == PROT_ICQ )
	{
	#endif
		int uin_ind = icq->findContact(id);
		
		if ( icq->isUnicodeCap(uin_ind) && icq->getStatus(uin_ind)!=STATUS_OFFLINE )
		{
			icq->sendMessage( ICQKid2Message( id , mes.utf8().data(), ICQKid2Message::UTF8, 0, false) );
		} else
		{
			QCString smes = codec->fromUnicode( mes );
			icq->sendMessage( ICQKid2Message( id, (string) smes, ICQKid2Message::LOCAL8BIT, 0, false) );
		}
		newMes(QString(id.c_str()),LNG_MY,mes,QDateTime::currentDateTime(),TYPE_MY_MESSAGE);
	#ifdef _XMPP
	} else	
	if ( protocol == PROT_JABBER )
	{
		logMes_3("sendMessange: XMPP");
		//if ( IS_CONFERENT(protocol) ) 		//			!!!!!!!!!!!! NEED FIX !!!!!!!!!!!!!
		//{
		//	xmpp->zXMPPConf->sendMessage(id, mes.utf8().data());
		//} else
		{
			xmpp->sendMessage( id, mes.utf8().data() );
			xmppNewMes(id,LNG_MY,mes,QDateTime::currentDateTime(),TYPE_MY_MESSAGE);
		}
	}
	#endif
}

bool  ZGui::eventFilter( QObject * o, QEvent * e)
{
	if ( e->type() == QEvent::KeyPress) 
	{ 
		if (o->className()!=QString("ZMyListBox"))
				return ZMainWidget::eventFilter( o, e );

		QKeyEvent *k = (QKeyEvent*)e;
		switch ( k->key() )
		{
			case Z6KEY_GREEN:
			{
				#ifdef _XMPP
				if ( !icq->connected && !xmpp->connected)
				#else
				if ( !icq->connected )
				#endif
					return true;
				
				cfg_dontShowOffLine = !cfg_dontShowOffLine;
				clearList();
				printContact( false );
				return true;
			}
			case Z6KEY_RED:
			{ 
				slot_ReturnToIdle(0);
				return true;
			}
			case Z6KEY_C:
			{
				ZEyeDialog * dlgEye  = new ZEyeDialog( );
				dlgEye->exec();
				delete dlgEye;
				dlgEye = NULL;

				return true;
			}	
			default:
				return ZMainWidget::eventFilter( o, e );
		} 
	}  
	return ZMainWidget::eventFilter( o, e ); 
}

QString ZGui::strtoqstr( string str, bool detectCodec )
{
	if ( detectCodec )
	{
		QTextCodec* codecStr = QTextCodec::codecForName("UTF-8");
		if ( codecStr->heuristicContentMatch( str.c_str(),strlen( str.c_str() ) ) == -1 )
		{
			return codec->toUnicode( str.c_str() );
		} else
		if ( codec->heuristicContentMatch( str.c_str(),strlen( str.c_str() ) ) == -1 )
		{
			return codecStr->toUnicode( str.c_str() );
		} else
		if ( codecStr->heuristicContentMatch( str.c_str(),strlen( str.c_str() ) ) > codec->heuristicContentMatch(str.c_str(),strlen( str.c_str() ) ) )
		{
			return QString::fromUtf8( str.c_str() );
		} else
		{
			return codec->toUnicode( str.c_str() );
		}
	} else
	{
		return codec->toUnicode( str.c_str() );
	}
}

#ifdef _SupportZPlayer
void ZGui::startPlayerChenel()
{
	logMes_1("startPlayerChenel: !!!!!!!!!!!!!!!!!!!!!!!");
	if ( chenelZPlayer == 0 )
		chenelZPlayer = new QCopChannel((QCString)channelNowPlaying, 0, 0);
	if ( QCopChannel::isRegistered( (QCString)channelNowPlaying ) )
	{
		logMes_1("startPlayerChenel: chenel ok");		
		connect(chenelZPlayer, SIGNAL(received(const QCString &, const QByteArray &)), this, SLOT(receivedZPlayerChenel(const QCString &, const QByteArray &)));
	}
	logMes_1("startPlayerChenel: end");
}

void ZGui::stopPlayerChenel()
{
	logMes_1("stopPlayerChenel: run");
	if ( QCopChannel::isRegistered( (QCString)channelNowPlaying ) )
		disconnect(chenelZPlayer, SIGNAL(received(const QCString &, const QByteArray &)), this, SLOT(receivedZPlayerChenel(const QCString &, const QByteArray &)));
}

/*
#define launcherChannel "/zIM/NowPlaying" 
QByteArray data; 
QDataStream s( data, IO_WriteOnly ); 
s << tagCommonTag.Artist; 
s << tagCommonTag.Title; 
s << tagCommonTag.Album; 
QCopChannel::send( (QCString)launcherChannel, QCString( "nowPlaying()" ), data );
*/

void ZGui::receivedZPlayerChenel(const QCString &msg, const QByteArray &data)
{
	logMes_1("receivedZPlayerChenel: run");
 	if ( msg == QCString( "nowPlaying()" ) ) 
	{	
		QDataStream s( data, IO_ReadOnly ); 
		QString Artist, Title, Album;
		s >> Artist;
		s >> Title;
		s >> Album;
		if (icq->connected)
			if (Artist.isEmpty()&&Title.isEmpty()&&Album.isEmpty() )
			{
				icq->setXStatusDescription("");
			}
			else	
			{
				QString status = cfg_nowPlaying;
				status.replace(QString("%artist%"), Artist);
				status.replace(QString("%album%"), Album);
				status.replace(QString("%title%"), Title);
				icq->setXStatusDescription( status.utf8().data() );
			}
	}
}
#endif
