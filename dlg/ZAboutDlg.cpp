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

#include "ZAboutDlg.h"
#include "config.h"

#include <qlabel.h>
#include <qfont.h>

#include <ZApplication.h>
#include <ZSoftKey.h>
#include <ZLabel.h>
#include <ZImage.h>
#include <ZSeparator.h>

#include "zDefs.h"

ZAboutDialog::ZAboutDialog()
    :MyBaseDlg()
{
	setMainWidgetTitle(LNG_ABOUT);
	
	myWidget = new ZWidget ();
	tabWidget = new ZNavTabWidget( myWidget );
	connect(tabWidget,SIGNAL(currentChanged(QWidget* )),this,SLOT(slotPageChanged(QWidget* )));
	
	sv = new ZScrollPanel(this);
	
	ZImage *img = new ZImage( this );
	img->setPixmap ( QPixmap( ProgDir + "/zMessanger_usr.png" ) );
	sv->addChild(img, 10,20);

	QString model;
	
	#if defined(EZX_Z6)
	model = "Z6";
	#elif defined(EZX_ZN5)
	model = "ZN5";
	#elif defined(EZX_U9)
	model = "U9";
	#elif defined(EZX_E8)
	model = "E8";
	#elif defined(EZX_EM30)
	model = "EM30";
	#elif defined(EZX_EM35)
	model = "EM35";
	#elif defined(EZX_VE66)
	model = "VE66";
	#elif defined(EZX_V8)
	model = "V8";
	#else
	model = "";
	#endif

	ZLabel * a = new ZLabel("zMessanger "+model,this);
	QFont font ( a->font() );
	font.setBold ( true );
	a->setFont(font);
	a->setFixedWidth(SCREEN_WIDTH-80);
	a->setAlignment(ZLabel::AlignCenter);
	sv->addChild(a, 80,15);
	QString ver = QString::number(VER_MAJOR)+"."+QString::number(VER_MINOR);

	switch (VER_STATUS)
	{
		case 0: ver = ver+" PreAlpha"; break;
		case 1: ver = ver+" Alpha";    break;
		case 2: ver = ver+" Betta";    break;
		case 3: ver = ver+" RC";       break;
		case 4: ver = ver+" Stable";   break;
	}

	a = new ZLabel(QString("ver. ")+ver,this);
	a->setFixedWidth(SCREEN_WIDTH-80);
	a->setAlignment(ZLabel::AlignCenter);
	sv->addChild(a, 80,45);		
	
	ZSeparator * sep = new ZSeparator();
	sep->setLength(SCREEN_WIDTH-20);
	sv->addChild(sep, 10,90);	
	
	a = new ZLabel("Copyrigth "+LNG_COPYRIGHT+": Ant-ON, 2010",this);
	a->setAutoResize(true);
	a->setPreferredWidth(SCREEN_WIDTH-10);
	sv->addChild(a, 15,100);
	
	a = new ZLabel("e-mail: prozanton@gmail.com",this);
	a->setAutoResize(true);
	a->setPreferredWidth(SCREEN_WIDTH-10);
	sv->addChild(a, 15,130);
	
	sep = new ZSeparator();
	sep->setLength(SCREEN_WIDTH-20);
	sv->addChild(sep, 10,190);			

	phoneInfo = new ZListBox( tabWidget );
	
	font.setPointSize ( 16 );
	phoneInfo->setItemFont (ZListBox::LISTITEM_REGION_A, ZSkinBase::StStandard, font );
	phoneInfo->setItemFont (ZListBox::LISTITEM_REGION_A, ZSkinBase::StHighlightSelected, font );	
	phoneInfo->setItemFont (ZListBox::LISTITEM_REGION_A, ZSkinBase::StSelected, font );	
	phoneInfo->setItemFont (ZListBox::LISTITEM_REGION_A, ZSkinBase::StHighlighted, font );	
	
	ZSettingItem* item;

	item = new ZSettingItem(phoneInfo, "%M");
	item->appendSubItem(0, "zIM "+model+" "+ver, false);
	phoneInfo->insertItem( item );

	item = new ZSettingItem(phoneInfo, "%M");
	item->appendSubItem(0, "Traffic", true);
    item->setSeparator("-",NULL);
	item->setSelectable(false);	
	phoneInfo->insertItem( item, -1, false );

	item = new ZSettingItem(phoneInfo, "%M");
	item->appendSubItem(0, "IN:", true);
	item->appendSubItem(0, QString::number((double)trafIN/1024, 'g', 2)+" KB" );
	phoneInfo->insertItem( item );
	
	item = new ZSettingItem(phoneInfo, "%M");
	item->appendSubItem(0, "OUT:", true);
	item->appendSubItem(0, QString::number((double)trafOUT/1024, 'g', 2)+" KB" );
	phoneInfo->insertItem( item );
	
	item = new ZSettingItem(phoneInfo, "%M");
	item->appendSubItem(0, "Build", true);
    item->setSeparator("-",NULL);
	item->setSelectable(false);	
	phoneInfo->insertItem( item, -1, false );

	item = new ZSettingItem(phoneInfo, "%M");
	item->appendSubItem(0, "Build number:", false);
	item->appendSubItem(0, QString::number( BUILD_NUMBER ), false );
	phoneInfo->insertItem( item );
	
	item = new ZSettingItem(phoneInfo, "%M");
	item->appendSubItem(0, "Buld date:", false);
	item->appendSubItem(0, QString( BUILD_DATE ), false );
	phoneInfo->insertItem( item );	
	
	phoneInfo->setCurrentItem(1);
	
	QPixmap pm;
	pm.load( ProgDir+ "/image/tab_about.png");
	tabWidget->addTab(sv, QIconSet(pm), "");	
	pm.load( ProgDir+ "/image/tab_othe_info.png");
	tabWidget->addTab(phoneInfo, QIconSet(pm), "");		

	setContentWidget ( tabWidget );  
	
	ZSoftKey *softKey = new ZSoftKey ( NULL, this, this );
	softKey->setText ( ZSoftKey::LEFT, LNG_OK, ( ZSoftKey::TEXT_PRIORITY ) 0 );
	softKey->setClickedSlot ( ZSoftKey::LEFT, this, SLOT ( accept() ) );
	setCSTWidget ( softKey );
}

ZAboutDialog::~ZAboutDialog()
{
	delete myWidget;
}

void ZAboutDialog::slotPageChanged(QWidget* )
{
	int i = tabWidget->currentPageIndex();
	
	switch (i)
	{
		case 0:
		{
			sv->setFocus();
			break;
		}
		case 1:
		{
			phoneInfo->setFocus();
			break;
		}
	}
}

