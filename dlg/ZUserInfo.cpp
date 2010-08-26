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

#include "BaseDlg.h"
#include "ZUserInfo.h"
#include "config.h"

#include <ZLabel.h>
#include <ZApplication.h>
#include <ZSoftKey.h>
#include <ZImage.h>
#include <ZLabel.h>
#include <ZSeparator.h>
#include <qrect.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <ZSkinBase.h>
#include <ZDetailView.h>

#include "const_strings.h"

#include "zgui.h"
#include "zDefs.h"

void * getInfo(void *);
void * updateInfo(void *);

ZUserInfo * dlgUserInfo;

ZUserInfo::ZUserInfo(std::string _id)
    :MyBaseDlg()
{
 	setMainWidgetTitle ( LNG_USERINFO );
	id = _id;
	dlgUserInfo = this;

	initInterface();

	ZSoftKey *softKey = new ZSoftKey ( NULL, this, this );
	softKey->setText ( ZSoftKey::LEFT, LNG_OK, ( ZSoftKey::TEXT_PRIORITY ) 0 );
	softKey->setClickedSlot ( ZSoftKey::LEFT, this, SLOT ( menu_close() ) );
	// TODO: FIX - Add update button
	//softKey->setText ( ZSoftKey::RIGHT, LNG_UPD, ( ZSoftKey::TEXT_PRIORITY ) 0 );
	//softKey->setClickedSlot ( ZSoftKey::RIGHT, this, SLOT ( menu_update() ) );	
	setCSTWidget ( softKey );

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&thread, &attr, getInfo, (void*) &id );
	pthread_attr_destroy(&attr);
}

void ZUserInfo::initInterface()
{
	logMes_2("ZUserInfo->initInterface: start");
	QFont font(qApp->font());
	font.setPointSize(14);

	QPixmap pm;

	myWidget = new ZWidget();
	tabWidget = new ZNavTabWidget(ZNavTabWidget::NEIGHBOR, false, false, myWidget);
	tabWidget->setFocusPolicy(ZNavTabWidget::NoFocus);
	tabWidget->setTabPosition(ZNavTabWidget::TOP);	
	setContentWidget ( tabWidget );

	spFull = new ZDetailView(this);
	spMain = new ZDetailView(this);	
	spHome = new ZDetailView(this);	
	spWork = new ZDetailView(this);
	tvDesc = new xTextView(this, zSmile);

	spFull->setFont(ZDetailItem::FontContent, font);
	spFull->setFont(ZDetailItem::FontTitle, font);
	spMain->setFont(ZDetailItem::FontContent, font);
	spMain->setFont(ZDetailItem::FontTitle, font);
	spHome->setFont(ZDetailItem::FontContent, font);
	spHome->setFont(ZDetailItem::FontTitle, font);
	spWork->setFont(ZDetailItem::FontContent, font);
	spWork->setFont(ZDetailItem::FontTitle, font);

	pm.load(ProgDir+ "/image/tab_full.png");
	tabWidget->addTab(spFull, QIconSet(pm), "");
	pm.load(ProgDir+ "/image/tab_main.png");
	tabWidget->addTab(spMain, QIconSet(pm), "");
	pm.load(ProgDir+ "/image/tab_home.png");
	tabWidget->addTab(spHome, QIconSet(pm), "");
	pm.load(ProgDir+ "/image/tab_work.png");
	tabWidget->addTab(spWork, QIconSet(pm), "");
	pm.load(ProgDir+ "/image/tab_note.png");
	tabWidget->addTab(tvDesc, QIconSet(pm), "");

	tabWidget->setFocus();
	
	logMes_2("ZUserInfo->initInterface: end");
}

void ZUserInfo::menu_close()
{
	if (!pthreadExit)
	{
		pthread_cancel(thread);
		pthreadExit = true;
	}
	close();
}

void ZUserInfo::menu_update()
{
	if (!pthreadExit)
	{
		pthread_cancel(thread);
		pthreadExit = true;
	}
	
	initInterface();
			
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&thread, &attr, updateInfo, &id);
	pthread_attr_destroy(&attr);	
}

void * getInfo(void * id)
{
	dlgUserInfo->pthreadExit = false;
	ICQKidFullUserInfo uinfo;

	logMes_2("ZUserInfo: Get full usr info");
	
	string suin = *((string *)id);
	
	if (zgui->icq->getFullUserInfo(suin, uinfo, false)) 
	{
		logMes_2("ZUserInfo: Show info");
		dlgUserInfo->showInfo(uinfo);
	}
	logMes_2("ZUserInfo: End show");	
	dlgUserInfo->pthreadExit = true;
	pthread_exit(NULL);
}

void * updateInfo(void * id)
{
	dlgUserInfo->pthreadExit = false;	
	ICQKidFullUserInfo uinfo;

	logMes_2("ZUserInfo: Update full usr info");
	string suin = *((string *)id); 

	if (zgui->icq->getFullUserInfo(suin, uinfo, true)) 
	{
		logMes_2("ZUserInfo: Show info");
		dlgUserInfo->showInfo(uinfo);
	}
	logMes_2("ZUserInfo: End show");
	dlgUserInfo->pthreadExit = true;	
	pthread_exit(NULL);
}

ZUserInfo::~ZUserInfo()
{
	delete myWidget;
	myWidget = NULL;
	
	delete spFull;
	delete spMain;	
	delete spHome;		
	delete spWork;			
	delete tvDesc;
	spFull = NULL;
	spMain = NULL;
	spHome = NULL;
	spWork = NULL;
	tvDesc = NULL;	
}

void ZUserInfo::showInfo(ICQKidFullUserInfo uinfo)
{
	logMes_2("ZUserInfo->ShowInfo: run");
	init_strings_maps();
	
	ZDetailItem *detailItem;

	// Full info
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_NICK+zgui->strtoqstr(uinfo.Nickname), QChar());
	detailItem->enableStructureDataParse(false);
	spFull->addItem(detailItem);

    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_NAME+zgui->strtoqstr(uinfo.Firstname), QChar());
	detailItem->enableStructureDataParse(false);
	spFull->addItem(detailItem);

    if ( uinfo.Age != 0 )
    {	
    	detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_AGE+QString::number(uinfo.Age), QChar());
		detailItem->enableStructureDataParse(false);
		spFull->addItem(detailItem);
    }
	
    QString strGender;
    switch (uinfo.Gender)
    {
	case 0: strGender=""; break;
	case 1: strGender=LNG_GANDER1; break;
	case 2: strGender=LNG_GANDER2; break;
    }
    if ( strGender != "" )
    {
		detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_GANDER+strGender, QChar());
		detailItem->enableStructureDataParse(false);
		spFull->addItem(detailItem);
    }
	
    if (uinfo.Birthmonth != 0 && uinfo.Birthday!=0)
    {
		detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_BYRTHDAY +QString::number(uinfo.Birthday) +"."+QString::number(uinfo.Birthmonth) +"."+QString::number(uinfo.Birthyear), QChar());
		detailItem->enableStructureDataParse(false);
		spFull->addItem(detailItem);
    }
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_HOME + zgui->strtoqstr(country2string(uinfo.Homecountry)) + " "+ zgui->strtoqstr(uinfo.Homecity), QChar());
	detailItem->enableStructureDataParse(false);
	spFull->addItem(detailItem);	
		
	//Main info
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_NICK+zgui->strtoqstr(uinfo.Nickname), QChar());
	detailItem->enableStructureDataParse(false);
	spMain->addItem(detailItem);
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_NAME+zgui->strtoqstr(uinfo.Firstname), QChar());
	detailItem->enableStructureDataParse(false);
	spMain->addItem(detailItem);	
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_LASTNAME+zgui->strtoqstr(uinfo.Lastname), QChar());
	detailItem->enableStructureDataParse(false);
	spMain->addItem(detailItem);	
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, "Uin: "+zgui->strtoqstr(uinfo.Uin), QChar());
	detailItem->enableStructureDataParse(false);
	spMain->addItem(detailItem);	
	
    if ( strGender != "" )
    {
		detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_GANDER+strGender, QChar());
		detailItem->enableStructureDataParse(false);
		spMain->addItem(detailItem);
    }	
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_AGE+QString::number(uinfo.Age), QChar());
	detailItem->enableStructureDataParse(false);
	spMain->addItem(detailItem);	
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_BYRTHDAY + QString::number(uinfo.Birthday) + "." + QString::number(uinfo.Birthmonth) + "." + QString::number(uinfo.Birthyear), QChar());
	detailItem->enableStructureDataParse(false);
	spMain->addItem(detailItem);	
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, "Email: "+zgui->strtoqstr(uinfo.Email), QChar());
	detailItem->enableStructureDataParse(false);
	spMain->addItem(detailItem);	
		
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_LANGUAGE, QChar());
    QString lang = "";
    if ( uinfo.Language1 != 0)
    {
    	lang = zgui->strtoqstr(lang2string(uinfo.Language1));
    }
    if ( uinfo.Language2 != 0)
    {
    	lang = lang+"\n"+zgui->strtoqstr(lang2string(uinfo.Language2));
    }   
    if ( uinfo.Language3 != 0)
    {
    	lang = lang+"\n"+zgui->strtoqstr(lang2string(uinfo.Language3));
    }    
    if ( lang == "" )
    {
    	lang = zgui->strtoqstr(lang2string(0));
    }
    detailItem->appendTextIconRow(lang);  	
	detailItem->enableStructureDataParse(false);
	spMain->addItem(detailItem);	
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_HOMEPAGE+zgui->strtoqstr(uinfo.Homepage), QChar());
	detailItem->enableStructureDataParse(false);
	spMain->addItem(detailItem);	
	
	detailItem = new ZDetailItem(ZDetailItem::ItemTypeSeparator);
	detailItem->appendTextIconRow("ItemTypeSeparator");
	spMain->addItem(detailItem);
		
	detailItem = new ZDetailItem(ZDetailItem::ItemTypeJ);
	detailItem->appendTextIconRow(LNG_INTEREST);
	spMain->addItem(detailItem);
	 
	detailItem = new ZDetailItem(ZDetailItem::ItemTypeSeparator);
	detailItem->appendTextIconRow("ItemTypeSeparator");
	spMain->addItem(detailItem); 
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, zgui->strtoqstr(interest2string(uinfo.Interest1category)), QChar());
    detailItem->appendTextIconRow(zgui->strtoqstr(uinfo.Interest1));  	
	detailItem->enableStructureDataParse(false);
	spMain->addItem(detailItem);	

    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, zgui->strtoqstr(interest2string(uinfo.Interest2category)), QChar());
    detailItem->appendTextIconRow(zgui->strtoqstr(uinfo.Interest2));  	
	detailItem->enableStructureDataParse(false);
	spMain->addItem(detailItem);	
	
	detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, zgui->strtoqstr(interest2string(uinfo.Interest3category)), QChar());
    detailItem->appendTextIconRow(zgui->strtoqstr(uinfo.Interest3));  	
	detailItem->enableStructureDataParse(false);
	spMain->addItem(detailItem);	
	
	detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, zgui->strtoqstr(interest2string(uinfo.Interest4category)), QChar());
    detailItem->appendTextIconRow(zgui->strtoqstr(uinfo.Interest4));  	
	detailItem->enableStructureDataParse(false);
	spMain->addItem(detailItem);		

	//Home info
	detailItem = new ZDetailItem(ZDetailItem::ItemTypeJ);
	detailItem->appendTextIconRow(LNG_HOMEADRESS);
	spHome->addItem(detailItem);
	 
	detailItem = new ZDetailItem(ZDetailItem::ItemTypeSeparator);
	detailItem->appendTextIconRow("ItemTypeSeparator");
	spHome->addItem(detailItem); 	
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_COUNTRY + zgui->strtoqstr( country2string(uinfo.Homecountry) ), QChar());
	detailItem->enableStructureDataParse(false);
	spHome->addItem(detailItem);		

    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_CITY+zgui->strtoqstr(uinfo.Homecity), QChar());
	detailItem->enableStructureDataParse(false);
	spHome->addItem(detailItem);	
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_STATE+zgui->strtoqstr(uinfo.Homestate), QChar());
	detailItem->enableStructureDataParse(false);
	spHome->addItem(detailItem);
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, "Zip: "+zgui->strtoqstr(uinfo.Homezip), QChar());
	detailItem->enableStructureDataParse(false);
	spHome->addItem(detailItem);
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_ADRESS+zgui->strtoqstr(uinfo.Homeaddress), QChar());
	detailItem->enableStructureDataParse(false);
	spHome->addItem(detailItem);	
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_HOMEPHONE+zgui->strtoqstr(uinfo.Homephone), QChar());
	detailItem->enableStructureDataParse(false);
	spHome->addItem(detailItem);		
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_FAX+zgui->strtoqstr(uinfo.Homefax), QChar());
	detailItem->enableStructureDataParse(false);
	spHome->addItem(detailItem);
		
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_MOBILE+zgui->strtoqstr(uinfo.Cellphone), QChar());
	detailItem->enableStructureDataParse(false);
	spHome->addItem(detailItem);
		
	detailItem = new ZDetailItem(ZDetailItem::ItemTypeSeparator);
	detailItem->appendTextIconRow("ItemTypeSeparator");
	spHome->addItem(detailItem); 		
		
	detailItem = new ZDetailItem(ZDetailItem::ItemTypeJ);
	detailItem->appendTextIconRow(LNG_ORIGADRESS);
	spHome->addItem(detailItem);
	 
	detailItem = new ZDetailItem(ZDetailItem::ItemTypeSeparator);
	detailItem->appendTextIconRow("ItemTypeSeparator");
	spHome->addItem(detailItem); 	
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_COUNTRY + zgui->strtoqstr( country2string(uinfo.Origcountry) ), QChar());
	detailItem->enableStructureDataParse(false);
	spHome->addItem(detailItem);		

    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_CITY+zgui->strtoqstr(uinfo.Origcity), QChar());
	detailItem->enableStructureDataParse(false);
	spHome->addItem(detailItem);	
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_STATE+zgui->strtoqstr(uinfo.Origstate), QChar());
	detailItem->enableStructureDataParse(false);
	spHome->addItem(detailItem);	

	//Work info	
	detailItem = new ZDetailItem(ZDetailItem::ItemTypeJ);
	detailItem->appendTextIconRow(LNG_WORCKADRESS);
	spWork->addItem(detailItem);
	 
	detailItem = new ZDetailItem(ZDetailItem::ItemTypeSeparator);
	detailItem->appendTextIconRow("ItemTypeSeparator");
	spWork->addItem(detailItem); 	
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_COUNTRY + zgui->strtoqstr( country2string(uinfo.Workcountry) ), QChar());
	detailItem->enableStructureDataParse(false);
	spWork->addItem(detailItem);		

    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_CITY+zgui->strtoqstr(uinfo.Workcity), QChar());
	detailItem->enableStructureDataParse(false);
	spWork->addItem(detailItem);	
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_STATE+zgui->strtoqstr(uinfo.Workstate), QChar());
	detailItem->enableStructureDataParse(false);
	spWork->addItem(detailItem);
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, "Zip: "+zgui->strtoqstr(uinfo.Workzip), QChar());
	detailItem->enableStructureDataParse(false);
	spWork->addItem(detailItem);
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_ADRESS+zgui->strtoqstr(uinfo.Workaddress), QChar());
	detailItem->enableStructureDataParse(false);
	spWork->addItem(detailItem);
		
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_STREET+zgui->strtoqstr(uinfo.Workposition), QChar());
	detailItem->enableStructureDataParse(false);
	spWork->addItem(detailItem);	
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_WORCKPHONE+zgui->strtoqstr(uinfo.Workphone), QChar());
	detailItem->enableStructureDataParse(false);
	spWork->addItem(detailItem);		
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, "Fax: "+zgui->strtoqstr(uinfo.Workfax), QChar());
	detailItem->enableStructureDataParse(false);
	spWork->addItem(detailItem);	
	
	detailItem = new ZDetailItem(ZDetailItem::ItemTypeSeparator);
	detailItem->appendTextIconRow("ItemTypeSeparator");
	spWork->addItem(detailItem); 		
		
	detailItem = new ZDetailItem(ZDetailItem::ItemTypeJ);
	detailItem->appendTextIconRow(LNG_COMPANY);
	spWork->addItem(detailItem);
	 
	detailItem = new ZDetailItem(ZDetailItem::ItemTypeSeparator);
	detailItem->appendTextIconRow("ItemTypeSeparator");
	spWork->addItem(detailItem); 	
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_COMPANYNAME + zgui->strtoqstr( uinfo.Workcompany ), QChar());
	detailItem->enableStructureDataParse(false);
	spWork->addItem(detailItem);	
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_OCCUPATION + zgui->strtoqstr( country2string(uinfo.Workoccupation) ), QChar());
	detailItem->enableStructureDataParse(false);
	spWork->addItem(detailItem);		
	
    detailItem = new ZDetailItem(ZDetailItem::ItemTypeE, LNG_WEBPAGE+zgui->strtoqstr(uinfo.Workwebpage), QChar());
	detailItem->enableStructureDataParse(false);
	spWork->addItem(detailItem);	

	// About info
	tvDesc->setFontSize( cfg_chatFontSize );
	tvDesc->setFixedHeight(SCREEN_WIDTH-5);
	tvDesc->setText(zgui->strtoqstr(uinfo.Notes));	
	
	tabWidget->setCurrentPage(0);
	spFull->repaintContents(tabWidget->currentPage()->rect());
}

