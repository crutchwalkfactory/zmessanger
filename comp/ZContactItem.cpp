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

#include "ZContactItem.h"

#include "ZMyListBox.h"

#include "config.h"
#include "zDefs.h"

#define update() if(!IsHide&&(lbParent->itemVisible(this))) lbParent->UpdateList()

using namespace std;

ZContactItem::ZContactItem( ZMyListBox* _container, CONTACT_TYPE _type ):
	ZSettingItem(_container, CONVERT_TYPE(_type) )
{
	type = _type;
	lbParent = _container;
	
	IsGroup = false;
	group = -1;
	protocol = PROT_ICQ;
	typeIn = false;
	newMes = false; 
	status = STATUS_OFFLINE;
	xStatus = -1;
	clientId = 255;
	uid = "";
	IsHide = true;
	
	appendSubItem ( GET_NUM_NICK(type), " ", true );

	switch ( type )
	{
		case ITEM_CONACT:
			// for dont jump image in list
			setPixmap ( GET_NUM_FAKE(type), QPixmap(ProgDir + "/CL/fake.png") );
			
			//connect( this, SIGNAL(needUpdate()), lbParent, SLOT(UpdateList()));	
			break;
		case ITEM_SPLITER:
			setSelectable(false);
			setSeparator("-",NULL);
			break;			
	}
}

ZContactItem::~ZContactItem()
{

}

void ZContactItem::setNick( QString nick )
{
	setSubItem( GET_NUM_NICK(type), 0, nick , true );
}

QString ZContactItem::getNick()
{
	return getSubItemText ( GET_NUM_NICK(type), 0 );
}

#ifdef _XMPP
bool ZContactItem::isConference()
{
	return IsConference;	
}

void ZContactItem::setConference(bool b)
{
	IsConference = b;	
}
#endif

bool ZContactItem::isGroup()
{
	return IsGroup;
}

void ZContactItem::setGroup(bool b)
{
	IsGroup = b;
	if ( IsGroup )
		setHide(false);
}

bool ZContactItem::isNoCL()
{
	return IsNoCL;
}

void ZContactItem::setNoCL(bool b)
{
	IsNoCL = b;
}

int ZContactItem::getGroupId()
{
	return group;
}

string ZContactItem::getGroupIdStr()
{
	return QString::number(group).latin1();
}

void ZContactItem::setGroupId(int id)
{
	group = id;
}    

int ZContactItem::getProtocol()
{
	return protocol;
}

void ZContactItem::setProtocol(int n)
{
	protocol = n;
	#ifdef _XMPP
	if ( n==PROT_JABBER )
		setStatus ( XMPP_OFFLINE, true );
	else
	#endif
		setStatus ( STATUS_OFFLINE, true );	
}

string ZContactItem::getUID()
{
	return uid;
}

void ZContactItem::setUID(string id)
{
	uid = id;
}

void ZContactItem::setStatus(int n, bool update)
{
	if ( status == n && !update )
		return;
		
	if ( typeIn || newMes )
		return;
		
	status = n;
	
	QString ico;
	QPixmap pm;	
	
	if ( protocol == PROT_ICQ )
	{
		switch(status)
		{
			case STATUS_OFFLINE    : ico = "offline.png"; break;
			case STATUS_ONLINE     : ico = "online.png"; break;
			case STATUS_AWAY       : ico = "away.png"; break;
			case STATUS_DND        : ico = "dnd.png"; break;
			case STATUS_NA         : ico = "na.png"; break;
			case STATUS_OCCUPIED   : ico = "occupied.png"; break;
			case STATUS_FREE4CHAT  : ico = "ffc.png"; break;
			case STATUS_EVIL       : ico = "evil.png"; break;
			case STATUS_DEPRESSION : ico = "depression.png"; break;
			case STATUS_HOME       : ico = "athome.png"; break;
			case STATUS_WORK       : ico = "atwork.png"; break;
			case STATUS_LUNCH      : ico = "lunch.png"; break;
			case STATUS_INVISIBLE  : ico = "invisible.png"; break;
			default: 
			{
				ico = "online.png";
				logMes("Error Status "+QString::number(status)+" not found!!!");
			}
		}
		pm.load( ProgDir + QString ( "/status/icq/" ) + ico );
	} 
	#ifdef _XMPP
	else if ( protocol == PROT_JABBER )
	{
		switch(status)
		{
			case XMPP_OFFLINE    : ico = "offline.png"; break;
			case XMPP_ONLINE     : ico = "online.png"; break;
			case XMPP_FREE4CHAT  : ico = "ffc.png"; break;
			case XMPP_AWAY       : ico = "away.png"; break;
			case XMPP_DND        : ico = "dnd.png"; break;
			case XMPP_NA         : ico = "na.png"; break;
			default                : ico = "unknown.png";
		}
	
		pm.load( ProgDir + QString ( "/status/jabber/" ) + ico );
	}
	#endif
	
	setPixmap ( GET_NUM_STATUS(type), pm );
	
	update();
}

void ZContactItem::setStatusX(int _xStatus)
{
	if ( xStatus == _xStatus )
		return;
	
	xStatus = _xStatus;
	
	if ( protocol == PROT_ICQ )
	{
		if ( xStatus != X_STATUS_NONE )
		{
			QPixmap pm;
			setPixmap ( GET_NUM_FAKE(type), pm );
			
			xStatus-=1;
			pm.load( ProgDir + QString ( "/status/icq/x/icq_xstatus" ) + QString::number(xStatus) + ".png");
			setPixmap ( GET_NUM_XSTATUS(type), pm );
		} else
		{
			QPixmap pm;
			setPixmap ( GET_NUM_XSTATUS(type), pm );

			pm.load( ProgDir + "/CL/fake.png" );
			setPixmap ( GET_NUM_FAKE(type), pm );
		}
	}
	
	update();
}

void ZContactItem::setClient(int n)
{
	logMes_3("ZContactItem::setClient( "+QString::number(n)+" )");
	
	if ( (clientId == n) || ( n >= ICQ_CLIENTS_COUNT ) )
		return;
	
	clientId = n;
	
	QPixmap pm;
	pm.load( ProgDir + QString( "/clients/cli_"+icq_clients[clientId].filename+".png" ) );
	setPixmap ( GET_NUM_CLIENT(type), pm );
}

int ZContactItem::getClient()
{
	return clientId;
}

void ZContactItem::setType(bool b)
{
	if ( typeIn == b )
		return;
		
	typeIn = b;	
	
	if ( b )
	{
		setPixmap ( GET_NUM_STATUS(type), QPixmap( ProgDir + "/CL/typing.png" ) );
	} else
	{
		setStatus( status, true );
	}

	update();
}

void ZContactItem::setNewMes(bool b)
{
	if ( newMes == b )
		return;
		
	newMes = b;	
	
	if ( b )
	{
		setPixmap ( GET_NUM_STATUS(type), QPixmap( ProgDir + "/CL/onmes.png" ) );
		setType( false );
	} else
	{
		setStatus( status, true );
	}
	
	if ( IsHide )
	{
		
	}
	
	update();
}

void ZContactItem::setWaitAuth(bool b)
{
	QPixmap pm;
	if ( b )
	{
		pm.load( ProgDir + "/CL/waitauth.png" );
		setPixmap ( GET_NUM_AUTH(type), pm );		
	} else
	{
		setPixmap ( GET_NUM_AUTH(type), NULL );
	}
}

int ZContactItem::intStat()
{
	if ( newMes )
		return 0;
	
	#ifdef _XMPP
	if ( protocol == PROT_JABBER )
	{
		switch ( status )
		{
			case XMPP_OFFLINE      : return 13; break;
			case XMPP_ONLINE       : return 2; break;
			case XMPP_AWAY         : return 8; break;
			case XMPP_DND          : return 10; break;
			case XMPP_NA           : return 11; break;
			case XMPP_FREE4CHAT    : return 1; break;
			default                : return 14;
		}	
	} else
	#endif
	{
		switch ( status )
		{
			case STATUS_OFFLINE    : return 13; break;
			case STATUS_ONLINE     : return 2; break;
			case STATUS_AWAY       : return 8; break;
			case STATUS_DND        : return 10; break;
			case STATUS_NA         : return 11; break;
			case STATUS_OCCUPIED   : return 9; break;
			case STATUS_FREE4CHAT  : return 1; break;
			case STATUS_EVIL       : return 6; break;
			case STATUS_DEPRESSION : return 7; break;
			case STATUS_HOME       : return 3; break;
			case STATUS_WORK       : return 4; break;
			case STATUS_LUNCH      : return 5; break;
			case STATUS_INVISIBLE  : return 12; break;
			default                : return 14;
		}
	}
}

bool ZContactItem::isHide()
{
	return IsHide;
}

void ZContactItem::setHide(bool hide)
{
	IsHide = hide;
	if ( IsGroup )
	{
		newMes = false;
			
		QPixmap pm;	
		if ( IsHide )
			pm.load( ProgDir + "/CL/group.png");
		else
			pm.load( ProgDir + "/CL/group_2.png");
		setPixmap ( GET_NUM_STATUS(type), pm );
	}
}

void ZContactItem::setBirthday(bool b)
{
	if ( b )
		setPixmap ( GET_NUM_AUTH(type), QPixmap( ProgDir + "/CL/birthday.png" ) );
}
