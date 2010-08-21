//
// C++ Implementation: ZStatusInfo
//
// Description:
//
//
// Author: Ant-ON <prozanton@gmail.com>, (C) 2009-2010

#include "BaseDlg.h"
#include "ZStatusInfo.h"
#include "config.h"

#include <qlabel.h>
#include <ZApplication.h>
#include <ZSoftKey.h>
#include <ZImage.h>
#include <ZLabel.h>
#include <ZSeparator.h>
#include <qrect.h>

#include "icqkid2.h"

#include "zgui.h"
#include "zDefs.h"

ZStatusInfo::ZStatusInfo( std::string _id, bool _protocol )
    :MyBaseDlg()
{
 	setMainWidgetTitle ( LNG_STATUSS );
 	
	myWidget = new ZWidget ( this, NULL, 0);

 	id = _id;
 	protocol = _protocol;

	sv = new ZScrollPanel(this);
	sv->setFixedWidth(SCREEN_WIDTH);

	ZLabel * a = new ZLabel(LNG_STATUS+":",sv);
	a->setAutoResize(true);
	a->setPreferredWidth(SCREEN_WIDTH-10);
	sv->addChild(a, 0, 2);	

	QString ico;
	QString stat;
	
	ZImage *img = new ZImage( sv );
	sv->addChild(img, 1,30);
		
	a = new ZLabel(stat, sv);
	a->setAutoResize(true);
	a->setPreferredWidth(SCREEN_WIDTH-10);
	sv->addChild(a, 20, 27);

	if ( protocol == PROT_ICQ )
	{	
		int icqId = zgui->icq->findContact(_id);
		
		switch( zgui->icq->getStatus( icqId ) )
		{
			case STATUS_OFFLINE    : ico = "offline.png"; stat = LNG_OFFLINE; break;
			case STATUS_ONLINE     : ico = "online.png"; stat = LNG_ONLINE; break;
			case STATUS_AWAY       : ico = "away.png"; stat = LNG_AWAY; break;
			case STATUS_DND        : ico = "dnd.png"; stat = LNG_DND; break;
			case STATUS_NA         : ico = "na.png"; stat = LNG_NA; break;
			case STATUS_OCCUPIED   : ico = "occupied.png"; stat = LNG_OCCUPED; break;
			case STATUS_FREE4CHAT  : ico = "ffc.png"; stat = LNG_FFC; break;
			case STATUS_EVIL       : ico = "evil.png"; stat = LNG_EVIL; break;
			case STATUS_DEPRESSION : ico = "depression.png"; stat = LNG_DEPRESSION; break;
			case STATUS_HOME       : ico = "athome.png"; stat = LNG_ATHOME; break;
			case STATUS_WORK       : ico = "atwork.png"; stat = LNG_ATWORCK; break;
			case STATUS_LUNCH      : ico = "lunch.png"; stat = LNG_LANCH; break;
			default                : ico = "invisible.png"; stat = LNG_INVISIBLE;
		}
		img->setPixmap ( QPixmap ( ProgDir + "/status/icq/" + ico ) );		
		
		a = new ZLabel("xStatus:", sv);
		a->setAutoResize(true);
		a->setPreferredWidth(SCREEN_WIDTH-10);
		sv->addChild(a, 0, 50);
		
		int st;
	
		if ( !zgui->icq->noAutoXTrazRequest )
		{
			if ( zgui->icq->getXStatus( icqId ) != X_STATUS_NONE )
			{
				img = new ZImage( sv );
				sv->addChild(img, 1,75);
	
				title = new ZLabel(QString::fromUtf8( zgui->icq->getXStatusTitle( icqId ).c_str() ) ,sv);
				
				
				title->setAutoResize(true);
				title->setPreferredWidth(SCREEN_WIDTH-10);
				sv->addChild(title, 18, 71);
	
				st = zgui->icq->getXStatus( icqId )-1;

				img->setPixmap ( QPixmap ( ProgDir +  "/status/icq/x/icq_xstatus" + QString::number(st-1)+ ".png" ) );
				
				desc = new ZMultiLineEdit(sv, false);			
				desc->setText(QString::fromUtf8( zgui->icq->getXStatusDescription( icqId ).c_str() ));
				desc->setFixedWidth( SCREEN_WIDTH );
				desc->setUnderline( false );
				desc->setSizePolicy( QSizePolicy ( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
				desc->setFixedHeight(135);	
				desc->setReadOnly(true);
				desc->setFontPercent( (float)cfg_chatFontSize/(float)22 );									
				sv->addChild(a, 0, 110);
			}
		} else
		{
			if ( zgui->icq->getXStatus( icqId) != X_STATUS_NONE )
			{
				img = new ZImage( sv );
				sv->addChild(img, 1,75);
	
				if ( QString::fromUtf8( zgui->icq->getXStatusTitle( icqId).c_str() ) != "" )
				{
					title = new ZLabel(QString::fromUtf8( zgui->icq->getXStatusTitle( icqId ).c_str() ) ,sv);
	
				} else
				{
					title = new ZLabel( "     "+LNG_READING+"..." ,this);
					zgui->icq->sendXtrazRequest( id );
				}
				title->setAutoResize(true);
				title->setFixedWidth(SCREEN_WIDTH-10);
				sv->addChild(title, 20, 71);
				
				st = zgui->icq->getXStatus( icqId);

				img->setPixmap ( QPixmap ( ProgDir +  "/status/icq/x/icq_xstatus" +
					QString::number(st-1)+ ".png" ) );
					
				desc = new ZMultiLineEdit(sv, false);			
				if ( QString::fromUtf8( zgui->icq->getXStatusDescription( icqId).c_str() ) != ""  )
					desc->setText(QString::fromUtf8( zgui->icq->getXStatusDescription( icqId).c_str() ));
				else
					desc->setText("");
				desc->setFixedWidth( SCREEN_WIDTH );
				desc->setUnderline( false );
				desc->setSizePolicy( QSizePolicy ( QSizePolicy::Expanding, QSizePolicy::Expanding ) );				
				desc->setReadOnly(true);
				desc->setFontPercent( (float)cfg_chatFontSize/(float)22 );
				desc->setFixedHeight(135);
				
				sv->addChild(desc, 0, 110);
			}
		}
	}
	#ifdef _XMPP
	if ( protocol == PROT_JABBER )
	{
		/* FIX ME !!!
		switch( zgui->xmpp->getStatus(id) )
		{
			case XMPP_OFFLINE    : ico = "offline.png"; stat = LNG_OFFLINE; break;
			case XMPP_ONLINE     : ico = "online.png"; stat = LNG_ONLINE; break;
			case XMPP_FREE4CHAT  : ico = "ffc.png"; stat = LNG_FFC; break;
			case XMPP_AWAY       : ico = "away.png"; stat = LNG_AWAY; break;
			case XMPP_DND        : ico = "dnd.png"; stat = LNG_DND; break;
			case XMPP_NA         : ico = "na.png"; stat = LNG_NA; break;
			default                : ico = "unknown.png"; stat = LNG_UNKNOWN;
		}

		img->setPixmap ( QPixmap ( ProgDir + "/status/jabber/" + ico ) );
		#ifndef _MyMultiLineEdit
		desc = new ZMultiLineEdit(sv, false);			
		#else
		desc = new ZMultiLineEditEx(sv, false);			
		#endif
		desc->setText(QString::fromUtf8( zgui->xmpp->getStatusText(id).c_str() ));
		desc->setReadOnly(true);
		desc->setFontPercent( (float)cfg_chatFontSize/(float)22 );
		desc->setFixedHeight(135);	
		sv->addChild(desc, 0, 110);
		*/
	}
	#endif
	setContentWidget ( sv );

	ZSoftKey *softKey = new ZSoftKey ( NULL, this, this );
	softKey->setText ( ZSoftKey::LEFT, LNG_OK, ( ZSoftKey::TEXT_PRIORITY ) 0 );
	softKey->setClickedSlot ( ZSoftKey::LEFT, this, SLOT ( accept() ) );
	if ( protocol == PROT_ICQ )
	{
		softKey->setText ( ZSoftKey::RIGHT, LNG_REQUEST, ( ZSoftKey::TEXT_PRIORITY ) 0 );
		softKey->setClickedSlot ( ZSoftKey::RIGHT, this, SLOT ( menu_request() ) );
	}	
	setCSTWidget ( softKey );
}

ZStatusInfo::~ZStatusInfo()
{

}

void ZStatusInfo::menu_request()
{
	#ifdef _XMPP
	if ( protocol == PROT_JABBER )
		return;
	#endif
	zgui->icq->sendXtrazRequest( id );
}

void ZStatusInfo::setDesc(QString S)
{
	#ifdef _XMPP
	if ( protocol == PROT_JABBER )
		return;
	#endif
	desc->setText(S);
}

void ZStatusInfo::setTitle(QString S)
{
	#ifdef _XMPP
	if ( protocol == PROT_JABBER )
		return;
	#endif
	title->setText(S);
	int h = title->height();
	if ( h%2 != 0 )
	{
		title->resize(title->width(),++h);
	} else
	{
		title->resize(title->width(),--h);
	}
}
