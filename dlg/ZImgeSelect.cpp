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

#include "ZImgeSelect.h"

#include "ZXStatusText.h"
#include <qlabel.h>
#include <ZApplication.h>
#include <ZSoftKey.h>

#include "config.h"

#include "zEmoticon.h"
#include "zgui.h"
#include "zDefs.h"


ZImgeSelect::ZImgeSelect(bool _smile, bool _addHotStatus)
    :MyBaseDlg()
{
	smile = _smile;
	addHotStatus = _addHotStatus;
	
	myWidget = new ZWidget ();
	
	softKey = new ZSoftKey ( NULL, this, this );
	softKey->setText ( ZSoftKey::LEFT, LNG_OK, ( ZSoftKey::TEXT_PRIORITY )0 );
	softKey->setClickedSlot ( ZSoftKey::RIGHT, this, SLOT ( slotLeftSoftKey()) );

	if (smile)
	{
		setMainWidgetTitle(LNG_SMYLESEL);

		iconView = new ZIconView(this, NULL, 0, zSmile->getSmileCount()-NO_SMILE, ZSkinService::clsZIconView2);

		iconView->setShowLabel(false);

		for ( uint i = NO_SMILE; i < zSmile->getSmileCount(); i++ ) 
		{
			ZIconViewItem *item = new ZIconViewItem(iconView);
			item->setPixmap(zSmile->getEmotIcon(i), false, false);
			item->setPixmapRect(QRect(0,0,25,25));
		}
		
		#if  ( defined(NEW_PLATFORM) && !defined(CUTED_PLATFORM) )
		iconView->setItemSizeFixed(true);
		iconView->setItemSize(QSize(25, 25));
		#endif
		iconView->setLayout(5,8);
		

		iconView->setCurrentItem(iconView->item(oldSmile));

		softKey->setClickedSlot ( ZSoftKey::LEFT, this, SLOT ( slotImegeSelected() ) );

		connect ( iconView, SIGNAL ( returnPressed(ZIconViewItem*) ), this, SLOT ( lbSmileSel(ZIconViewItem*) ) );

		setContentWidget ( iconView );

		softKey->setText ( ZSoftKey::RIGHT, LNG_CANCEL, ( ZSoftKey::TEXT_PRIORITY )0 );
	} else
	{
		setMainWidgetTitle("xStatus");

		init_strings_maps();

		tabWidget = new ZNavTabWidget(myWidget);

		lbStatus = new ZListBox ( QString ( "%I16%M" ), tabWidget, 0);
		lbStatus->setFixedWidth ( SCREEN_WIDTH ); 

		QPixmap pm;
		for (int i=0;i<38;i++)
		{
			pm.load( ProgDir + "/status/icq/x/icq_xstatus"  + QString::number(i-1) + ".png" );
			ZSettingItem* listitem = new ZSettingItem(lbStatus, QString("%I16%M") );
			listitem->setPixmap ( 0, pm );
			listitem->appendSubItem ( 1, QString::fromUtf8(x_status2string(i).c_str()) , true );
			lbStatus->insertItem ( listitem, -1, true ); 				
		}
		
		connect ( lbStatus, SIGNAL ( selected ( int ) ), this, SLOT ( lbStatusSel ( int ) ) );

		pm.load(ProgDir+ "/menu/xStatus.png");
		tabWidget->addTab(lbStatus, QIconSet(pm), "");
		
		if ( !addHotStatus )
		{
			lbQuickStatus = new ZListBox ( QString ( "%I16%M" ), tabWidget, 0);
			lbQuickStatus->setFixedWidth ( SCREEN_WIDTH ); 		
			
			ZConfig cfg(ProgDir+"/QuickXStatus.cfg");
			lbQuickStatus->clear();
			QString st;
			int nst;
			for (int i=1;i<20;i++)
			{
				nst = cfg.readNumEntry(QString("QuickXStatus"), QString("Status"+QString::number(i)), -1);
				if (nst > -1)
				{
					pm.load( ProgDir + "/status/icq/x/icq_xstatus"  + QString::number(nst-1) + ".png" );
					ZSettingItem* listitem = new ZSettingItem(lbQuickStatus, QString("%I16%M") );
					listitem->setPixmap ( 0, pm );
					st = cfg.readEntry(QString("QuickXStatus"), QString("Text"+QString::number(i)), "") + " "+cfg.readEntry(QString("QuickXStatus"), QString("Desc"+QString::number(i)), "");
					if (st.length()>30)
					{
						st.remove(30,st.length());
						st = st + "...";	
					}
					listitem->appendSubItem ( 1, st , false );	
					listitem->setReservedData ( i );
					lbQuickStatus->insertItem ( listitem,-1,true );
				} else
				{
					break;
				}
			}

			connect ( lbQuickStatus, SIGNAL ( selected ( int ) ), this, SLOT ( lbQStatusSel ( int ) ) );

			pm.load(ProgDir+ "/image/tab_private.png");
			tabWidget->addTab(lbQuickStatus, QIconSet(pm), "");
		
			connect(tabWidget,SIGNAL(currentChanged(QWidget* )),this,SLOT(slotPageChanged(QWidget* )));
		} else
			slotPageChanged(NULL);

		softKey->setClickedSlot ( ZSoftKey::LEFT, this, SLOT ( lbStatusSel ( int ) ) );

		setContentWidget ( tabWidget );

		lbStatus->setFocus();
	}

	setCSTWidget ( softKey );
}


ZImgeSelect::~ZImgeSelect()
{
	delete myWidget;
	myWidget=NULL;
}

void ZImgeSelect::lbStatusSel(int i)
{
	if ( addHotStatus && i==0 )
		accept();
	
    if( i == 0)
    { 
    	zgui->icq->setXStatus(i,"", "");
    } else
    if ( i>=0 )
    {
    	ZXStatusText * dlgXStatusText = new ZXStatusText(i, addHotStatus);
    	dlgXStatusText->exec();
    	delete dlgXStatusText;
    	dlgXStatusText = NULL;
    }
    
    accept();
}

void ZImgeSelect::lbQStatusSel(int n)
{
	if ( n>=0 )
	{
		ZSettingItem* listitem = (ZSettingItem*)lbQuickStatus->item ( n );

		int i = listitem->getReservedData();

		ZConfig cfg(ProgDir+"/QuickXStatus.cfg");
		int idStatus = cfg.readNumEntry(QString("QuickXStatus"), QString("Status"+QString::number(i)), -1);
		QString statTitle = cfg.readEntry(QString("QuickXStatus"), QString("Text"+QString::number(i)), "");
		QString statDesc = cfg.readEntry(QString("QuickXStatus"), QString("Desc"+QString::number(i)), ""); 
		
		#ifdef _SupportZPlayer
		if ( statDesc == "%nowPlaying%" )
		{
			zgui->icq->setXStatus(idStatus, statTitle.utf8().data(), "");
			zgui->startPlayerChenel();
		}
		else
		{
			zgui->icq->setXStatus(idStatus, statTitle, statDesc);
			zgui->stopPlayerChenel();
		}
		#else
		zgui->icq->setXStatus(idStatus, statTitle, statDesc);
		#endif
	}
	accept();
}

void ZImgeSelect::slotPageChanged(QWidget* )
{
	int i = tabWidget->currentPageIndex();
	
	switch (i)
	{
		case 0:
		{
			softKey->setClickedSlot( ZSoftKey::LEFT, this, SLOT( lbStatusChange() ) );
			softKey->setText ( ZSoftKey::RIGHT, LNG_CANCEL, ( ZSoftKey::TEXT_PRIORITY )0 );
			break;
		}
		case 1:
		{
			softKey->setClickedSlot( ZSoftKey::LEFT, this, SLOT( lbQStatusChange() ) );
			softKey->setText ( ZSoftKey::RIGHT, LNG_ADD, ( ZSoftKey::TEXT_PRIORITY )0 );
			break;
		}
	}
}

void ZImgeSelect::lbQStatusChange()
{
	lbQStatusSel(lbQuickStatus->currentItem());
}

void ZImgeSelect::lbStatusChange()
{
	lbStatusSel(lbStatus->currentItem());
}

void ZImgeSelect::lbSmileSel(ZIconViewItem * item)
{
	if ( item )
	{	
		int i = item->index();
		emit addSmile(zSmile->getEmotStr(NO_SMILE+i, false));
		oldSmile = i;
	}
    accept();
}

void ZImgeSelect::slotImegeSelected()
{
    lbSmileSel(iconView->currentItem());
}

void ZImgeSelect::slotLeftSoftKey()
{
	if ( !smile && !addHotStatus && tabWidget->currentPageIndex()==1 )
	{
		ZImgeSelect * dlg = new ZImgeSelect(smile, !addHotStatus);
		dlg->exec();
		delete dlg;
		dlg = NULL;
		return;
	}
	reject();
}
