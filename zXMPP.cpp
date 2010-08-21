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

#include "zXMPP.h"

#include <qglobal.h>

#include <client.h>
#include <jid.h>
#include <connectionbase.h>
#include <connectionlistener.h>
#include <gloox.h>
#include <disco.h>
#include "zgui.h"

static const struct
{
	QString name;
	QString filename;
	QString resource;
} xmpp_clients[XMPP_CLIENTS_COUNT] =
{
	{"Unknown","unknown",""},
	{"QIP Infium","qip_infium", "qip"},
	{"Miranda","miranda","miranda"},
	{"qutIM","qutim", "qutim"},
	{"mChat","mchat", "mchat"},
	{"Bombus Mod","bombusmod", "bombusmod"},
	{"Bombus +","bombusp", "bombus+"},
	{"Psi","psi", "psi"},
	{"zMessanger","zmessanger", "zmessanger"},
	{"JabbIM","jabbim", "jabbim"},
	{"Talkonaut","talk", "talkonaut"},
	{"Pidgin","pidgin", "РџРёРґР¶РёРЅ"},
	{"Bombus","bombus", "bombus"},
};

extern ZGui* zgui;

using namespace gloox;

zXMPP::zXMPP(): QObject(0, ""), connected(0)
{
	XMPPClient = 0;
	XMPPThread = 0;
	xmppBook = 0;
	XMPPThreadPing = 0;
	zXMPPConf = 0;
}

zXMPP::~zXMPP()
{
	//if (connected)
	//	XMPPClient->disconnect();
}

bool zXMPP::Disconnect()
{
	qDebug("zXMPP::Disconnect()");	
	if (connected)
		XMPPClient->disconnect();
	if (XMPPThread->running()) 
		XMPPThread->exit();
	delete XMPPThread;
	XMPPThread = 0;
	delete xmppBook;
	xmppBook = 0;
	if (XMPPThreadPing->running()) 
		XMPPThreadPing->exit();	
	delete XMPPThreadPing;
	XMPPThreadPing = 0;	
	delete zXMPPConf;
	zXMPPConf = 0;		
	delete XMPPClient;
	XMPPClient = 0;	
	qDebug("zXMPP::Disconnect() end");
}

bool zXMPP::Connect()
{
	qDebug("zXMPP::Connect()");

	if (connected)
		return false;
 
	if (XMPPThread)
	{
		delete XMPPThread;
		XMPPThread = 0;
	}
	if (XMPPClient)
	{
		delete XMPPClient;
		XMPPClient = 0;
	}

	XMPPClient = new Client(JID(jid.latin1()), pas.latin1());
	XMPPClient->registerConnectionListener(this);
	XMPPClient->rosterManager()->registerRosterListener( this );
	XMPPClient->registerMessageHandler( this );
	XMPPClient->setCompression( true );

	xmppBook = new BookmarkStorage( XMPPClient );
	xmppBook->registerBookmarkHandler( this );

	#if defined(EZX_EM30)
	#define platform "Motorola EM30"
	#elif defined(EZX_E8)
	#define platform "Motorola E8"
	#elif defined(EZX_ZN5)
	#define platform "Motorola ZN5"
	#elif defined(EZX_U9)
	#define platform "Motorola U9"
	#elif defined(EZX_V8)
	#define platform "Motorola V8"
	#elif defined(EZX_Z6)
	#define platform "Motorola Z6"
	#elif defined(EZX_EM35)
	#define platform "Motorola EM35"
	#elif defined(EZX_VE66)
	#define platform "Motorola VE66"
	#else
	#define platform "Motorola LinuxBase"									
	#endif									

	XMPPClient->disco()->setVersion( "zMessanger", QString(QString::number(VER_MAJOR)+"."+QString::number(VER_MINOR)).latin1(), platform );
	XMPPClient->setResource("zMessanger");
	int errCon = 0;
	oldConError = ConnConnectionRefused;
	while ( errCon!=-1 && ( errCon < 4 && oldConError == ConnConnectionRefused ) )
	{
		if ( XMPPClient->connect(false) )
		{
			errCon = -1;
			XMPPThread = new zXMPPThread( XMPPClient );
			XMPPThread->start();
		} else
		{
			errCon++;
		}
	}
	if ( errCon==3 )
	{
		emit onErrorConnect( "Error id = "+QString::number(oldConError) );
	} else
	{
		zXMPPConf = new zXMPPConferenc(XMPPClient);
	}
	qDebug("zXMPP::Connect() end");
}

void zXMPP::setProfile(QString _jid, QString _pas)
{
	jid = _jid;
	pas = _pas;
}

void zXMPP::sendMessage(std::string jid, std::string mes)
{
	Stanza *s = Stanza::createMessageStanza(jid, mes );
	XMPPClient->send( s );
}

void zXMPP::handleItemAdded(const gloox::JID &jid)
{
	qDebug("zXMPP::handleItemAdded");
	RosterItem *ritem = XMPPClient->rosterManager()->getRosterItem(jid);
	XMPPContListAdapter contact;
	contact.item = ritem;
	ContactListUins.push_back ( contact );
	string name;
	if(ritem->name().c_str()[0] == 0)
		name = ritem->jid();
	else
		name = ritem->name();
	zgui->xmppAddUser(name, ritem->online(),ContactListUins.size()-1);
}

void zXMPP::handleItemRemoved(const gloox::JID &jid)
{
	qDebug("zXMPP::handleItemRemoved");
	zgui->lbContact->contactRemove( jid.bare() );
	int i=findContact( jid.bare() );
	ContactListUins.erase ( ContactListUins.begin() + i );
};

void zXMPP::handleRoster(const Roster &roster) 
{
	qDebug("zXMPP::handleRoster");
	QString name;
	ContactListUins.clear();
	ContactListGroups.clear();
	for(Roster::const_iterator p = roster.begin(); p != roster.end(); p++)
	{
		XMPPContListAdapter contact;
		contact.item = p->second;
		ContactListUins.push_back ( contact );

		if(p->second->name().c_str()[0] == 0)
			name = QString::fromUtf8(p->first.c_str());
		else
			name = QString::fromUtf8(p->second->name().c_str());
		
		if (p->second->groups().size() > 0)
		{
			if ( findGroup( (*p->second->groups().begin()) ) == -1 )
			{
				ContactListGroups.push_back( (*p->second->groups().begin()) );	
			}
		}
	}
	ContactListGroups.push_back( "NoGroup" );
	ContactListGroups.push_back( "Conference" );
	xmppBook->requestBookmarks();
}

void zXMPP::handleBookmarks( const BookmarkList &bList, const ConferenceList &cList )
{
	confList = cList;
	bookList = bList;

	ConferenceList::const_iterator it_c = cList.begin();
	for( ; it_c != cList.end(); it_c++ )
	{
		printf( "jid: %s, name: %s, nick: %s, pwd: %s\n", (*it_c).jid.c_str(), (*it_c).name.c_str(), (*it_c).nick.c_str(), (*it_c).password.c_str() );
		ConferentList.push_back( XMPPConferentList((*it_c).jid, (*it_c).name, (*it_c).nick, (*it_c).password, (*it_c).autojoin));
		//if ( (*it_c).autojoin )
		//	zXMPPConf->joinRoom((*it_c).jid, (*it_c).nick, (*it_c).password);
	}
	zgui->xmppPrintContact();
}

bool zXMPP::handleSubscriptionRequest(const gloox::JID &jid, const std::string &msg)
{
	return true; //Auto ask auth request - true
}

void zXMPP::handleSelfPresence( const RosterItem& item, const std::string& resource, Presence presence, const std::string& /*msg*/ )
{
      printf( "self presence received: %s/%s -- %d\n", item.jid().c_str(), resource.c_str(), presence );
}

void zXMPP::handleRosterPresence(const RosterItem &item, const std::string &resource, Presence presence, const std::string &msg) 
{
	printf( "presence received: %s/%s -- %d\n", item.jid().c_str(), resource.c_str(), presence );
	int n = findContact(item.jid());
	int status = 0;
	if ( presence == XMPP_ONTOOFF)
	{
		status = XMPP_ONLINE;
	} else
	{
		status = presence;
	}
	for ( int i=1; i<XMPP_CLIENTS_COUNT; i++ )
	{
		if ( xmpp_clients[i].resource == QString::fromUtf8(resource.c_str()).lower() )
		{
			setClientId(n, i);
			zgui->xmppUpdateStatus(item.jid(),-1,status, i);
			return;
		}
		if ( i == XMPP_CLIENTS_COUNT - 1 )
			setClientId(n, 0);	
	}
	zgui->xmppUpdateStatus(item.jid(),-1,status);
}

void zXMPP::handleMessage( Stanza *stanza, MessageSession *session) //Incomming message
{
	qDebug("Incomming mes");
	if ( stanza->body().c_str()[0] != 0  )
	{
		int n = findContact(stanza->from().bare());
		zgui->xmppNewMes(stanza->from().bare(), QString(getNick(n).c_str()), QString::fromUtf8(stanza->body().c_str()), QDateTime::currentDateTime(), TYPE_MESSAGE);
		zgui->alert();
	}
}

void zXMPP::onDisconnect(gloox::ConnectionError e)
{
	qDebug("XMPP::onDisconnect");
	oldConError = e;
	ContactListUins.clear();
	ContactListGroups.clear();
	if ( connected )
	{
		//if ( XMPPThreadPing != NULL && XMPPThreadPing->running() )
		//	XMPPThreadPing->exit();
		qDebug("emit onConnected(false)");
		emit onConnected(false);
	}
	connected=false;
	qDebug("XMPP::onDisconnect end");
};

void zXMPP::onConnect()
{
	qDebug("XMPP::onConnect");
	connected=true;
	qDebug("emit onConnected(true)");
	XMPPThreadPing = new zXMPPThreadPing( XMPPClient, (int)(zgui->cfg_timeKeepConnect/3) );
	XMPPThreadPing->start();
	emit onConnected(true);
	qDebug("XMPP::onConnect end");
};

/////////////////////////////////////////////////////////////////
//////////////////////CONTACT MANAGER////////////////////////////
/////////////////////////////////////////////////////////////////

std::string zXMPP::toLower(std::string str)
{
	return QString::fromUtf8(str.c_str()).lower().latin1();	
}

int zXMPP::findContact(std::string jid)
{
	qDebug("zXMPP::findContact");
	jid = toLower(jid);
	for (size_t i=0; i<ContactListUins.size(); ++i)
		if ( ContactListUins[i].item->jid() ==  jid)
		{
			qDebug("return: "+QString::number(i));
			return i;
		}
	return -1;
}

bool zXMPP::isMesIcon(int id)
{
	if ( id>=ContactListUins.size() || id<0 )
		return false;
	return ContactListUins[id].mesIcon;
}

void zXMPP::setMesIcon(int id, bool res)
{
	if ( id>=ContactListUins.size() || id<0 )
		return;
	ContactListUins[id].mesIcon = res;
}

int zXMPP::getStatus(int id, int resId)
{
	if ( id>=ContactListUins.size() || id<0 )
		return 0;
	if ( ContactListUins[id].item->online() )
	{
		RosterItem::ResourceMap test = ContactListUins[id].item->resources();
		if ( test.size() > resId ) //test.begin() != test.end() )
		{
			return test.begin()->second->presence();
		}
	}
	return 0;
}

std::string zXMPP::getStatusText(int id, int resId)
{
	if ( id>=ContactListUins.size() || id<0 )
		return "";
	if ( ContactListUins[id].item->online() )
	{
		RosterItem::ResourceMap test = ContactListUins[id].item->resources();
		if ( test.size() > resId ) //test.begin() != test.end() )
		{
			return test.begin()->second->message();
		}
	}
	return "";
}

string zXMPP::getNick(int id)
{
	if ( id>=ContactListUins.size() || id<0 )
		return " ";
	if(ContactListUins[id].item->name().c_str()[0] == 0)
		return ContactListUins[id].item->jid();
	else
		return ContactListUins[id].item->name();
}

string zXMPP::getJID(int id)
{
	if ( id>=ContactListUins.size() || id<0 )
		return " ";
	return ContactListUins[id].item->jid();
}

int zXMPP::findGroup(std::string name)
{
	for (size_t i=0; i<ContactListGroups.size(); ++i)
		if (ContactListGroups[i] == name)
		{
			return i;
		}
	return -1;
}

int zXMPP::getContactCount()
{
	return ContactListUins.size();
}

int zXMPP::getGroupCount()
{
	return ContactListGroups.size();
}

std::string zXMPP::getGroupName(int id)
{
	if ( id>=ContactListGroups.size() || id<0 )
		return " ";
	return ContactListGroups[id];	
}

std::string zXMPP::getContactGroup(int id)
{
	if ( id>=ContactListUins.size() || id<0 )
		return "NoGroup";
	if ( ContactListUins[id].item->groups().size() == 0 )
		return "NoGroup";
	return (*ContactListUins[id].item->groups().begin());
}

bool zXMPP::isWaitAuth(int id)
{
	if ( id>=ContactListUins.size() || id<0 )
		return false;
	switch ( ContactListUins[id].item->subscription() )
	{
		case RosterItem::S10nNone:
		case RosterItem::S10nNoneOut:
		case RosterItem::S10nNoneIn:
		case RosterItem::S10nNoneOutIn:
      		case RosterItem::S10nFrom:
        	case RosterItem::S10nFromOut:
			return true;
		default:
			return false;
	}
}

bool zXMPP::authRequest(std::string jid)
{
	Stanza *s = Stanza::createSubscriptionStanza(jid, "", StanzaS10nSubscribe);
	XMPPClient->send( s );
}

bool zXMPP::authDelete(std::string jid)
{
	Stanza *s = Stanza::createSubscriptionStanza(jid, "", StanzaS10nUnsubscribed);
	XMPPClient->send( s );
}

int zXMPP::getClientId(int id)
{
	if ( id>=ContactListUins.size() || id<0 )
		return 0;
	return ContactListUins[id].clientId;
}

void zXMPP::setClientId(int id, int res)
{
	if ( id>=ContactListUins.size() || id<0 )
		return;
	ContactListUins[id].clientId = res;
}

int zXMPP::getGroupId(int id)
{
	std::string group = getContactGroup(id);
	for (size_t i=0; i<ContactListGroups.size(); ++i)
	if ( ContactListGroups[i] == group )
	{
		return i;
	}
	return -1;
}

QString zXMPP::getClientImageNeme(int id)
{
	if ( id > XMPP_CLIENTS_COUNT || id < 1 )
		return "-";
	return xmpp_clients[id].filename;
}

QString zXMPP::getClientNeme(int id)
{
	if ( id > XMPP_CLIENTS_COUNT || id < 1 )
		return "-";
	return xmpp_clients[id].name;
}

int zXMPP::getConferenceCount()
{
	return ConferentList.size();
}

std::string zXMPP::getConferenceJID(int id)
{
	if ( id>=ConferentList.size() || id<0 )
		return "";
	return ConferentList[id].jid;
}

std::string zXMPP::getConferenceName(int id)
{
	if ( id>=ConferentList.size() || id<0 )
		return "-";
	return ConferentList[id].name;
}

QString zXMPP::getConferenceText(int id)
{
	return zXMPPConf->xmppRommMes[getConferenceJID(id)];
}

bool zXMPP::joinRoom(std::string jid)
{
	for (size_t i=0; i<ConferentList.size(); ++i)
		if (ConferentList[i].jid == jid)
		{
			zXMPPConf->joinRoom(ConferentList[i].jid,ConferentList[i].nick,ConferentList[i].pas);
			return true;
		}

	return false;
}

/////////////////////////////////////////////////////////////////
/////////////////////////RUN THREAD//////////////////////////////
/////////////////////////////////////////////////////////////////

zXMPPThreadPing::zXMPPThreadPing(gloox::ClientBase *XMPPClient, int Time): QObject(0, ""), QThread()//
{
	this->XMPPClient = XMPPClient;
	_time = Time;
}

zXMPPThreadPing::~zXMPPThreadPing()
{
}

void zXMPPThreadPing::run() 
{
	qDebug("zXMPPThreadPing::run()");
	time_t oldPing = time(NULL);
	while ( zgui->xmpp->connected )
	{
		//sleep(time);
		qApp->processEvents(10);
		sleep(5);
		if ( (time(NULL)-oldPing) > _time )
		{
			XMPPClient->whitespacePing();
			oldPing = time(NULL);
		}
	}
	qDebug("zXMPPThreadPing::run() end");
}

zXMPPThread::zXMPPThread( gloox::ClientBase *XMPPClient ): QObject(0, ""), QThread()//
{
	this->XMPPClient = XMPPClient;
}

zXMPPThread::~zXMPPThread()
{
}

void zXMPPThread::run() 
{
	qDebug("zXMPPThread::run()");
	ConnectionError ce = ConnNoError;
	while ( ce == ConnNoError )
	{
		ce = XMPPClient->recv();
	}
	qDebug("zXMPPThread::run() end");
}
