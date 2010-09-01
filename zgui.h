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

#ifndef ZGUI_H
#define ZGUI_H

#include "GUI_Define.h"

//C/C++
#include <unistd.h>
#include <stdlib.h>
#include <sys/timeb.h>

//EZX
#include <ZKbMainWidget.h>
#include <ZListBox.h>
#include <ZSoftKey.h>
#include <ZOptionsMenu.h>
#include <ZProgressDlg.h>
#include <ZMessageDlg.h>

//QT
#include <qobject.h>
#include <qmap.h>
#include <qdatetime.h>
#include <qvaluelist.h>
#include <qthread.h>
#include <qregexp.h>
#include <qcopchannel_qws.h>

// dlg
#include "ZStatusInfo.h"
#include "ZUserInfo.h"
#include "zChatForm.h"
#include "ZAddProfile.h"

//NAPI
#include "NAPIManager.h"
#include "NAPILink.h"

//Custom component
#include "ZMyListBox.h"
#include "ZContactItem.h"

#include "config.h"
#include "zIMType.h"

//Protocol
#include "zICQ.h"
#ifdef _XMPP
#include "zXMPP.h"
#endif



class ZGui :public ZKbMainWidget
{
    Q_OBJECT
  public:
    ZGui (QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~ZGui();
    
    QString getProgramDir();

    ZMyListBox *lbContact;
    
    void printContact(bool Clear = true);
    
    ZContactItem * getICQContact(string uin);
    int  icqAddUser(QString Name, string uin, int Status, int xStatus, int clientId, int id, int groupId = -1, bool auth = false, bool mes = false, bool invisible = false);
    int  icqAddGroup(QString Name, int groupId);  

    void showProc(QString text);
    void setOperat(QString text, int max);
    void setProc(int i);
    void closeProc();
    bool showMesDlg(QString title, QString mes, int ico = 0);
    void newMes(QString uin,QString title, QString mes, QDateTime time, TYPE_BDMES type);
    void addContactToList(QString uin);

    tHistory messageList;

    zICQ *icq; 
    void sendMessange(string id, QString mes, int protocol = PROT_ICQ);

    void showProfileList();

    QString bdMesToText(BDMes mes, bool forHistory = false);

    ZOptionsMenu* menu;

    QTextCodec * codec;

    int strtoint(string str);
    QString strtoqstr( string str, bool detectCodec = true );

    bool trayMes;

    bool icqSet;

    void alert();

    #ifdef _XMPP
    bool xmppSet;
    zXMPP * xmpp;

    int xmppAddUser(string Name, int Status = 0, int id = 0, int clientId = 0, int groupId = -1, bool auth = false, bool mes = false);

    void xmppUpdateStatus(string jid, int contactId=-1, int Status=-1, int clientId = -1, bool mes=false, bool typing=false, bool ignorAll=false );

    int xmppAddGroup( QString Name, int groupId );

    void xmppNewMes(string jid, QString title, QString mes, QDateTime time, int type);

    void xmppPrintContact(bool Clear = true);
    
    ZContactItem * getXMPPContact(string jid);
    #endif

    void clearList();

	#ifdef _SupportZPlayer
    void startPlayerChenel();
    void stopPlayerChenel();
    #endif

    void createMenuConected();
    void createMenuDisconected();

    void onConnectChange (int protocol, bool online);
    
    void activSlot( bool con );

    void saveHistory( bool all = true, int uin = 0 ); 

  signals:
	void clickOnStatusMenu( int i);

  public slots:
    void menu_connect();
    void menu_minimize();
    void menu_showXStatus();
    void menu_userInfo();
    void menu_showChat();
    void menu_setXStatus();
    void menu_addContact();
    void menu_removeContact();
    void menu_addGroup();
    void menu_removeGroup();
    void menu_settings();
    void menu_about();
    void menu_copyUin();
    void menu_profile_del();
    void menu_profile_add();
    void menu_profile_setdef();
    void menu_profile_change();    
    void slot_Raise();
    void slot_ReturnToIdle( int reason );
    void updateStatus(); 
    void cancellICQ();
    void slotFixMenuBag();
    void slotFixMenuBag( int ) { slotFixMenuBag(); };

    void openChatTest();
    void openChat(ZContactItem* listitem);
    void menu_statusChange(int status);
    void menu_privStatusChange(int status);
	
    void slot_onUserNotify(string uin, uint32_t stat1, uint32_t stat2, bool invis_flag);
    void slot_onIncomingMTN(string from, uint16_t atype);
    void slot_onIncomingMsg(ICQKid2Message msg);
    void slot_onAuthRequest(string from, string text);
    void slot_onAuthReply(string from, string text, uint8_t aflag);
    void slot_onContactListChanged();
    void slot_onWasAdded(string from);
    void slot_onXstatusChanged(string uin, size_t x_status, string x_title, string x_descr);
    void slot_onErrorConnectICQ(QString mes);
    void slot_onConnectedICQ(bool conect);
    void slot_onClientChange( string uin, size_t clientId );
    void slot_onSingOff(uint16_t err_code, string err_url);
    void lbContactSel(int i);

	#ifdef _XMPP
    void slot_onErrorConnectXMPP(QString mes);	
    void slot_onConnectedXMPP(bool conect);
    void slot_onChangeTextChat(std::string, QString); 
    #else
    void slot_onErrorConnectXMPP(QString){};	
    void slot_onConnectedXMPP(bool){};   
    void slot_onChangeTextChat(std::string, QString){};     
	#endif

    void slot_internetConnected();

	#ifdef _SupportZPlayer
    void receivedZPlayerChenel(const QCString &, const QByteArray &);
    #else
    void receivedZPlayerChenel(const QCString &, const QByteArray &){}; 
    #endif
    void siganalReceived(const QCString&, const QByteArray&);

  protected:
    bool  eventFilter( QObject * o, QEvent * e);  

  private:
    void CreateWindow(QWidget* parent);
    void updateMyIcon();  
  
    ZProgressDlg *pProgressDialog;
    QTimer *timer;
    ZStatusInfo * dlgStat;
    zChatForm * dlgChat;
    ZSoftKey *softKey;
    ZOptionsMenu* menuContact;
    ZOptionsMenu* menustatus;
    ZOptionsMenu* menuCL;
    ZOptionsMenu* menuProfile;
    ZOptionsMenu* menuPrivatStatus;    

    bool isShown;
    int timeNotAction;

    ZWidget * myWidget;
    ZHeader* m_pHeader;

    bool showedChat;

    NAPIManager *napi;
    NAPILink *link;
	
    mutable QMutex mutexPrintContact;

	#ifdef _SupportZPlayer
    QCopChannel *chenelZPlayer;
    QString cfg_nowPlaying;
    #endif
    
    bool slotActiv;
};

#endif

