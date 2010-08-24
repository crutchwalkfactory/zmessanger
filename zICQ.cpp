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

#include "zICQ.h"
#include "const_strings.h"
#include "icqkid2.h"

#include <qglobal.h>

#include <zgui.h>
#include "zDefs.h"

void * startConnect_(void*);
//void * startMainLoop_(void*);

zICQ::zICQ()
	: ICQKid2(), now_connect(false), idle_marker(0), idle_deep(0)
{
	ftime(&mark_tmstamp);	
	connected = false;
	
    cfg_oldStatus = 0;
    cfg_oldXStatus = 0;
    cfg_oldXStstusText = "";
    cfg_oldXStstusTitle = "";
    loadOldStatus = false;
}

zICQ::~zICQ()
{
	
}

void zICQ::Disconnect()
{
	connected = false;
	doDisconnect();
	emit onConnected(false);
}

int zICQ::findNoCLUIN(string uin)
{
	for (size_t i=0; i<NoContactListUins.size(); ++i)
		if (NoContactListUins[i].uin==uin)
			return i+1000;
	
	return -1;
}

int zICQ::addContactToNoCLList(QString uin, QString nick)
{
	SSIUINEntry uen;
	uen.uin=uin.latin1();
	uen.nick=nick.latin1();
	uen.groupid=ICQ_NOT_IN_LIST_GROUP;
	uen.groupname="";
	uen.itemid=0;
	uen.waitauth = false;
	uen.unicode_cap = false;	
	NoContactListUins.push_back(uen);
	int id = NoContactListUins.size()-1;
	return id + 1000;
}

bool zICQ::Connect()
{
	logMes_1("zICQ: Connect()");
	now_connect=true;
	if ( !doConnect() )
	{
		doDisconnect();
		connected = false;
		
		QString error = "Cannot connect to ICQ service!\nConnection error: ";
		switch(connect_error_code)
		{
			case CONN_ERR_BAD_LOGIN :
				error = error+"CONN_ERR_BAD_LOGIN";
				break;
			case CONN_ERR_BAD_PASSWORD :
				error = error+"CONN_ERR_BAD_PASSWORD";
				break;
			case CONN_ERR_RATE_LIMIT :
				error = error+"CONN_ERR_RATE_LIMIT";
				break;
			default :
				error = error+"UNKNOWN";
				break;
		}
		error = error+"\nDescription url:\n"+QString::fromLocal8Bit(connect_error_url.c_str())+"\nStep: "+QString::number( stepConnect );
		
		logMes("zICQ: Error - "+error);
		
		logMes_2("zICQ: onErrorConnect(error)");
		emit onErrorConnect(error);

		return false;
	} else
	{
		if ( loadOldStatus )
		{
			ZConfig cfg( ProgDir+"/oldStatus.cfg");
    		cfg_oldStatus = cfg.readNumEntry(QString("ICQ"), QString("status"), 0); 	
    		cfg_oldXStatus = cfg.readNumEntry(QString("ICQ"), QString("xstatus"), 0);
    		cfg_oldXStstusText = cfg.readEntry(QString("ICQ"), QString("xstatusText"), "");
    		cfg_oldXStstusTitle = cfg.readEntry(QString("ICQ"), QString("xstatusTitle"), ""); 
    		
    		if ( cfg_oldXStatus > 0)	
    			setXStatus( cfg_oldXStatus, cfg_oldXStstusText, cfg_oldXStstusTitle, false );
    		if ( cfg_oldStatus > 0)	
				setStatus ( cfg_oldStatus, false );			
		}
		connected = true;
		now_connect = false;
		
		logMes_1("zICQ: Conected!!!!!");
		emit onConnected(true);
		return true;
	} 
	return false;
}

string zICQ::USC2BEto8BIT(string str)
{
	string ret_str;
	size_t steps = str.length()/2;
	for (size_t i=0; i<steps; ++i) ret_str += str[i*2+1];
	return ret_str;
}

QString zICQ::unicodeToUtf8(string str)
{
	 string ret_str;
	 for ( uint i = 0; i < str.length(); i +=2)
	 {
		 Q_UINT16 symbol = static_cast<Q_UINT16>(str[i] * 0x100 + str[i+1]);
		 if ( symbol < 0x0080)
			 ret_str += ((unsigned char) symbol);
		 else if (symbol < 0x0800) {
			 ret_str += (0xc0 | ( symbol >> 6));
			 ret_str += (0x80 | (symbol & 0x3f));
		 } else {
			 ret_str += (0xe0 | ( symbol >> 12));
			 ret_str += (0x80 | ((symbol >> 6 )& 0x3f));
			 ret_str += (0x80 | (symbol & 0x3f));
		 }
	 }
	 return QString::fromUtf8(ret_str.c_str());
}

void zICQ::startConnect()
{
	pthread_attr_t attr;
	pthread_t thread;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&thread, &attr, startConnect_, NULL);
	pthread_attr_destroy(&attr);
}

void * startConnect_(void*)
{
	if ( zgui->icq->Connect() )
	{	
		logMes_3("startConnect_: 1");
		zgui->printContact();
		logMes_3("startConnect_: 2");
		if ( !zgui->icq->getOfflineMessages() )
 			logMes_3("startConnect_: Error read off line message!");
		logMes_3("startConnect_: 3");
		zgui->icq->mainLoop();
	}
	pthread_exit(NULL);
}


int zICQ::findContact(string uin)
{
	int id = findCLUIN(uin);
	if (id<0)
	{
		logMes_3("findContact: Not find in cl!\nFind in no cl:");
		id = findNoCLUIN(uin);
		if ( id >= 0 )
		{
			logMes_3("findContact: Finded in NO CL");
			id += 1000;
		} else
			logMes_3("findContact: Not in no cl");
	}
	
	logMes_2("findContact: Return: %d", id);
	return id;
}

int zICQ::getCountCL()
{
	return ContactListUins.size();
}

int zICQ::getCountNoCL()
{
	return NoContactListUins.size();
}

int zICQ::getCountGroup()
{
	return ContactListGroups.size();
}

bool zICQ::isWaitAuth(uint id)
{
	if ( id < 1000 )
	{
		if ( id>=ContactListUins.size() )
			return false;
		return ContactListUins[id].waitauth;
	} else
	{
		id -= 1000;
		if ( id>=NoContactListUins.size() )
			return false;
		return NoContactListUins[id].waitauth;	
	}
}

string zICQ::getNick(uint id)
{
	if ( id < 1000 )
	{
		if ( id>=ContactListUins.size() )
			return "";
		return ContactListUins[id].nick;
	} else
	{
		id -= 1000;
		if ( id>=NoContactListUins.size() )
			return "";
		return NoContactListUins[id].nick;	
	}
}

string zICQ::getUIN(uint id)
{
	if ( id < 1000 )
	{
		if ( id>=ContactListUins.size() )
			return "0";
		return ContactListUins[id].uin;
	} else
	{
		id -= 1000;
		if ( id>=NoContactListUins.size() )
			return "0";
		return NoContactListUins[id].uin;	
	}
}

bool zICQ::isUnicodeCap(uint id)
{
	if ( id < 1000 )
	{
		if ( id>=ContactListUins.size() )
			return 0;
		return ContactListUins[id].unicode_cap;
	} else
	{
		id -= 1000;
		if ( id>=NoContactListUins.size() )
			return 0;
		return NoContactListUins[id].unicode_cap;	
	}
}

int zICQ::getStatus(uint id)
{
	if ( id < 1000 )
	{
		if ( id>=ContactListUins.size() )
			return 0;
		return ContactListUins[id].invisible?STATUS_INVISIBLE:ContactListUins[id].online_status;
	} else
	{
		id -= 1000;
		if ( id>=NoContactListUins.size() )
			return 0;
		return NoContactListUins[id].online_status;	
	}
}

bool zICQ::isMesIcon(uint id)
{
	if ( id < 1000 )
	{
		if ( id>=ContactListUins.size() )
			return false;
		return ContactListUins[id].mesIcon;
	} else
	{
		id -= 1000;
		if ( id>=NoContactListUins.size() )
			return false;
		return NoContactListUins[id].mesIcon;	
	}
}

void zICQ::setMesIcon(uint id, bool res)
{
	if ( id < 1000 )
	{
		if ( id>=ContactListUins.size() )
			return;
		ContactListUins[id].mesIcon = res;
	} else
	{
		id -= 1000;
		if ( id>=NoContactListUins.size() )
			return;
		NoContactListUins[id].mesIcon = res;
	}
}

void zICQ::setAskAuth(uint id, bool res)
{
	if ( id < 1000 )
	{
		if ( id>=ContactListUins.size() )
			return;
		ContactListUins[id].askAuth = res;
	} else
	{
		id -= 1000;
		if ( id>=NoContactListUins.size() )
			return;
		NoContactListUins[id].askAuth = res;
	}
}

bool zICQ::isAskAuth(uint id)
{
	if ( id < 1000 )
	{
		if ( id>=ContactListUins.size() )
			return false;
		return ContactListUins[id].askAuth;
	} else
	{
		id -= 1000;
		if ( id>=NoContactListUins.size() )
			return false;
		return NoContactListUins[id].askAuth;	
	}
}

bool zICQ::isInBLM(uint id)
{
	if ( id < 1000 )
	{
		if ( id>=ContactListUins.size() )
			return true;
		return ContactListUins[id].isBLM;
	} else
	{
		id -= 1000;
		if ( id>=NoContactListUins.size() )
			return true;
		return NoContactListUins[id].isBLM;	
	}
}

int zICQ::getXStatus(uint id)
{
	if ( id < 1000 )
	{
		if ( id>=ContactListUins.size() )
			return 0;
		return ContactListUins[id].xStatus;
	} else
	{
		id -= 1000;
		if ( id>=NoContactListUins.size() )
			return 0;
		return NoContactListUins[id].xStatus;
	}
}

bool zICQ::setXStatus(int idStatus, QString title, QString desc, bool write )
{
	if (write)
	{	
		cfg_oldXStatus = idStatus;
		cfg_oldXStstusText = title;
		cfg_oldXStstusTitle = desc;
	
		ZConfig cfg( ProgDir+"/oldStatus.cfg");
    	cfg.writeEntry(QString("ICQ"), QString("xstatus"), cfg_oldXStatus);
   		cfg.writeEntry(QString("ICQ"), QString("xstatusText"), cfg_oldXStstusText);
    	cfg.writeEntry(QString("ICQ"), QString("xstatusTitle"), cfg_oldXStstusTitle);      
    	cfg.flush();
    }
    
    return ICQKid2::setXStatus(idStatus, title.utf8().data(), desc.utf8().data());
}

bool zICQ::setStatus ( uint astat, bool write )
{
	if (write)
	{
		cfg_oldStatus = astat;
	
		ZConfig cfg( ProgDir+"/oldStatus.cfg");
    	cfg.writeEntry(QString("ICQ"), QString("status"), cfg_oldStatus); 
    	cfg.flush();
    }
    
    return ICQKid2::setStatus(astat);	
}

string zICQ::getXStatusTitle(uint id)
{
	if ( id < 1000 )
	{
		if ( id>=ContactListUins.size() )
			return "";
		return ContactListUins[id].xStatusTitle;
	} else
	{
		id -= 1000;
		if ( id>=NoContactListUins.size() )
			return "";
		return NoContactListUins[id].xStatusTitle;	
	}
}

string zICQ::getXStatusDescription(uint id)
{
	if ( id < 1000 )
	{
		if ( id>=ContactListUins.size() )
			return "";
		return ContactListUins[id].xStatusDescription;
	} else
	{
		id -= 1000;
		if ( id>=NoContactListUins.size() )
			return "";
		return NoContactListUins[id].xStatusDescription;	
	}
}

int zICQ::getClientId(uint id)
{
	if ( id < 1000 )
	{
		if ( id>=ContactListUins.size() )
			return 0;
		return ContactListUins[id].clientId;
	} else
	{
		id -= 1000;
		if ( id>=NoContactListUins.size() )
			return 0;
		return NoContactListUins[id].clientId;	
	}
}

int zICQ::getGroupId(uint id)
{
	if ( id < 1000 )
	{
		if ( id>=ContactListUins.size() )
			return 0;
		return ContactListUins[id].groupid;

	} else
	{
		id -= 1000;
		if ( id>=NoContactListUins.size() )
			return 0;
		return NoContactListUins[id].groupid;

	}
}

string zICQ::getGroupName(uint id)
{
	if ( id>=ContactListGroups.size() )
		return " ";
	return ContactListGroups[id].name;
}

int zICQ::getGroupItemId(uint id)
{
	if ( id>=ContactListGroups.size() )
		return 0;
	return ContactListGroups[id].id;
}

int zICQ::findGroup(uint id)
{
	id = findCLGroup(id);
	return id;
}
