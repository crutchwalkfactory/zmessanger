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

#include "zXMPPConferenc.h"

#include <qglobal.h>

#include <client.h>
#include <jid.h>
#include <connectionbase.h>
#include <connectionlistener.h>
#include <gloox.h>
#include <disco.h>

#include "qdatetime.h"

#include "zgui.h"
#include "lng.h"

extern ZGui* zgui;

using namespace gloox;

zXMPPConferenc::zXMPPConferenc(ClientBase *parent): QObject(0, "")
{
	clientbase = parent;
}

zXMPPConferenc::~zXMPPConferenc()
{
	qDebug ("zXMPPConferenc::~zXMPPConferenc()");
	listRooms::Iterator it;
	for ( it = xmppRooms.begin(); it != xmppRooms.end(); ++it ) 
	{
		delete *it;
	}
	qDebug ("zXMPPConferenc::~zXMPPConferenc() end");
}

void zXMPPConferenc::joinRoom(std::string room, std::string nick, std::string pwd)
{
	qDebug ("Join room: "+QString(room.c_str())+" "+QString(nick.c_str ())+" "+QString(pwd.c_str ()));
	JID roomJID( room + "/" + nick );
	xmppRooms[room] = new MUCRoom( clientbase, roomJID, this, 0 );
	xmppRooms[room]->setNick(nick);
	xmppRooms[room]->setPassword(pwd);
	xmppRommMes[room] = "";
	xmppRooms[room]->join();
}

void zXMPPConferenc::sendMessage(std::string jid, std::string message)
{
	
	std::string room = jid; //QString::fromUtf8(jid.c_str()).mid(0,QString::fromUtf8(jid.c_str()).find("@")).utf8().data();
	if ( xmppRooms[room] == NULL )
	{
		qDebug("Error: not room "+QString::fromUtf8(room.c_str()));
		return;
	}
	xmppRooms[room]->send(message);
	QString text = QString("$4@$ ") + QString::fromUtf8(xmppRooms[room]->nick().c_str()) + JOIN_ROOM + QString("#\n");
	xmppRommMes[jid] = xmppRommMes[jid] + text;
	emit onChangeTextChat(jid, text);	
}

void zXMPPConferenc::handleMUCParticipantPresence( MUCRoom *room, const MUCRoomParticipant participant,Presence presence )
{
	if( presence == PresenceAvailable )
	{
		QString text = QString("$4@$ ") + QString::fromUtf8(participant.nick->resource().c_str()) + JOIN_ROOM + QString("#\n");
		std::string jid = room->name() +"@"+ room->service();
		xmppRommMes[jid] = xmppRommMes[jid] + text;
		emit onChangeTextChat(jid, text);
		printf( "%s is in the room, too\n", participant.nick->resource().c_str() );
	} else 
	if( presence == PresenceUnavailable )
	{
		QString text = QString("$4@$ ") + QString::fromUtf8(participant.nick->resource().c_str()) + LEFT_ROOM + QString("#\n");
		std::string jid = room->name() +"@"+ room->service();		
		xmppRommMes[jid] = xmppRommMes[jid] + text;
		emit onChangeTextChat(jid, text);
		printf( "%s left the room\n", participant.nick->resource().c_str() );
	}
	else
	printf( "Presence is %d of %s\n", presence, participant.nick->resource().c_str() );

      /**
Эта функция вызывается, когда в комнате жильца входит в комнату, изменения присутствия внутри помещения, или выходит из комнаты.
@ записка MUCRoomParticipant структуры, в том числе ссылки на JIDs, будут очищены после этой функции возвращаются.
@ Param номер комнаты.
@ Param участника А структуры, описывающие жильца статус и / или действий.
@ Param присутствие в присутствии пассажира.

This function is called whenever a room occupant enters the room, changes presence inside the room, or leaves the room.
@note The MUCRoomParticipant struct, including pointers to JIDs, will be cleaned up after this function returned.
@param room The room.
@param participant A struct describing the occupant's status and/or action.
@param presence The occupant's presence.
       */
}

void zXMPPConferenc::handleMUCMessage( MUCRoom *room, const std::string& nick,
                                     const std::string& message, bool history,
                                     const std::string& when, bool privateMessage )
{
	QString privat = "";
	if ( privateMessage )
		privat = " $2^$ [Private]#";
	std::string jid = room->name() +"@"+ room->service();
	QString text;
	if (history) 
	{
		QString date = QString::fromUtf8(when.c_str());
		date.remove(1,9);
		text = QString("$4%$ ") + QString::fromUtf8(nick.c_str()) + " [" + date + QString("]# $4@$ ") + QString::fromUtf8(message.c_str()) + privat + QString("# \n");
	} else
	{
		text = QString("$1%$ ") + QString::fromUtf8(nick.c_str()) + " [" + QDateTime::currentDateTime().time().toString() + QString("] # ") + QString::fromUtf8(message.c_str()) + privat + QString("\n");
	}
	xmppRommMes[jid] = xmppRommMes[jid] + text;
	qDebug ("onChangeTextChat(jid, text);");
	emit onChangeTextChat(jid, text);
	//printf( "%s said: '%s' (history: %s, private: %s) - %s\n", nick.c_str(), message.c_str(),
        //      history ? "yes" : "no", privateMessage ? "yes" : "no", when );
      /**
	
Эта функция вызывается при поступлении сообщения через комнату.
@ отметить Это может быть личное сообщение! Если сообщение является частным, и вы хотите на него ответить в частном порядке, вы должны создать новую MessageSession к пользователю полный номер ник и использовать его для каких-либо дальнейших частного общения с пользователем.
@ Param Комната сообщение пришло от.
@ Param ник Направляющее псевдоним пользователя в комнате.
@ Param сообщение сообщение.
@ Param истории Указывает ли или нет это сообщение, которое было направлено перед входом в комнату, и входит в комнату истории комнате посылает после присоединения.
@ Param при этом используется только в случае с историей @ @ B является подлинной, а затем содержит Datetime сообщение было направлено в нотации, как описано в XEP-0082.
@ Param privateMessage Обозначает ли это личное сообщение.

This function is called when a message arrives through the room.
@note This may be a private message! If the message is private, and you want to answer it privately, you should create a new MessageSession to the user's full room nick and use that for any further private communication with the user.
@param room The room the message came from.
@param nick The sending user's nickname in the room.
@param message The message.
@param history Indicates whether or not this is a message that was sent prior to entering the room and is part of the room history the room sends after joining.
@param when This is only used if @c history is @b true and then contains the datetime the message was sent in a notation as described in XEP-0082.
@param privateMessage Indicates whether this is a private message.
       */
}

bool zXMPPConferenc::handleMUCRoomCreation( MUCRoom *room )
{
printf( "room %s didn't exist, beeing created.\n", room->name().c_str() );

      /**
Эта функция вызывается, если в комнате, которая была только что присоединились не существовало до попытки присоединиться. Therfore комната была создана MUC службы. Чтобы принять умолчанию конфигурации зала назначен на MUC службы, возвращение @ B верно с этой функцией. Комната будет открыта в MUC услуга доступна и для других пользователей присоединиться к нему. Если вы не хотите, чтобы принять предложенное по умолчанию конфигурации комнат, возвращение @ B ложных от этой функции. В комнате будет оставаться заблокированной до тех пор, пока он либо полностью настроен, созданное в мгновение комнату, или создание отменяется.
Если вы вернулись из ложной этой функции вы должны использовать один из следующих вариантов:
@ ли использовать MUCRoom:: cancelRoomCreation (), чтобы отменить создание и удаление комната,
@ ли использовать MUCRoom:: acknowledgeInstantRoom (), чтобы согласиться на номер по умолчанию конфигурацию, либо
@ ли использовать MUCRoom:: requestRoomConfig (), чтобы просить номер конфигурации форме.
@ Param номер комнаты.
@ @ B вернуться True принять стандартный номер конфигурации, @ B ложных сохранить номер блокируется до тех пор, пока настроен вручную комнате владельца.

This function is called if the room that was just joined didn't exist prior to the attempted join. Therfore the room was created by MUC service. To accept the default configuration of the room assigned by the MUC service, return @b true from this function. The room will be opened by the MUC service and available for other users to join. If you don't want to accept the default room configuration, return @b false from this function. The room will stay locked until it is either fully configured, created as an instant room, or creation is canceled.
If you returned false from this function you should use one of the following options:
@li use MUCRoom::cancelRoomCreation() to abort creation and delete the room,
@li use MUCRoom::acknowledgeInstantRoom() to accept the room's default configuration, or
@li use MUCRoom::requestRoomConfig() to request the room's configuration form.
@param room The room.
@return @b True to accept the default room configuration, @b false to keep the room locked until configured manually by the room owner.
       */
}

void zXMPPConferenc::handleMUCSubject( MUCRoom *room, const std::string& nick,const std::string& subject )
{
      if( nick.empty() )
        printf( "Subject: %s\n", subject.c_str() );
      else
        printf( "%s has set the subject to: '%s'\n", nick.c_str(), subject.c_str() );

      /**
Эта функция вызывается, когда в комнату вопросу была изменена.
@ Param номер комнаты.
@ Param ником ник из пассажиров, которые изменили номер вопроса.
@ сведению с некоторыми MUC услуги ник может быть пустым, если помещение является первым вступил.
@ Param вопросу Новый номер вопроса.

This function is called when the room subject has been changed.
@param room The room.
@param nick The nick of the occupant that changed the room subject.
@note With some MUC services the nick may be empty when a room is first entered.
@param subject The new room subject.
       */
}

void zXMPPConferenc::handleMUCInviteDecline( MUCRoom *room, const JID& invitee, const std::string& reason )
{
      /**
	
Эта функция вызывается, когда пользователь предложил кто-то (например, с помощью MUCRoom:: Приглашение ()) в комнате, но предложение было отклонено в пользу этого лица.
@ Param номер комнаты.
@ Param приглашенных JID В том случае, если лицо, которое отклонил приглашение.
@ Param причине факультативные основания для снижения приглашение.

This function is called when the user invited somebody (e.g., by using MUCRoom::invite()) to the room, but the invitation was declined by that person.
@param room The room.
@param invitee The JID if the person that declined the invitation.
@param reason An optional  reason for declining the invitation.
       */
}

void zXMPPConferenc::handleMUCError( MUCRoom *room, StanzaError error )
{
	QString text = QString("$1@$ Error! Code: ") + QString::number(error) + QString("#\n");
	std::string jid = room->name() +"@"+ room->service();
	xmppRommMes[jid] = xmppRommMes[jid] + text;
	emit onChangeTextChat(jid, text);

	printf( "!!!!!!!!got an error: %d", error );
      /**
Эта функция вызывается при возникновении ошибки в помещении или при входе в комнату.
@ отметить следующие ошибки, указанные условия являются для MUC:
LI @ @ B @ B Не Уполномоченный: Пароль требуется.
LI @ @ B Запрещено: Отказано в доступе, пользователю запрещается.
LI @ @ B @ B Пункт Не @ B Наиденно: в комнате, не существует.
LI @ @ B @ B Не допускаются: Номер создание ограничен.
LI @ @ B @ B Не допускается: Номер ники заблокированы вниз.
LI @ @ B @ B регистрации обязательно: Пользователь не о членах списка.
@ @ B ли конфликт: Желаемый номер псевдоним уже используется или другим зарегистрированным пользователям.
LI @ @ B @ B служба недоступна: Максимальное количество пользователей, была достигнута.
Другие ошибки могут появляться в зависимости от реализации услуг.
@ Param номер комнаты.
@ Param ошибка ошибке.

This function is called when an error occurs in the room or when entering the room.
@note The following error conditions are specified for MUC:
@li @b Not @b Authorized: Password required.
@li @b Forbidden: Access denied, user is banned.
@li @b Item @b Not @b Found: The room does not exist.
@li @b Not @b Allowed: Room creation is restricted.
@li @b Not @b Acceptable: Room nicks are locked down.
@li @b Registration @b Required: User is not on the member list.
@li @b Conflict: Desired room nickname is in use or registered by another user.
@li @b Service @b Unavailable: Maximum number of users has been reached.
Other errors might appear, depending on the service implementation.
@param room The room.
@param error The error.
       */
}

void zXMPPConferenc::handleMUCInfo( MUCRoom *room, int features, const std::string& name, const DataForm *infoForm )
{
printf( "features: %d, name: %s, form xml: %s\n", features, name.c_str(), infoForm->tag()->xml().c_str() );
      /**
Эта функция, как правило (см. ниже) вызывается в ответ на призыв к MUCRoom:: getRoomInfo ().
@ Param номер комнаты.
@ Param функции ORed MUCRoomFlag's.
@ Param название комнаты имя возвращается Service Discovery.
@ Param infoForm А DataForm продлен номер, содержащий информацию. Может быть 0, если служба не поддерживает расширенный номере информации. См. раздел 15.5 XEP-0045 для определенных типов полей.
@ сведению данную функцию можно назвать без предварительного вызова на MUCRoom:: getRoomInfo (). Это происходит, если в комнате конфиг изменяется, e.g. в комнате администратора.
This function usually (see below) is called in response to a call to MUCRoom::getRoomInfo().
@param room The room.
@param features ORed MUCRoomFlag's.
@param name The room's name as returned by Service Discovery.
@param infoForm A DataForm containing extended room information. May be 0 if the service doesn't support extended room information. See Section 15.5 of XEP-0045 for defined field types.
@note This function may be called without a prior call to MUCRoom::getRoomInfo(). This happens if the room config is changed, e.g. by a room admin.
       */
}

void zXMPPConferenc::handleMUCItems( MUCRoom *room, const StringMap& items )
{
      StringMap::const_iterator it = items.begin();
      for( ; it != items.end(); ++it )
      {
        printf( "%s -- %s is an item here\n", (*it).first.c_str(), (*it).second.c_str() );
      }
      /**
Эта функция вызывается в ответ на призыв к MUCRoom:: getRoomItems ().
@ Param номер комнаты.
@ Param пунктов Карта номер участников. Ключевым является наименование, стоимость является жильца комнаты JID. Карта может быть пустым, если такая информация является конфиденциальной.
This function is called in response to a call to MUCRoom::getRoomItems().
@param room The room.
@param items A map of room participants. The key is the name, the value is the occupant's room JID. The map may be empty if such info is private.
       */
}
