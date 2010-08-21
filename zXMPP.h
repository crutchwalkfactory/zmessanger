#ifndef _XMPP_H_
#define _XMPP_H_

#include <qstring.h>
#include <qthread.h>
#include <qobject.h>

#include <string>
#include <vector>
#include <map>
#include <set>

#include <client.h>
#include <jid.h>
#include <connectionbase.h>
#include <connectionlistener.h>
#include <gloox.h>
#include <rosterlistener.h>
#include <clientbase.h>
#include <messagehandler.h>
#include <stanza.h>
#include <messagesession.h>
#include <rosteritem.h>
#include <bookmarkhandler.h>
#include <bookmarkstorage.h>

#include "zXMPPConferenc.h"

#include "zIMType.h"

#include "config.h"

using namespace gloox;

class XMPPContListAdapter
{
	public:
		inline XMPPContListAdapter() 
		{ 
			mesIcon = false;
			clientId = 0;
		}
		RosterItem * item;
		bool mesIcon;
		char clientId;
};

class XMPPConferentList
{
	public:
		inline XMPPConferentList() 
		{ 
			jid = "";
			name = "";
			nick = "";
			pas = "";
			autoLogin = false;
		}
		inline XMPPConferentList( std::string jid_, std::string name_, std::string nick_, std::string pas_, bool autoLogin_) 
		{ 
			jid = jid_;
			name = name_;
			nick = nick_;
			pas = pas_;
			autoLogin = autoLogin_;
		}
		std::string jid;
		std::string name;
		std::string nick;
		std::string pas;
		bool autoLogin;
};

class zXMPPThread:public QObject, public QThread
{
    Q_OBJECT
  public:
    zXMPPThread(gloox::ClientBase *XMPPClient);
    virtual ~zXMPPThread();

  protected:
	virtual void run();

  private:
	gloox::ClientBase * XMPPClient;
};

class zXMPPThreadPing:public QObject, public QThread
{
    Q_OBJECT
  public:
    zXMPPThreadPing(gloox::ClientBase *XMPPClient, int Time = 100);
    virtual ~zXMPPThreadPing();

  protected:
	virtual void run();

  private:
	gloox::ClientBase * XMPPClient;
	int _time;
};

class zXMPP: public QObject, public gloox::RosterListener, public gloox::ConnectionListener, public gloox::MessageHandler, public BookmarkHandler
{
    Q_OBJECT
  public:
	zXMPP();
	virtual ~zXMPP();

	bool Connect();
	void setProfile(QString _jid, QString _pas);
	bool Disconnect();
	bool connected;

	//RosterListener
	void handleItemAdded (const gloox::JID &jid);
	void handleItemSubscribed (const gloox::JID &jid){qDebug("1");};
	void handleItemRemoved (const gloox::JID &jid);
	void handleItemUpdated (const gloox::JID &jid){qDebug("3");};
	void handleItemUnsubscribed (const gloox::JID &jid){qDebug("4");};
	void handleRoster (const gloox::Roster &roster);
	void handleRosterPresence (const gloox::RosterItem &item, const std::string &resource, gloox::Presence presence, const std::string &msg);
	void handleSelfPresence (const gloox::RosterItem &item, const std::string &resource, gloox::Presence presence, const std::string &msg);
	bool handleSubscriptionRequest (const gloox::JID &jid, const std::string &msg);
	bool handleUnsubscriptionRequest (const gloox::JID &jid, const std::string &msg){ qDebug("9"); return 0;};
	void handleNonrosterPresence (gloox::Stanza *stanza){qDebug("10");};
	void handleRosterError (gloox::Stanza *stanza){qDebug("11");};

	//ConnectionListener
	void onConnect();
	void onDisconnect(gloox::ConnectionError e);
	void onResourceBindError(gloox::ResourceBindError error){qDebug("XMPP::onResourceBindError");};
	void onSessionCreateError(gloox::SessionCreateError error){qDebug("XMPP::onSessionCreateError");};
	bool onTLSConnect(const gloox::CertInfo &info){qDebug("XMPP::onTLSConnect"); return true;};

	//MessageHandler
	virtual void handleMessage( Stanza* stanza, MessageSession* session = 0 );

	//BookmarkHandler
	virtual void handleBookmarks( const BookmarkList &bList, const ConferenceList &cList );
	
	//Messages array/interface
	tHistoryXMPP messageList;

	int findContact(std::string jid);
	int findGroup(std::string name);

	int getContactCount();
	int getGroupCount();

	std::string getGroupName(int id);
	std::string getContactGroup(int id);

	bool isWaitAuth(int id);
	bool authDelete(std::string jid);
	bool authRequest(std::string jid);

	bool isMesIcon(int id);
	void setMesIcon(int id, bool res);

	int getStatus(int id, int resId = 0);
	std::string getStatusText(int id, int resId = 0);
	std::string getNick(int id);
	std::string getJID(int id);
	int getClientId(int id);
	void setClientId(int id, int res);
	int getGroupId(int id);
	QString getClientImageNeme(int id);
	QString getClientNeme(int id);
	void sendMessage(std::string jid, std::string mes);
	std::string toLower(std::string str);

	int getConferenceCount();
	std::string getConferenceJID(int id);
	std::string getConferenceName(int id);
	QString getConferenceText(int id);
	bool joinRoom(std::string jid);

	//Public for connect signal
	zXMPPConferenc * zXMPPConf;

  signals:
	void onConnected(bool);
	void onErrorConnect(QString);

  private:
	QString jid;
	QString pas;

	gloox::Client * XMPPClient;
	BookmarkStorage * xmppBook;
	zXMPPThread * XMPPThread;
	zXMPPThreadPing * XMPPThreadPing;

	std::vector <XMPPContListAdapter> ContactListUins;
	std::vector <std::string> ContactListGroups;
	std::vector <XMPPConferentList> ConferentList;

	BookmarkList bookList;
	ConferenceList confList;
	ConnectionError oldConError;
};

#endif
