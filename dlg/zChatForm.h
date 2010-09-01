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

#ifndef ZCHATFORM_H
#define ZCHATFORM_H

#include <string>

#include "config.h"
#include "BaseDlg.h"
#include "xTextView.h"
#include "ZImgeSelect.h"
#include <ZMultiLineEdit.h>
#include <qtimer.h>
#include <qmutex.h>
#include <ZImage.h>
#include <ZLabel.h>
#include <ZOptionsMenu.h>

#ifndef NEW_PLATFORM
#include "KbInputFieldAdapter.h"
#endif
#include <KbEditContext.h>

using namespace std;

class zChatForm : public MyBaseDlg 
{
  Q_OBJECT

  public:
	zChatForm( string uin_, QString nick, int protocol, bool conference = false );
	~zChatForm();
	
	xTextView * eeChat;
	ZMultiLineEdit * mleMes; 		
	
	string uin;
	int protocol;

	void setType(QString str);
	void setClient(QString str);
	void mesTyped( bool tuped );
	void setContOff();
	void stopTimerTyped();
	void buildMenu();
	#ifdef _XMPP
	void buildMenuConference();
	#endif
	void kbChangeByKey();
	void changeSelect();
	void addMes( QString str);

  protected:
	virtual bool eventFilter(QObject*, QEvent*);
	
  public slots:
  	void sendClick();
  	void musicPress();
	void addSmile(QString smile);
	void kbChange();
	void mesTextChanged();
	void timeDone();
	void slotAuthRequest();
	void slotAddContact();
	void slotAuthAcepted();
	void slotAuthDeclined();
	void slotClearMes();
	void slotClearChat();
	void slotTranslit();
	void copyText();
	void pastText();
	void slotViewHistory();
	#ifdef _XMPP
	void slotAuthDelete();
	void slotJoinRoom();
	#else
	void slotAuthDelete(){};
	void slotJoinRoom(){};
	#endif

  private:
  	ZLabel * labTyping;
  	ZLabel * labClient;
	ZLabel * labLng;
	ZLabel * labInputMode;
  	ZScrollPanel * spInfo;
	
	#ifndef CUTED_PLATFORM
	KbEditContext  * kbInput;
	#endif
	int nKbState;

	#ifdef _FixQVBoxLayout
	ZWidget *myWidget;
	#endif

	QTimer * timer;
	bool startTyped;

	ZOptionsMenu* menu;

	mutable QMutex mutexOnAddMes;
	mutable QMutex mutexOnType;
};

#endif

