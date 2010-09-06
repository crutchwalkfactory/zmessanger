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
#include "config.h"

#include "const_strings.h"

#include "zEmoticon.h"
#include "zgui.h"
#include "zDefs.h"

#include "qdir.h"

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
	
	optICQPort = new ZOptionItem(net, ZOptionItem::EDIT_TEXT_NUM);
	optICQPort->setText( cfg.readEntry(QString("Inet"), QString("Port"), "5190") );
	optICQPort->setTitle(LNG_PORT);
	net->insertItem(optICQPort);
	
	optICQKeepTime = new ZOptionItem(net, ZOptionItem::EDIT_TEXT_NUM);
	optICQKeepTime->setText( cfg.readEntry(QString("Inet"), QString("TimeKeepConnect"), "500") );
	optICQKeepTime->setTitle(LNG_KEEPCONNECTTIME);
	net->insertItem(optICQKeepTime);
	
	optICQdelayReadSnec = new ZOptionItem(net, ZOptionItem::EDIT_TEXT_NUM);
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
	
	optToneVol = new ZOptionItem(alert, ZOptionItem::EDIT_NUM);
	optToneVol->setTitle( LNG_RINGTONEVOL );
	optToneVol->setNum( cfg.readNumEntry("Alert", "Volume", 2) );
	optToneVol->setMaxMin(10, 0);
	alert->insertItem(optToneVol);	
	
	optTonePath = new ZOptionItem(alert, ZOptionItem::EDIT_ONE_OF_LIST);
	optTonePath->setTitle( LNG_RINGTONEPATH );
	
	QDir dirTone ( ProgDir+"/alert/", QString("*.mp3 *.wav") );
	dirTone.setFilter ( QDir::Files | QDir::Hidden );
	toneList = new QStringList( dirTone.entryList() );
	toneList->sort();

	int sel=0, i=0;
	QString text = cfg.readEntry(QString("Alert"), QString("Path"), "msn.mp3");
	for ( QStringList::Iterator it = toneList->begin(); it != toneList->end(); ++it )
		if ( *it == text )
		{
			sel=i;
			break;
		} else
			i++;
	
	optTonePath->setList( toneList );
	optTonePath->setNum( sel );
	
	alert->insertItem(optTonePath);

	//#########################################  3  #######################################
	CL = new ZListBox( tabWidget );
	pm.load( ProgDir+ "/image/tab_CL.png");
	tabWidget->addTab(CL, QIconSet(pm), "");
	//#####################################################################################
	optJampNewMes = new ZOptionItem(CL, ZOptionItem::EDIT_BOOL_YESNO);
	optJampNewMes->setTitle( LNG_AUTO_JAMP_NEW_MESSAGE );
	optJampNewMes->setNum( cfg.readBoolEntry(QString("ContactList"), QString("jampNewMes"), true) );
	CL->insertItem(optJampNewMes);	
	
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
	
	QStringList * modes = new QStringList();
	modes->append(LNG_NO_SORTCONTACT);
	modes->append(LNG_STATUS_SORTCONTACT);
	modes->append(LNG_STATUS_NICK_SORTCONTACT);
	
	optSortType = new ZOptionItem(CL, ZOptionItem::EDIT_ONE_OF_LIST);
	optSortType->setTitle(LNG_SORTCONTACT);
	optSortType->setList( modes );
	optSortType->setNum( cfg.readNumEntry(QString("ContactList"), QString("sortType"), 2) );
	CL->insertItem(optSortType);

	//#######################################  4  ##########################################
	Chat = new ZListBox( tabWidget );
	pm.load( ProgDir+ "/image/tab_chat.png");
	tabWidget->addTab(Chat, QIconSet(pm), "");
	//#####################################################################################
	
	optSmilePack = new ZOptionItem(Chat, ZOptionItem::EDIT_ONE_OF_LIST);
	optSmilePack->setTitle( LNG_SMILE_PACK );
	
	smileList = zSmile->getSmilePackList();
	
	optSmilePack->setList( smileList );
	
	sel=0; i=0;
	text = cfg.readEntry(QString("Chat"), QString("smilePack"), "kolobok");
	for ( QStringList::Iterator it = smileList->begin(); it != smileList->end(); ++it )
		if ( *it == text )
		{
			sel=i;
			break;
		} else
			i++;	
	
	optSmilePack->setNum( sel );
	Chat->insertItem(optSmilePack);	
	
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
	
	optNoShowStatusBarInChat = new ZOptionItem(Chat, ZOptionItem::EDIT_BOOL_ONOFF);
	optNoShowStatusBarInChat->setTitle( LNG_NO_SHOW_STATUS_BAR_IN_CHAT );
	optNoShowStatusBarInChat->setNum( !cfg.readBoolEntry(QString("Chat"), QString("noShowStatusBarInChat"), false) );
	Chat->insertItem(optNoShowStatusBarInChat);	
	
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
	
	codec = new QStringList();
	codec->append( "KOI8-R" );
	codec->append( "KOI8-U" );
	codec->append( "ISO8859-2" );
	codec->append( "ISO8859-3" );
	codec->append( "ISO8859-4" );
	codec->append( "ISO8859-5" );
	codec->append( "ISO8859-6-I" );
	codec->append( "ISO8859-7" );
	codec->append( "ISO8859-8-I" );
	codec->append( "ISO8859-9" );
	codec->append( "ISO8859-10" );
	codec->append( "ISO8859-11" );		
	codec->append( "ISO8859-13" );
	codec->append( "ISO8859-14" );
	codec->append( "ISO8859-15" );
	codec->append( "PT154" );
	codec->append( "CP874" );
	codec->append( "CP1250" );
	codec->append( "CP1251" );
	codec->append( "CP1252" );
	codec->append( "CP1253" );
	codec->append( "CP1254" );
	codec->append( "CP1255" );
	codec->append( "CP1256" );	
	codec->append( "CP1257" );	
	codec->append( "CP1258" );

	sel=0, i=0;
	text = cfg.readEntry(QString("Message"), QString("CodePage"), "CP1251");
	for ( QStringList::Iterator it = codec->begin(); it != codec->end(); ++it )
		if ( *it == text )
		{
			sel=i;
			break;
		} else
			i++;
	optCodePage = new ZOptionItem(othe, ZOptionItem::EDIT_ONE_OF_LIST);
	optCodePage->setTitle( LNG_DEFCP );	
	optCodePage->setList( codec );
	optCodePage->setNum( sel );
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
	cfg.writeEntry("Alert", "Volume", optToneVol->getNum() );
	cfg.writeEntry(QString("Alert"), QString("Path"), *(toneList->at(optTonePath->getNum())) );
		
	cfg.writeEntry(QString("ContactList"), QString("jampNewMes"), optJampNewMes->getNum() );
	cfg.writeEntry(QString("ContactList"), QString("sortType"), optSortType->getNum());
	cfg.writeEntry(QString("ContactList"), QString("dontShowOffLine"), !optShowOffLine->getNum());
	cfg.writeEntry(QString("ContactList"), QString("dontShowGroup"), !optShowGroup->getNum());
	cfg.writeEntry(QString("ContactList"), QString("rigthAlignXStatus"), optRigthXStatus->getNum());
	
	cfg.writeEntry(QString("Chat"), QString("chatFontSize"), optChatFontSize->getNum());
	cfg.writeEntry(QString("Chat"), QString("writeMesFontSize"), optMesFontSize->getNum());
	cfg.writeEntry(QString("Chat"), QString("maxNumLines"), optMaxNumLine->getNum());
	cfg.writeEntry(QString("Chat"), QString("smilePack"), *(smileList->at(optSmilePack->getNum())));

	cfg.writeEntry(QString("Message"), QString("CodePage"), *(codec->at(optCodePage->getNum())));
	cfg.writeEntry(QString("Message"), QString("noAutoXTrazRequest"),!optXTrazRequest->getNum());
	cfg.writeEntry(QString("Message"), QString("noAutoMsgRequest"), !optAutoMsgRequest->getNum());
	cfg.writeEntry(QString("Othe"), QString("enabledEye"), optEye->getNum());
	cfg.writeEntry(QString("Chat"), QString("notSendTypeMes"), !optSendTypeMes->getNum());
	cfg.writeEntry(QString("Chat"), QString("sendByCenter"), optSendByCenter->getNum());
	cfg.writeEntry(QString("Chat"), QString("noShowStatusBarInChat"), !optNoShowStatusBarInChat->getNum());
	

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
	cfg_alertRingVol = optToneVol->getNum();
	
	zgui->lbContact->setJampToNewMes( optJampNewMes->getNum() );
	cfg_dontShowGroup = !optShowGroup->getNum();
	cfg_rigthAlignXStatus = optRigthXStatus->getNum();
	cfg_notSendTypeMes = !optSendTypeMes->getNum();
	cfg_sendByCenter = optSendByCenter->getNum();

	cfg_alertPath = *(toneList->at(optTonePath->getNum()));
	zgui->lbContact->setSortType( (ZMyListBox::SORT_TYPE)optSortType->getNum() );
	cfg_maxNumLines = optMaxNumLine->getNum();
	cfg_mesFontSize = optMesFontSize->getNum();
	cfg_chatFontSize = optChatFontSize->getNum();
	cfg_noShowStatusBarInChat = !optNoShowStatusBarInChat->getNum();

	zgui->codec = QTextCodec::codecForName( *(codec->at(optCodePage->getNum())) );
	if ( zgui->codec == 0 )
		zgui->codec = QTextCodec::codecForName( "CP1251" );
	
	zSmile->setSmilePack(*(smileList->at(optSmilePack->getNum())));
	
	accept();
}
