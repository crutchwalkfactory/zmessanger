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

#include "ZEye.h"

#include "ZXStatusText.h"
#include <qlabel.h>
#include <ZApplication.h>
#include <ZSoftKey.h>

#include "zEmoticon.h"
#include "zgui.h"
#include "zDefs.h"

ZEyeDialog::ZEyeDialog()
    :MyBaseDlg()
{
	softKey = new ZSoftKey ( NULL, this, this );
	softKey->setText ( ZSoftKey::LEFT, LNG_OK, ( ZSoftKey::TEXT_PRIORITY ) 0 );
	softKey->setClickedSlot ( ZSoftKey::LEFT, this, SLOT ( reject() ) );
	setCSTWidget ( softKey );

	setMainWidgetTitle(LNG_EYE);

	lbList = new ZListBox ( QString ( "%M\n%M" ), this, 0);
	lbList->setFixedWidth ( SCREEN_WIDTH ); 

	QFont font ( qApp->font() );
	font.setPointSize ( 14 );
	
	lbList->setItemFont (ZListBox::LISTITEM_REGION_C, font, true );
	lbList->setItemFont (ZListBox::LISTITEM_REGION_C, font, false );

	QString title;
	QString action;
	for (int i=0;i<zgui->icq->listEye.size();i++)
	{
		ZSettingItem* listitem = new ZSettingItem(lbList, QString("%M") );
		title = zgui->icq->listEye[i].time+ " [";
		if (zgui->icq->listEye[i].nick != "" )
		{
			title = title+zgui->strtoqstr( zgui->icq->listEye[i].nick.c_str() ) + "(" + QString( zgui->icq->listEye[i].uin.c_str() ) + ")]";
		} else
		{
			title = title+QString( zgui->icq->listEye[i].uin.c_str() )+"]";
		}
		switch ( zgui->icq->listEye[i].type )
		{
			case 0: action = "Unknown"; break;
			case 1: action = "Read xStatus"; break;
			case 2: action = "Read Status"; break;
			case 3: action = "Invisible???"; break;
			default: action = "Unknown";
		}
		listitem->appendSubItem ( 0, title , true );
		listitem->appendSubItem ( 0, action , true );
		lbList->insertItem ( listitem, -1, true );
	}

	setContentWidget ( lbList );

	lbList->setFocus();
	lbList->setSelected(lbList->count()-1, true);
}


ZEyeDialog::~ZEyeDialog()
{
}
