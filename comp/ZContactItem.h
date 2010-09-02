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

#ifndef ZCONTCTITEM_H
#define ZCONTCTITEM_H

#include <string>
#include <qobject.h>

#ifdef OLD_SDK
#include <ZListBox.h>
#else
#include <ZSettingItem.h>
#endif

#include "zDefs.h"

class ZMyListBox;

class Q_EXPORT ZContactItem : public ZSettingItem
{  
public: 
	ZContactItem( ZMyListBox* _container, CONTACT_TYPE _type );
	~ZContactItem();

	void setNick( QString nick );
	QString getNick();

    bool isGroup();
    void setGroup(bool b);
    
    bool isNoCL();
    void setNoCL(bool b);    
    
    #ifdef _XMPP
    bool isConference();
    void setConference(bool b); 
    #endif   
    
    int getGroupId();
    string getGroupIdStr();
    void setGroupId(int id);    

    int getProtocol();
	void setProtocol(int n);
	
    string getUID();
	void setUID(string id);
	
	void setStatus(int n, bool update = false);
	void setStatusX(int n);
	void setClient(int n);
	int  getClient();
	void setType(bool b);
	void setNewMes(bool b);
	bool isNewMes();
	void setWaitAuth(bool b);
	void setBirthday(bool b);
	
	int intStat();
	
	bool isHide();
	void setHide(bool hide);
	
protected:
	#ifdef _XMPP
	bool IsConference;
	#endif
	bool IsGroup;
	bool IsNoCL;
	int  group;
	char protocol;
	
	bool typeIn;
	bool newMes;
	int status;
	int xStatus;
	int clientId;

	string uid;
	
	bool IsHide;
	
	CONTACT_TYPE type;
	
	ZMyListBox * lbParent;
};

#endif
