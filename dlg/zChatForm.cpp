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

#include "zChatForm.h"
#include "config.h"

#include <qclipboard.h>
#include <ZApplication.h>
#include <ZSoftKey.h>

#include <ZLabel.h>
#include <ZScrollView.h>
#include <ZMainWidget.h>  
#include <ZScrollPanel.h> 
#include <ZAppInfoArea.h> 
#include <RES_ICON_Reader.h>
#include <ZSingleCaptureDlg.h>

#include "ZHistoruView.h"

#include "ZMyApplication.h"
#include "zgui.h"
#include "zDefs.h"

#ifdef CUTED_PLATFORM
#include "ZKbConfig.h"
#define insertItem(a,b,c,d,e,f) insertItem(QString("   ")+a,b,c,d,e,f)
#endif


zChatForm::zChatForm(std::string _uin, QString nick, int _protocol, bool /*_conference*/)
    :MyBaseDlg()
{
	logMes_2("zChatForm: start");
	
	uin = _uin;
	protocol = _protocol;

	setFullScreenMode(cfg_noShowStatusBarInChat, false);
	
	if (nick != "")
	{
		setMainWidgetTitle ( nick );
	} else
	{
		setMainWidgetTitle ( QString::fromUtf8( uin.c_str() ) );
	}

	logMes_3("zChatForm: create VBoxLayout");
	#ifndef _FixQVBoxLayout
	QVBoxLayout *myVBoxLayout = getVBoxLayout();
	#define PERENT this
	#else
	myWidget = new ZWidget ( this, NULL, 0);
	QVBoxLayout *myVBoxLayout = new QVBoxLayout(myWidget);
	#define PERENT myWidget
	#endif
	
	logMes_3("zChatForm: create xTextView");
	eeChat = new xTextView( PERENT, zSmile );
	eeChat->setFixedWidth(SCREEN_WIDTH);
	
	#ifdef SCREEN_GORIZONTAL
	eeChat->setFixedHeight( 165-(cfg_noShowStatusBarInChat?10:headerSize().height())-1 );
	#else
	eeChat->setFixedHeight( 235-(cfg_noShowStatusBarInChat?10:headerSize().height()) );
	#endif

	eeChat->setFontSize( cfg_chatFontSize );
	eeChat->setFocusPolicy( QWidget::NoFocus );

	myVBoxLayout->addWidget(eeChat,0);

	logMes_3("zChatForm: create ZScrollPanel");
	spInfo = new ZScrollPanel ( PERENT, NULL, 0, ZSkinService::clsZScrollPanel);
	spInfo->setFixedWidth(SCREEN_WIDTH);
	spInfo->setFixedHeight(15);
	
	logMes_3("zChatForm: create label");
	labTyping = new ZLabel(spInfo, "");
	labClient = new ZLabel(spInfo, "");
	labLng = new ZLabel(spInfo, "");
	labInputMode = new ZLabel(spInfo, "");
	 
	QFont font ( qApp->font() ); 
	font.setPointSize ( 12 );
	labClient->setFont( font );
	labTyping->setFont( font );
	labLng->setFont( font ); 
	labInputMode->setFont( font ); 
	labInputMode->setFixedWidth(40);
	labTyping->setFontColor( QColor(0,0,255) );

	labClient->setFixedWidth(80);
	labTyping->setFixedWidth(120);
	labInputMode->setFixedWidth(20);
	labLng->setFixedWidth(20);

	spInfo->addChild(labLng, SCREEN_WIDTH - 17, 1);
	spInfo->addChild(labTyping, ((int)(SCREEN_WIDTH / 2))-25, 1);
	spInfo->addChild(labClient, 5, 1);
	spInfo->addChild(labInputMode, SCREEN_WIDTH - 42, 1);
	
	myVBoxLayout->addWidget(spInfo,1);
	
	logMes_3("zChatForm: create ZMultiLineEdit");
	mleMes = new ZMultiLineEdit( PERENT, false );
	mleMes->setUnderline( false );
	mleMes->setSizePolicy( QSizePolicy ( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
	mleMes->setInsertionMethod ( ZMultiLineEdit::atPosNoSelected );
	mleMes->setFixedWidth( SCREEN_WIDTH );
	
	#ifdef SCREEN_GORIZONTAL
	mleMes->setFixedHeight( 40+(cfg_noShowStatusBarInChat?10:0) );
	#else
	mleMes->setFixedHeight( 70+(cfg_noShowStatusBarInChat?10:0) );
	#endif

	mleMes->setFontPercent( (float)cfg_mesFontSize/(float)22 );
	mleMes->setText(oldMes);
	
	myVBoxLayout->addWidget(mleMes,2);

	mleMes->setFocusPolicy( QWidget::StrongFocus );
	mleMes->setFocus();

	if ( !cfg_notSendTypeMes && protocol==PROT_ICQ )
		connect( mleMes, SIGNAL( textChanged() ), this, SLOT( mesTextChanged() ) );
	
	logMes_3("zChatForm: create menu");
	
	QRect rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	QPoint p = QPoint( 0, SCREEN_HEIGHT );
	menu = new ZOptionsMenu ( rect, this, NULL, 0);
	menu->setItemSpacing(10);
	menu->setSpacing(0);
	menu->setLimitRect(rect);
	menu->setPosition( ZOptionsMenu::BottomLeft, p );
	#ifdef _XMPP
	if ( _conference )
	{
		buildMenuConference();
	} else
	#endif
	{
		buildMenu();
	}

	#ifndef NEW_PLATFORM
	kbInput = app->getInputFieldAdapter()->getEditContext();
	#elif !defined( CUTED_PLATFORM )
	logMes_3("zChatForm: get edit context");
	kbInput = KbEditContext::instance();
	#endif
	nKbState = 0;
	
	this->installEventFilter( this );
	mleMes->installEventFilter( this );

	startTyped = false;
	timer = NULL;
	
	//FIX ME: cuted platform have metod setMorphMode
	#if defined(NEW_PLATFORM) && !defined(CUTED_PLATFORM)
	setMorphMode(3); // for light all key
	#endif
	
	#ifdef _FixQVBoxLayout
	setContentWidget(myWidget);
	#endif
	
	setSoftKey(NULL);
	
	logMes_3("zChatForm: end");
}

zChatForm::~zChatForm()
{
	QMutexLocker locker(&mutexOnAddMes);
	this->removeEventFilter( this );
	mleMes->removeEventFilter( this );
	oldMes = mleMes->text();
	if (!cfg_notSendTypeMes && protocol==PROT_ICQ )
		stopTimerTyped();
	zSmile->clearCash();
}

void zChatForm::buildMenu()
{
	logMes_3("zChatForm->buildMenu: start");
	bool auth, notlist, askAuth;

	auth = false;
	notlist = false;
	askAuth = false;
	
	if ( protocol==PROT_ICQ )
	{
		int n = zgui->icq->findContact( uin );
		if ( n != -1 )
		{
			if (zgui->icq->isWaitAuth(n))
			{
				auth = true;
			}	
			if ( zgui->icq->isInBLM(n) )
			{
				notlist = true;
			}
			if ( zgui->icq->isAskAuth(n) )
			{
				askAuth = true;
			}
		}
	} 
	#ifdef _XMPP
	else if ( protocol==PROT_JABBER )
	{
		int n = zgui->xmpp->findContact( uin );
		if  (zgui->xmpp->isWaitAuth(n) )
		{
			auth = true;
		}
	}
	#endif

	QPixmap* pm  = new QPixmap();

	pm->load(ProgDir+ "/menu/send_mes.png");
	menu->insertItem ( LNG_SEND , NULL, pm, true, 0, 0 );
	menu->connectItem ( 0, this, SLOT ( sendClick() ) );
	pm->load(ProgDir+ "/menu/add_smile.png");
	menu->insertItem ( LNG_ADDSMILE , NULL, pm, true, 1, 1 );
	menu->connectItem ( 1, this, SLOT ( musicPress() ) );	
  	menu->insertSeparator(2, 2); 

	if (askAuth)
	{
		pm->load(ProgDir+ "/menu/auth_acepted.png");
		menu->insertItem ( LNG_ASKAUTH_ACEPTIT , NULL, pm, true, 3, 3 );
		menu->connectItem ( 3, this, SLOT ( slotAuthAcepted() ) );
		pm->load(ProgDir+ "/menu/auth_declined.png");
		menu->insertItem ( LNG_ASKAUTH_DECLINED , NULL, pm, true, 3, 3 );
		menu->connectItem ( 3, this, SLOT ( slotAuthDeclined() ) );
		menu->insertSeparator(4, 4); 
	}

	if (auth)
	{
		pm  = new QPixmap();
		menu->insertItem ( LNG_MENU_AUTHREQUEST , NULL, pm, true, 5, 5 );
		menu->connectItem ( 5, this, SLOT ( slotAuthRequest() ) );
	} 
	#ifdef _XMPP
	else if ( protocol==PROT_JABBER )
	{
		pm  = new QPixmap();
		menu->insertItem ( LNG_AUTHDEL , NULL, pm, true, 5, 5 );
		menu->connectItem ( 5, this, SLOT ( slotAuthDelete() ) );
		auth = true; // for add Separator
	}
	#endif

	if (notlist)
	{
		pm->load(ProgDir+ "/menu/user_add.png");
		menu->insertItem ( LNG_ADDCONTACT , NULL, pm, true, 6, 6 );
		menu->connectItem ( 6, this, SLOT ( slotAddContact() ) );
	}		
	if ( auth || notlist )
	{
		menu->insertSeparator(7, 7); 
	}
	pm->load(ProgDir+ "/menu/copy.png");
	menu->insertItem ( LNG_COPY , NULL, pm, true, 8, 8 );
	menu->connectItem ( 8, this, SLOT ( copyText() ) );
	pm->load(ProgDir+ "/menu/past.png");	
	menu->insertItem ( LNG_PAST , NULL, pm, true, 9, 9 );	
	menu->connectItem ( 9, this, SLOT ( pastText() ) );
	menu->insertSeparator(10, 10); 
	
	pm->load(ProgDir+ "/menu/clear_chat.png");	
	menu->insertItem ( LNG_CLEAR_CHAT, NULL, pm, true, 11, 11 );
	menu->connectItem ( 11, this, SLOT ( slotClearChat() ) );	
	pm  = new QPixmap();
	menu->insertItem ( LNG_CLEAR , NULL, pm, true, 12, 12 );
	menu->connectItem ( 12, this, SLOT ( slotClearMes() ) );
	pm->load(ProgDir+ "/menu/history.png");	
	menu->insertItem ( LNG_HISTORU , NULL, pm, true, 13, 13 );
	menu->connectItem ( 13, this, SLOT ( slotViewHistory() ) );	
	pm->load(ProgDir+ "/menu/translit.png");
	menu->insertItem ( LNG_TRANSLIT , NULL, pm, true, 14, 14 );	
	menu->connectItem ( 14, this, SLOT ( slotTranslit() ) );
	
	logMes_3("zChatForm->buildMenu: end");
}

#ifdef _XMPP
void zChatForm::buildMenuConference()
{
	QPixmap* pm  = new QPixmap();

	menu->clear();

	//pm->load(ProgDir+ "/menu/auth_acepted.png");
	menu->insertItem ( LNG_JOIN_ROOM , NULL, pm, true, 3, 3 );
	menu->connectItem ( 3, this, SLOT ( slotJoinRoom() ) );
	//pm->load(ProgDir+ "/menu/auth_declined.png");
	//menu->insertItem ( LNG_ASKAUTH_DECLINED , NULL, pm, true, 3, 3 );
	//menu->connectItem ( 3, this, SLOT ( slotAuthDeclined() ) );
	//menu->insertSeparator(4, 4); 

	pm->load(ProgDir+ "/menu/send_mes.png");
	menu->insertItem ( LNG_SEND , NULL, pm, true, 0, 0 );
	menu->connectItem ( 0, this, SLOT ( sendClick() ) );
	pm->load(ProgDir+ "/menu/add_smile.png");
	menu->insertItem ( LNG_ADDSMILE , NULL, pm, true, 1, 1 );
	menu->connectItem ( 1, this, SLOT ( musicPress() ) );	
  	menu->insertSeparator(2, 2); 

	pm->load(ProgDir+ "/menu/copy.png");
	menu->insertItem ( LNG_COPY , NULL, pm, true, 8, 8 );
	menu->connectItem ( 8, this, SLOT ( copyText() ) );
	pm->load(ProgDir+ "/menu/past.png");	
	menu->insertItem ( LNG_PAST , NULL, pm, true, 9, 9 );	
	menu->connectItem ( 9, this, SLOT ( pastText() ) );
	menu->insertSeparator(10, 10); 
	
	pm  = new QPixmap();
	menu->insertItem ( LNG_CLEAR , NULL, pm, true, 11, 11 );
	menu->connectItem ( 11, this, SLOT ( slotClearMes() ) );
	pm->load(ProgDir+ "/menu/translit.png");
	menu->insertItem ( LNG_TRANSLIT , NULL, pm, true, 13, 13 );	
	menu->connectItem ( 13, this, SLOT ( slotTranslit() ) );
}
#endif

void zChatForm::slotAuthAcepted()
{
	if ( protocol==PROT_ICQ )
	{
		zgui->icq->authReply(uin, "You are welcome!", AUTH_ACCEPTED);
		int n = zgui->icq->findContact( uin );
		zgui->icq->setAskAuth(n, false);
	}	
	buildMenu();
}

void zChatForm::slotAuthDeclined()
{
	if ( protocol==PROT_ICQ )
	{
		zgui->icq->authReply(uin, "Sorry :(", AUTH_DECLINED);
		int n = zgui->icq->findContact( uin );
		zgui->icq->setAskAuth(n, false);
	}
	buildMenu();
}

void zChatForm::slotAuthRequest()
{
	if ( protocol==PROT_ICQ )
	{
		ZSingleCaptureDlg* zscd = new ZSingleCaptureDlg(LNG_AUTHREQUEST, LNG_TEXT, 	ZSingleCaptureDlg::TypeLineEdit, this, "", true, 0, 0);
		ZLineEdit* zle = (ZLineEdit*)zscd->getLineEdit();
		zle->setText("Please, authorize me!");

		if (zscd->exec() == 1)
		{
			QString text = zle->text().stripWhiteSpace();
			zgui->icq->authRequest( uin, text.utf8().data() );
		}
		delete zle;
		delete zscd;
	} 
	#ifdef _XMPP
	else if ( protocol==PROT_JABBER )
	{
		zgui->xmpp->authRequest( uin );
	}
	#endif
}

#ifdef _XMPP
void zChatForm::slotAuthDelete()
{
	zgui->xmpp->authDelete( uin );
}
#endif

void zChatForm::slotAddContact()
{
	zgui->addContactToList(QString(uin.c_str()));
}

void zChatForm::slotClearMes()
{
	mleMes->setText("");
}

void zChatForm::slotClearChat()
{
	zgui->saveHistory( false, atoi(uin.c_str()) );
	eeChat->setText("");
}

void zChatForm::slotViewHistory()
{
	ZHistoryView * dlgHistory = new ZHistoryView( uin );
	dlgHistory->exec();
	delete dlgHistory;
	dlgHistory = NULL;	
}


void zChatForm::slotTranslit()
{
	QString rus[33] = {"а", "б", "в", "г", "д", "е", "ё", "ж", "з", "и", "й", "к", "л", "м", "н", "о", "п", "р", "с", "т", "у", "ф", "х", "ц", "ч", "ш", "щ", "ъ", "ы", "ь", "э", "ю", "я"};

	QString rus_lat[33] = {"a", "b", "v", "g", "d", "e", "e", "zh", "z", "i", "i", "k", "l", "m", "n", "o", "p", "r", "s", "t", "u", "f", "h", "c", "ch", "sh", "sh", "'", "y", "'", "e", "yu", "ya"};

	QString  txt;
	txt = mleMes->text();

	for (int i = 0; i < 33; i++)
	{
		txt.replace( QString::fromUtf8(rus[i]), QString(rus_lat[i]));
		txt.replace( QString(rus[i].upper()), QString(rus_lat[i].upper()));
	}
	mleMes->setText(txt);
}

void zChatForm::stopTimerTyped()
{
	QMutexLocker locker(&mutexOnType);
	if ( startTyped )
	{
		mesTyped (false);
	}
	if ( timer != NULL && timer->isActive() )
	{
		timer->stop(); 
		delete timer;
		timer = NULL;
	}
}

void zChatForm::timeDone()
{
	mesTyped( false );
}

void zChatForm::mesTextChanged()
{
	if ( mutexOnType.locked() )
		return;
	QMutexLocker locker(&mutexOnType);
	if ( timer != NULL && timer->isActive() )
	{
		timer->stop();
		delete timer;
		timer = NULL;
	}
	timer = new QTimer( this ); 
	connect ( timer, SIGNAL( timeout() ), this, SLOT( timeDone() ) );
	timer->start( 5000, TRUE); 
	if (!startTyped)
	{
		mesTyped( true );
	}
}

void zChatForm::mesTyped( bool tuped )
{
	if (cfg_notSendTypeMes && protocol==PROT_ICQ )
		return;
	if ( !zgui->icq->connected )
		return;
	if ( !tuped )
	{
		zgui->icq->sendMTN(uin, MTN_FINISH);
		startTyped = false;
	}else
	if ( !mleMes->text().isEmpty() )
	{
		zgui->icq->sendMTN(uin, MTN_BEGIN);
		startTyped = true;
	} 	
}

void zChatForm::setType(QString str)
{
	labTyping->setText(str);
	labTyping->update();
}

void zChatForm::addMes(QString str)
{
	QMutexLocker locker(&mutexOnAddMes);
	eeChat->addText( str );
	eeChat->toEnd();
}

void zChatForm::setClient(QString str)
{
	labClient->setFontColor( QColor(0,0,0) );
	labClient->setText(str);
	labClient->update();
}

void zChatForm::setContOff()
{
	labClient->setFontColor( QColor(255,0,0) );
	labClient->setText(LNG_OFFLINE);
	labClient->update();
}

void zChatForm::addSmile(QString smile)
{
	mleMes->insert(" "+smile+" ");
	mesTextChanged();
}

void zChatForm::musicPress()
{
	ZImgeSelect * dlgSmile  = new ZImgeSelect();
	connect(dlgSmile, SIGNAL( addSmile(QString) ), this, SLOT( addSmile(QString) ) );
	dlgSmile->exec();
	disconnect ( dlgSmile, SIGNAL( addSmile(QString) ) );
	delete dlgSmile;
	dlgSmile = NULL;
}

void zChatForm::sendClick()
{
	#ifdef _XMPP
	if ( (zgui->icq->connected && protocol==PROT_ICQ ) || (zgui->xmpp->connected && protocol==PROT_JABBER ) )
	#else
	if ( zgui->icq->connected )
	#endif
	{
		if (mleMes->text() != "")
		{	
			if ( !cfg_notSendTypeMes && protocol==PROT_ICQ )
			{
				stopTimerTyped();
			}
			zgui->sendMessange( uin, mleMes->text(), protocol);
			mleMes->setText("");
		}
	} else
	{
		
		if ( mleMes->text() != "" )
		{
			#ifndef _VersionTest
			eeChat->addText("[*MES*] $1%$Server#\nNot connect to server...");
			mleMes->setText("");		
			#else
			eeChat->addText("[*MES*] $1%$TestMes#\n"+mleMes->text());
			eeChat->toEnd();
			mleMes->setText("");					
			#endif
		}
	}
	
}

void zChatForm::kbChange()
{
	logMes_3("zChatForm->kbChange: start");
	#ifndef CUTED_PLATFORM
	QUuid inputMode = kbInput->getFieldInputMode(0);
	#else
	QUuid inputMode;
	ZKbConfig::getFieldInputMode(0, inputMode);
	#endif
	logMes_3("zChatForm->kbChange: mode - "+inputMode);

	if (inputMode == ZKB_INPUT_PREDICTIVE )
	{
		nKbState=0;
	} else
	if (inputMode == ZKB_INPUT_MULTITAP)
	{
		nKbState=1;
	} else
	if (inputMode == ZKB_INPUT_NUMERIC)
	{
		nKbState=2;
	} else
	if (inputMode == ZKB_INPUT_SYMBOL)
	{
		nKbState=3;
	} else
	{
		nKbState=-1;
	}
	logMes_3("zChatForm->kbChange: is num "+QString::number(nKbState));

	QString n;
	switch (nKbState)
	{
		case 0:  n="iTap"; break;
		case 1:  n="Tap";  break;
		case 2:  n="123";  break;
		case 3:  n="#*)";  break;
		default: n="?";	
	}
	
	#ifndef CUTED_PLATFORM
	labLng->setText(QString(kbInput->getFieldInputLang(0).at(0))+QString(kbInput->getFieldInputLang(0).at(1)));
	#else
	QString lng;
	ZKbConfig::getFieldInputLang(0, lng);
	labLng->setText(QString(lng.at(0))+QString(lng.at(1)));
	#endif
	labInputMode->setText(n);
	
	logMes_3("zChatForm->kbChange: end");
}

void zChatForm::kbChangeByKey()
{
	if (nKbState == 3)
	{
		nKbState = 0;
	} else
	{
		nKbState++;
	}

	QString n;

	switch (nKbState)
	{
		case 0:  n="iTap"; break;
		case 1:  n="Tap";  break;
		case 2:  n="123";  break;
		case 3:  n="#*)";  break;
		default: n="?";	
	}

	labInputMode->setText(n);
}

void zChatForm::changeSelect()
{
	/*
	if ( mleMes->hasFocus() )
	{
		eeChat->selMesIndex = 0;
		eeChat->setFocus();
	} else
	{
		mleMes->setFocus();
	}
	*/
}

bool zChatForm::eventFilter(QObject* o, QEvent* pEvent)
{

	if ( QEvent::FocusIn == pEvent->type() && o == mleMes )
	{
		kbChange();
	} else
	if (  QEvent::KeyPress == pEvent->type())
	{
		switch ( ((QKeyEvent*)pEvent)->key() )
		{
			case Z6KEY_POUND:       if (!((QKeyEvent*)pEvent)->isAutoRepeat()) kbChangeByKey();	          break;
			case Z6KEY_SIDE_UP:     eeChat->pageUp();   									return true;  break;
			case Z6KEY_SIDE_DOWN:   eeChat->pageDown(); 									return true;  break;
			case Z6KEY_SIDE_SELECT: changeSelect();     									return true;  break;
			case Z6KEY_GREEN:	if ( !cfg_sendByCenter )					  {sendClick(); return true;} break;
			case Z6KEY_CENTER:	if ( mleMes->hasFocus() && cfg_sendByCenter ) {sendClick(); return true;} break;
			#ifdef NEW_PLATFORM
			case Z6KEY_RED:
			#else
			case Z6KEY_RED:                             									return true;  break;			
			#endif
			case Z6KEY_CARRIER:
			case Z6KEY_MUSIC:	musicPress(); 												return true;  break;		
		}
	} else
	if (  QEvent::KeyRelease == pEvent->type())
	{			
		switch ( ((QKeyEvent*)pEvent)->key() )
		{		
			case Z6KEY_LSK:			if (o!=menu) {menu->popup();	return true;} break;
			case Z6KEY_RSK:			if (o!=menu) {reject();			return true;} break;
		}
	}
	
	return QWidget::eventFilter( o, pEvent ); 
}

void zChatForm::copyText()
{
	//if ( mleMes->hasFocus() )
		#ifdef CUTED_PLATFORM
		QApplication::clipboard()->setText( mleMes->text() );
		#else
		mleMes->copy();
		#endif
}

void zChatForm::pastText()
{
	//if ( mleMes->hasFocus() )
		#ifdef CUTED_PLATFORM	
		mleMes->insert( QApplication::clipboard()->text() );
		#else
		mleMes->paste();
		#endif		
}

#ifdef _XMPP
void zChatForm::slotJoinRoom()
{
	zgui->xmpp->joinRoom(uin);
}
#endif
