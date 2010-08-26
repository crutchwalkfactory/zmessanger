//
// C++ Implementation: ZXStatusText
//
// Description: 
//
//
// Author: Ant-ON <prozanton@gmail.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "ZXStatusText.h"
#include "./../BaseDlg.h"
#include "./../config.h"

#include <qlabel.h>
#include <ZApplication.h>
#include <ZSoftKey.h>
#include <ZImage.h>
#include <ZLabel.h>
#include <ZSeparator.h>
#include <qrect.h>
#include <ZFormContainer.h>

#include "./../icqlib/icqkid2.h"
#include "./../icqlib/const_strings.h"

ZXStatusText::ZXStatusText(int _idStatus, bool _addHotStatus)
    :MyBaseDlg()
{
 	setMainWidgetTitle ( "xStatus" );
 	
 	idStatus = _idStatus;
 	addHotStatus = _addHotStatus;

	ZFormContainer *sv = new ZFormContainer(this, 0, ZSkinService::clsZFormContainer);
	
	#ifdef _FixLineEdit
	statTitle = new ZMyLineEdit( QString( QString::fromUtf8(x_status2string(idStatus).c_str())), sv);
	#else
	statTitle = new ZLineEdit( QString( QString::fromUtf8(x_status2string(idStatus).c_str())), sv);
	#endif
	statTitle->setTitle( LNG_STATUSTITLE );
	//statTitle->setText( QString( QString::fromUtf8(x_status2string(idStatus).c_str())) );
	statTitle->setFixedWidth(SCREEN_WIDTH-10);
	statTitle->setMaxLength(28);
	sv->addChild(statTitle);
	
	statDesc = new ZMultiLineEdit(sv, false);
	statDesc->setFontPercent( (float)cfg_mesFontSize/(float)22 );
	statDesc->setFixedWidth( SCREEN_WIDTH );
	statDesc->setUnderline( false );
	statDesc->setSizePolicy( QSizePolicy ( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
	statDesc->setTitle(LNG_STATUSDESCRIPTION);
	statDesc->setText("");
	statDesc->setFixedHeight(145);
	statDesc->setFixedWidth(SCREEN_WIDTH-10);

	sv->addChild(statDesc);

	setContentWidget ( sv );
	

	ZSoftKey *softKey = new ZSoftKey ( NULL, this, this );
	softKey->setText ( ZSoftKey::LEFT, LNG_OK, ( ZSoftKey::TEXT_PRIORITY ) 0 );
	softKey->setClickedSlot ( ZSoftKey::LEFT, this, SLOT ( menu_setStat() ) );
	softKey->setText ( ZSoftKey::RIGHT, LNG_CANCEL, ( ZSoftKey::TEXT_PRIORITY ) 0 );
	softKey->setClickedSlot ( ZSoftKey::RIGHT, this, SLOT ( reject() ) );	
	setCSTWidget ( softKey );
}

void ZXStatusText::menu_setStat()
{
	if ( !addHotStatus )
	{
		#ifdef _SupportZPlayer
		if ( statDesc->text() == "%nowPlaying%" )
		{
			zgui->icq->setXStatus(idStatus, statTitle->text(), "" );
			zgui->startPlayerChenel();
		}
		else
		{
			zgui->icq->setXStatus(idStatus, statTitle->text(), statDesc->text());
			zgui->stopPlayerChenel();
		}
		#else
		zgui->icq->setXStatus(idStatus, statTitle->text(), statDesc->text());
		#endif
	} else
	{
			ZConfig cfg(ProgDir+"/QuickXStatus.cfg");
			int nst;
			for (int i=1;i<20;i++)
			{
				nst = cfg.readNumEntry(QString("QuickXStatus"), QString("Status"+QString::number(i)), -1);
				if (nst < 0)
				{
					cfg.writeEntry(QString("QuickXStatus"), QString("Status"+QString::number(i)), idStatus);
					cfg.writeEntry(QString("QuickXStatus"), QString("Text"+QString::number(i)), statTitle->text());
					cfg.writeEntry(QString("QuickXStatus"), QString("Desc"+QString::number(i)), statDesc->text());
					cfg.flush();
					break;
				}
			}		
	}
	accept();
}
