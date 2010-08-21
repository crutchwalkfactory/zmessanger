#ifndef _XMPP_CONFER_H_
#define _XMPP_CONFER_H_

//#include "zXMPP.h"

#include <mucroomhandler.h>
#include <mucroom.h>
#include <disco.h>
#include <stanza.h>
#include <dataform.h>

#include <qstring.h>
#include <qmap.h>
#include <qobject.h>

using namespace gloox;

typedef QMap<std::string, MUCRoom* > listRooms;
typedef QMap<MUCRoom*, std::string > listJid;
typedef QMap<std::string, QString > listMes;
//typedef QMap<int, std::string> listRoomsId;


class zXMPPConferenc: public QObject, public MUCRoomHandler
{
    Q_OBJECT
  public:
	zXMPPConferenc(ClientBase *parent);
	virtual ~zXMPPConferenc();
	
	//MUCRoomHandler
	virtual void handleMUCParticipantPresence( MUCRoom *room, const MUCRoomParticipant participant,Presence presence );
	virtual void handleMUCMessage( MUCRoom *room, const std::string& nick, const std::string& message, bool history, const std::string& when, bool privateMessage );
	virtual bool handleMUCRoomCreation( MUCRoom *room );
	virtual void handleMUCSubject( MUCRoom *room, const std::string& nick,const std::string& subject );
	virtual void handleMUCInviteDecline( MUCRoom *room, const JID& invitee,const std::string& reason );
	virtual void handleMUCError( MUCRoom *room, StanzaError error );
	virtual void handleMUCInfo( MUCRoom *room, int features, const std::string& name,const DataForm *infoForm );
	virtual void handleMUCItems( MUCRoom *room, const StringMap& items );

	//User
	void joinRoom(std::string room, std::string nick, std::string pwd);
	void sendMessage(std::string jid, std::string message);

	//
	listMes xmppRommMes;

  signals:
	void onChangeTextChat(std::string, QString);
	void onAlert();

  private:
    listRooms xmppRooms;
    //listRoomsId xmppRoomsId;
    ClientBase * clientbase;
};

#endif
