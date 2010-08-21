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

#include "./../BaseDlg.h"
#include "MessageDlg.h"
#include "./../config.h"

#include <qlabel.h>
#include <ZApplication.h>
#include <ZSoftKey.h>
#include <ZImage.h>
#include <ZLabel.h>
#include <ZSeparator.h>
#include <qrect.h>

#include "zDefs.h"

MessageDlg::MessageDlg(int type, QString caption, QString text)
    :MyBaseDlg()
{
	logMes_2("MessageDlg: start");
 	setMainWidgetTitle ( "zMessanger" );

	sv = new ZScrollPanel(this);
	setContentWidget ( sv );
	sv->setFixedWidth( SCREEN_WIDTH );
	//sv->setFixedHeight(260);
	
	ZImage *img = new ZImage( sv );
	if ( type == 0)
	{
		img->setPixmap ( QPixmap ( ProgDir + "/dlg/conecterr.png") );
	} else
	{
		img->setPixmap ( QPixmap ( ProgDir + "/dlg/question.png" ) );
	}
	sv->addChild(img, 5,5);

	zlCaption = new ZLabel(caption,sv);
	zlCaption->setAutoResize(true);
	zlCaption->setAlignment(ZLabel::AlignCenter);
	zlCaption->setFixedWidth(SCREEN_WIDTH-45);
	//zlCaption->setPreferredWidth(200);
	sv->addChild(zlCaption, 70, 10);

	zlText = new ZLabel(text,sv);
	zlText->setAutoResize(true);
	//zlText->setAlignment(ZLabel::AlignCenter);
	zlText->setFixedWidth(SCREEN_WIDTH-10);
	//zlCaption->setPreferredWidth(200);
	sv->addChild(zlText, 5, 85);

	ZSoftKey *softKey = new ZSoftKey ( NULL, this, this );

	if ( type == 0)
	{
		softKey->setText ( ZSoftKey::LEFT, LNG_OK, ( ZSoftKey::TEXT_PRIORITY ) 0 );
		softKey->setClickedSlot ( ZSoftKey::LEFT, this, SLOT ( accept() ) );
	} else
	{
		softKey->setText ( ZSoftKey::LEFT, LNG_YES, ( ZSoftKey::TEXT_PRIORITY ) 0 );
		softKey->setClickedSlot ( ZSoftKey::LEFT, this, SLOT ( accept() ) );
  		softKey->setText ( ZSoftKey::RIGHT, LNG_NO, ( ZSoftKey::TEXT_PRIORITY ) 0 );
 		softKey->setClickedSlot ( ZSoftKey::RIGHT, this, SLOT ( reject() ) );
	}

	setCSTWidget ( softKey );

	logMes_2("MessageDlg: end");
}

MessageDlg::~MessageDlg()
{

}

