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

#ifndef zICQ_H
#define zICQ_H

#include <qstring.h>
#include <ZConfig.h>
#include "./icqlib/icqkid2.h"
#include <sys/timeb.h>

class zICQ : public ICQKid2
{
    Q_OBJECT	
  public:
	zICQ();
	~zICQ();

	QString ProgDir;

	bool Connect();
	void Disconnect();
	void startConnect();
	bool connected;

	bool miif_break_flag;

	string USC2BEto8BIT(string str);
	QString unicodeToUtf8(string str);

	vector<SSIUINEntry> NoContactListUins;
	int findNoCLUIN(string uin);
	int addContactToNoCLList(QString uin, QString nick);

	//Interface
	int findContact(string uin);
	int findGroup(uint id);
	int getCountCL();
	int getCountNoCL();
	int getCountGroup();

	//void setWaitAuth();
	bool isWaitAuth(uint id);
	string getNick(uint id);
	string getUIN(uint id);
	bool isUnicodeCap(uint id);
	int getStatus(uint id);
	bool isMesIcon(uint id);
	void setMesIcon(uint id, bool res);
	void setAskAuth(uint id, bool res);
	bool isAskAuth(uint id);
	bool isInBLM(uint id);
	int getXStatus(uint id);
	string getXStatusTitle(uint id);
	string getXStatusDescription(uint id);
	int getClientId(uint id);
	int getGroupId(uint id);
	int getGroupItemId(uint id);

	bool setXStatus( int idStatus, QString title, QString desc, bool write = true );
	bool setStatus ( uint astat, bool write = true );
	void loadOldStatusPostConnect(bool b){ loadOldStatus = b; };

	string getGroupName(uint id);
	

  signals:
  	void onErrorConnect(QString mes);
  	void onConnected(bool);
   
  private:
	void printContactList();  
	bool now_connect;
	timeb mark_tmstamp;
	int idle_marker;
	int idle_deep;

    int cfg_oldStatus;
    int cfg_oldXStatus;
    QString cfg_oldXStstusText;
    QString cfg_oldXStstusTitle;
    bool loadOldStatus;
};

#endif
