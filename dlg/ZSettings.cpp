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

#include "ZSettings.h"

#include "ZXStatusText.h"
#include <qlabel.h>
#include <ZApplication.h>
#include <ZSoftKey.h>
#include <qtextcodec.h>
#include "config.h"

#include "const_strings.h"

#include "zEmoticon.h"
#include "zgui.h"
#include "zDefs.h"

ZSettingsDlg::ZSettingsDlg()
    :MyBaseDlg()
{
	setMainWidgetTitle(LNG_SETTINGS);

	myWidget = new ZWidget ();

	tabWidget = new ZNavTabWidget( myWidget );
	setContentWidget ( tabWidget );

	QPixmap pm;

	ZConfig cfg ( ProgDir+"/zMessanger.cfg");
	//##################################   1   ############################################
	net = new ZListBox( tabWidget );
	pm.load( ProgDir+ "/image/tab_net.png");
	tabWidget->addTab(net, QIconSet(pm), "");
	//#####################################################################################
	optInetLink = new ZOptionItem(net, ZOptionItem::EDIT_INTERNET_PROFILE);
	optInetLink->setText( cfg.readEntry(QString("Inet"), QString("ProfileName"), "") );
	optInetLink->setTitle(LNG_INETPROF);
	net->insertItem(optInetLink);
	
	optICQServer = new ZOptionItem(net, ZOptionItem::EDIT_TEXT);
	optICQServer->setText( cfg.readEntry(QString("Inet"), QString("Server"), "login.icq.com") );
	optICQServer->setTitle(LNG_ICQSERV);
	net->insertItem(optICQServer);
	
	optICQPort = new ZOptionItem(net, ZOptionItem::EDIT_TEXT);
	optICQPort->setText( cfg.readEntry(QString("Inet"), QString("Port"), "5190") );
	optICQPort->setTitle(LNG_PORT);
	net->insertItem(optICQPort);
	
	optICQKeepTime = new ZOptionItem(net, ZOptionItem::EDIT_TEXT);
	optICQKeepTime->setText( cfg.readEntry(QString("Inet"), QString("TimeKeepConnect"), "500") );
	optICQKeepTime->setTitle(LNG_KEEPCONNECTTIME);
	net->insertItem(optICQKeepTime);
	
	optICQdelayReadSnec = new ZOptionItem(net, ZOptionItem::EDIT_TEXT);
	optICQdelayReadSnec->setText( cfg.readEntry(QString("Inet"), QString("delayReadSnec"), "2000") );
	optICQdelayReadSnec->setTitle(LNG_DELAYNEWSOCKREAD);
	net->insertItem(optICQdelayReadSnec);
	
	#ifndef _NoUSBNetOption
	optUSBNet = new ZOptionItem(net, ZOptionItem::EDIT_BOOL_ONOFF);
	optUSBNet->setTitle( "Internet on USB" );
	optUSBNet->setNum( cfg.readBoolEntry("Inet", "inetOnUSB", false) );
	net->insertItem(optUSBNet);
	#endif
	//#######################################  2  #########################################
	alert = new ZListBox( tabWidget );
	pm.load( ProgDir + "/image/tab_alert.png");
	tabWidget->addTab(alert, QIconSet(pm), "");
	//#####################################################################################
	
	optVibrate = new ZOptionItem(alert, ZOptionItem::EDIT_BOOL_ONOFF);
	optVibrate->setTitle( LNG_VIBRATE );
	optVibrate->setNum( cfg.readBoolEntry("Alert", "Vibrate", true) );
	alert->insertItem(optVibrate);
	
	optTone = new ZOptionItem(alert, ZOptionItem::EDIT_BOOL_ONOFF);
	optTone->setTitle( LNG_RINGTONE );
	optTone->setNum( cfg.readBoolEntry("Alert", "Ring", false) );
	alert->insertItem(optTone);
	
	optTonePath = new ZOptionItem(alert, ZOptionItem::EDIT_FILE);
	optTonePath->setText( cfg.readEntry(QString("Alert"), QString("Path"), "") );
	optTonePath->setTitle( LNG_RINGTONEPATH );
	alert->insertItem(optTonePath);

	//#########################################  3  #######################################
	CL = new ZListBox( tabWidget );
	pm.load( ProgDir+ "/image/tab_CL.png");
	tabWidget->addTab(CL, QIconSet(pm), "");
	//#####################################################################################
	optShowOffLine = new ZOptionItem(CL, ZOptionItem::EDIT_BOOL_YESNO);
	optShowOffLine->setTitle( LNG_SHOWOFFCONT );
	optShowOffLine->setNum( !cfg.readBoolEntry(QString("ContactList"), QString("dontShowOffLine"), true) );
	CL->insertItem(optShowOffLine);
	
	optShowGroup = new ZOptionItem(CL, ZOptionItem::EDIT_BOOL_YESNO);
	optShowGroup->setTitle( LNG_SHOWGROUP );
	optShowGroup->setNum( !cfg.readBoolEntry(QString("ContactList"), QString("dontShowGroup"), false) );
	CL->insertItem(optShowGroup);
	
	optRigthXStatus = new ZOptionItem(CL, ZOptionItem::EDIT_BOOL_YESNO);
	optRigthXStatus->setTitle( LNG_XSTATUSRIGTH );
	optRigthXStatus->setNum( cfg.readBoolEntry(QString("ContactList"), QString("rigthAlignXStatus"), true) );
	CL->insertItem(optRigthXStatus);
	
	QStringList modes;
	modes.append(LNG_NO_SORTCONTACT);
	modes.append(LNG_STATUS_SORTCONTACT);
	modes.append(LNG_STATUS_NICK_SORTCONTACT);
	
	optSortType = new ZOptionItem(CL, ZOptionItem::EDIT_ONE_OF_LIST);
	optSortType->setTitle(LNG_SORTCONTACT);
	optSortType->setList( &modes );
	optSortType->setNum( cfg.readNumEntry(QString("ContactList"), QString("sortType"), 2) );
	CL->insertItem(optSortType);

	//#######################################  4  ##########################################
	Chat = new ZListBox( tabWidget );
	pm.load( ProgDir+ "/image/tab_chat.png");
	tabWidget->addTab(Chat, QIconSet(pm), "");
	//#####################################################################################
	optChatFontSize = new ZOptionItem(Chat, ZOptionItem::EDIT_NUM);
	optChatFontSize->setMaxMin(35, 2);
	optChatFontSize->setNum( cfg.readNumEntry(QString("Chat"), QString("chatFontSize"), 11) );
	optChatFontSize->setTitle( LNG_FONTSIZECHAT );
	Chat->insertItem(optChatFontSize);
	
	optMesFontSize = new ZOptionItem(Chat, ZOptionItem::EDIT_NUM);
	optMesFontSize->setMaxMin(35, 2);
	optMesFontSize->setNum( cfg.readNumEntry(QString("Chat"), QString("writeMesFontSize"), 11) );
	optMesFontSize->setTitle( LNG_FONTSIZEMES );
	Chat->insertItem(optMesFontSize);
	
	optMaxNumLine = new ZOptionItem(Chat, ZOptionItem::EDIT_NUM);
	optMaxNumLine->setMaxMin(500, 3);
	optMaxNumLine->setNum( cfg.readNumEntry(QString("Chat"), QString("maxNumLines"), 40) );
	optMaxNumLine->setTitle( LNG_MAXNUMLINESCHAT );
	Chat->insertItem(optMaxNumLine);	
	
	optSendByCenter = new ZOptionItem(Chat, ZOptionItem::EDIT_BOOL_YESNO);
	optSendByCenter->setTitle( LNG_SEND_BY_CENTER );
	optSendByCenter->setNum( cfg.readBoolEntry(QString("Chat"), QString("sendByCenter"), false) );
	Chat->insertItem(optSendByCenter);

	optSendTypeMes = new ZOptionItem(Chat, ZOptionItem::EDIT_BOOL_YESNO);
	optSendTypeMes->setTitle( LNG_SENDTYPEMES );
	optSendTypeMes->setNum( !cfg.readBoolEntry(QString("Chat"), QString("notSendTypeMes"), false) );
	Chat->insertItem(optSendTypeMes);	
	
	//#####################################  5  ###########################################
	othe = new ZListBox( tabWidget );
	pm.load( ProgDir+ "/image/tab_othe.png");
	tabWidget->addTab(othe, QIconSet(pm), "");
	//#####################################################################################
	optCodePage = new ZOptionItem(othe, ZOptionItem::EDIT_TEXT);
	optCodePage->setText( cfg.readEntry(QString("Message"), QString("CodePage"), "CP1251") );
	optCodePage->setTitle( LNG_DEFCP );
	othe->insertItem(optCodePage);
	
	optXTrazRequest = new ZOptionItem(othe, ZOptionItem::EDIT_BOOL_YESNO);
	optXTrazRequest->setTitle( LNG_XSTATUSAUTOREQUEST );
	optXTrazRequest->setNum( !cfg.readBoolEntry(QString("Message"), QString("noAutoXTrazRequest"), true) );
	othe->insertItem(optXTrazRequest);
	
	optAutoMsgRequest = new ZOptionItem(othe, ZOptionItem::EDIT_BOOL_YESNO);
	optAutoMsgRequest->setTitle( LNG_STATUSAUTOREQUEST );
	optAutoMsgRequest->setNum( !cfg.readBoolEntry(QString("Message"), QString("noAutoMsgRequest"), true) );
	othe->insertItem(optAutoMsgRequest);
		
	optSaveStusWithExit = new ZOptionItem(othe, ZOptionItem::EDIT_BOOL_ONOFF);
	optSaveStusWithExit->setTitle( LNG_SAVE_STATUS_WITH_EXIT );
	optSaveStusWithExit->setNum( cfg.readBoolEntry(QString("Status"), QString("saveStausCanDisconect"), true) );
	othe->insertItem(optSaveStusWithExit);

	optEye = new ZOptionItem(othe, ZOptionItem::EDIT_BOOL_ONOFF);
	optEye->setTitle( LNG_EYE );
	optEye->setNum(cfg.readBoolEntry(QString("Othe"), QString("enabledEye"), true));
	othe->insertItem(optEye);
	//#####################################################################################
	
	net->setFocus(); // set focus to widget in first tab
	
	ZSoftKey *softKey = new ZSoftKey ( NULL, this, this );
	softKey->setText ( ZSoftKey::LEFT, LNG_OK, ( ZSoftKey::TEXT_PRIORITY ) 0 );
	softKey->setText ( ZSoftKey::RIGHT, LNG_CANCEL, ( ZSoftKey::TEXT_PRIORITY ) 0 );
	softKey->setClickedSlot ( ZSoftKey::RIGHT, this, SLOT ( reject() ) );
	softKey->setClickedSlot ( ZSoftKey::LEFT, this, SLOT ( saveSetting() ) );
	setCSTWidget ( softKey );
}

ZSettingsDlg::~ZSettingsDlg()
{
	delete myWidget;
}

void ZSettingsDlg::saveSetting()
{
	ZConfig cfg( ProgDir+"/zMessanger.cfg");

	cfg.writeEntry(QString("Inet"), QString("ProfileName"), optInetLink->getText());
	cfg.writeEntry(QString("Inet"), QString("Server"), optICQServer->getText());
	cfg.writeEntry(QString("Inet"), QString("Port"), optICQPort->getText());
	cfg.writeEntry(QString("Inet"), QString("TimeKeepConnect"), optICQKeepTime->getText());
	cfg.writeEntry(QString("Inet"), QString("delayReadSnec"), optICQdelayReadSnec->getText());
	#ifndef _NoUSBNetOption
	cfg.writeEntry(QString("Inet"), QString("inetOnUSB"), optUSBNet->getNum());
	#endif
	
	cfg.writeEntry("Alert", "Vibrate", optVibrate->getNum());
	cfg.writeEntry("Alert", "Ring", optTone->getNum());
	cfg.writeEntry(QString("Alert"), QString("Path"), optTonePath->getText());
	
	cfg.writeEntry(QString("ContactList"), QString("sortType"), optSortType->getNum());
	cfg.writeEntry(QString("ContactList"), QString("dontShowOffLine"), !optShowOffLine->getNum());
	cfg.writeEntry(QString("ContactList"), QString("dontShowGroup"), !optShowGroup->getNum());
	cfg.writeEntry(QString("ContactList"), QString("rigthAlignXStatus"), optRigthXStatus->getNum());
	
	cfg.writeEntry(QString("Chat"), QString("chatFontSize"), optChatFontSize->getNum());
	cfg.writeEntry(QString("Chat"), QString("writeMesFontSize"), optMesFontSize->getNum());
	cfg.writeEntry(QString("Chat"), QString("maxNumLines"), optMaxNumLine->getNum());

	cfg.writeEntry(QString("Message"), QString("CodePage"), optCodePage->getText());
	cfg.writeEntry(QString("Message"), QString("noAutoXTrazRequest"),!optXTrazRequest->getNum());
	cfg.writeEntry(QString("Message"), QString("noAutoMsgRequest"), !optAutoMsgRequest->getNum());
	cfg.writeEntry(QString("Othe"), QString("enabledEye"), optEye->getNum());
	cfg.writeEntry(QString("Chat"), QString("notSendTypeMes"), !optSendTypeMes->getNum());
	cfg.writeEntry(QString("Chat"), QString("sendByCenter"), optSendByCenter->getNum());

	cfg.writeEntry(QString("Status"), QString("saveStausCanDisconect"), optSaveStusWithExit->getNum());
	cfg.flush();

	cfg_inetProfile = optInetLink->getText();
	zgui->icq->setLoginHost(optICQServer->getText().latin1(), optICQPort->getText().toInt());
	cfg_timeKeepConnect = optICQKeepTime->getText().toInt();
	zgui->icq->delayReadSnec = optICQdelayReadSnec->getText().toInt();
	#ifndef _NoUSBNetOption
	cfg_InetOnUSB = optUSBNet->getNum();
	#endif
	
	zgui->icq->noAutoXTrazRequest = !optXTrazRequest->getNum();
	zgui->icq->noAutoMsgRequest = !optAutoMsgRequest->getNum();
	zgui->icq->enabledEye = optEye->getNum();
	
	cfg_alertVibr = optVibrate->getNum();
	cfg_alertRing = optTone->getNum();
	cfg_dontShowGroup = !optShowGroup->getNum();
	cfg_rigthAlignXStatus = optRigthXStatus->getNum();
	cfg_notSendTypeMes = !optSendTypeMes->getNum();
	cfg_sendByCenter = optSendByCenter->getNum();

	cfg_alertPath = optTonePath->getText();
	cfg_sortType = optSortType->getNum();
	cfg_maxNumLines = optMaxNumLine->getNum();
	cfg_mesFontSize = optMesFontSize->getNum();
	cfg_chatFontSize = optChatFontSize->getNum();

	zgui->codec = QTextCodec::codecForName( optCodePage->getText() );
	if ( zgui->codec == 0 )
		zgui->codec = QTextCodec::codecForName( "CP1251" );
	
	accept();
}
