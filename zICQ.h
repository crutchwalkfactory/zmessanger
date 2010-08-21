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
	int findGroup(int id);
	int getCountCL();
	int getCountNoCL();
	int getCountGroup();

	//void setWaitAuth();
	bool isWaitAuth(int id);
	string getNick(int id);
	string getUIN(int id);
	bool isUnicodeCap(int id);
	int getStatus(int id);
	bool isMesIcon(int id);
	void setMesIcon(int id, bool res);
	void setAskAuth(int id, bool res);
	bool isAskAuth(int id);
	bool isInBLM(int id);
	int getXStatus(int id);
	string getXStatusTitle(int id);
	string getXStatusDescription(int id);
	int getClientId(int id);
	int getGroupId(int id);
	int getGroupItemId(int id);

	bool setXStatus(int idStatus, QString title, QString desc, bool write = true );
	bool setStatus ( uint astat, bool write = true );
	void loadOldStatusPostConnect(bool b){ loadOldStatus = b; };

	string getGroupName(int id);
	

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
