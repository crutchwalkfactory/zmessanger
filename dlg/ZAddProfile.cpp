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

#include "ZAddProfile.h"

#include "zDefs.h"

ZAddProfile::ZAddProfile()
    :MyBaseDlg()
{
	id = "";
	pas = "";
	edit = false;
	num = 0;
    prot = 0;

	setMainWidgetTitle(LNG_ADDPROFILE);
	
	ZFormContainer *form = new ZFormContainer(this, 0, ZSkinService::clsZFormContainer);
	setContentWidget(form);
	
	zcbProtocol = new ZComboBox(form);
	zcbProtocol->setTitle( LNG_PROTOCOL );
	QPixmap pm;
	pm.load(ProgDir+ "/status/icq/online.png");
	zcbProtocol->insertItem(pm, "ICQ", 0);
	#ifdef _XMPP
	pm.load(ProgDir+ "/status/jabber/online.png");
	zcbProtocol->insertItem(pm, "JABBER", 1);
	#endif
		
	form->addChild(zcbProtocol);
	
	zleID = new ZLineEdit("", form);
	zleID->setTitle( LNG_LOGIN );
	form->addChild ( zleID );
	
	zlePas = new ZLineEdit("", form);
	zlePas->setTitle( LNG_PASSWORD );
	setInputMethod(zlePas, ZKB_INPUT_PREDICTIVE, ZKbInputField::FIELD_TYPE_PASSWORD);
	zlePas->setEchoMode(ZLineEdit::Password);
	form->addChild ( zlePas );

	connect ( zcbProtocol, SIGNAL ( activated(int) ), this, SLOT ( changeProtocol(int) ) );
	changeProtocol(0);

	QValueList<QUuid> flist2;
	flist2.append( ZKB_INPUT_MULTITAP );
	flist2.append( ZKB_INPUT_SYMBOL );
	setInputMethods((QWidget*)zlePas, (const QUuid&)ZKB_INPUT_MULTITAP, flist2);
	
	ZSoftKey *softKey  = new ZSoftKey("CST_2", this, this);
	softKey->setText ( ZSoftKey::LEFT, LNG_ADD, ( ZSoftKey::TEXT_PRIORITY ) 0 );
	softKey->setText ( ZSoftKey::RIGHT, LNG_CANCEL, ( ZSoftKey::TEXT_PRIORITY ) 0 );
	softKey->setClickedSlot ( ZSoftKey::RIGHT, this, SLOT ( reject() ) );
	softKey->setClickedSlot ( ZSoftKey::LEFT, this, SLOT ( addProfile() ) );
	setSoftKey(softKey);
}

void ZAddProfile::setEditProfile(QString ID, QString Pas, int Num, int Prot)
{
	edit = true;
	zleID->setText(ID);
	zlePas->setText(Pas);
	num = Num;
	zcbProtocol->setCurrentItem(Prot);
}

void ZAddProfile::addProfile()
{
	pas = zlePas->text();
	id = zleID->text();
	
	ZConfig cfg(ProgDir+"/zMessanger.cfg");	
	
	QString prefID;
	QString prefPas;
	
	prot = zcbProtocol->currentItem();
	switch( prot )
	{
		case 0: //ICQ
		{
			prefID = "UIN";
			prefPas = "PAS";
			break;
		}
		case 1: //JABBER
		{
			prefID = "JID";
			prefPas = "JPAS";
			break;
		}
	}
	
	if ( !edit )
	{
		for (int i=1;i<20;i++)
		{
			if ( cfg.readEntry(QString("Login"), QString(prefID+QString::number(i)), "") == "" )
			{
				cfg.writeEntry(QString("Login"), QString(prefID+QString::number(i)), id);
				cfg.writeEntry(QString("Login"), QString(prefPas+QString::number(i)), pas);
				num = i;
				break;
			}
		}
	} else
	{
		cfg.writeEntry(QString("Login"), QString(prefID+QString::number(num)), id);
		cfg.writeEntry(QString("Login"), QString(prefPas+QString::number(num)), pas);		
	}
	cfg.flush();
	
	MyBaseDlg::accept();
}

void ZAddProfile::changeProtocol(int n)
{
	QValueList<QUuid> flist1;
	flist1.append( ZKB_INPUT_NUMERIC );

	QValueList<QUuid> flist2;
	flist2.append( ZKB_INPUT_PREDICTIVE );
	flist2.append( ZKB_INPUT_SYMBOL );
	
	switch(n)
	{
		case 0: //ICQ
		{
			setInputMethods((QWidget*)zleID, (const QUuid&)ZKB_INPUT_NUMERIC, flist1);
			break;
		}
		case 1: //JABBER
		{
			setInputMethods((QWidget*)zleID, (const QUuid&)ZKB_INPUT_PREDICTIVE, flist2);
			break;
		}
	}
}
