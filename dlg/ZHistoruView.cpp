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

#include "ZHistoruView.h"
#include "config.h"

#include <qlabel.h>
#include <qrect.h>
#include <qfile.h>
#include <qtextstream.h>

#include <ZApplication.h>
#include <ZSoftKey.h>
#include <ZImage.h>
#include <ZLabel.h>
#include <ZSeparator.h>
#include <ZFormContainer.h>

#include "icqkid2.h"
#include "const_strings.h"

#include "zgui.h"
#include "zDefs.h"

ZHistoryView::ZHistoryView( string uin )
    :MyBaseDlg()
{
 	setMainWidgetTitle ( LNG_HISTORU );

	zmleHistory = new xTextView( this, zSmile );
	zmleHistory->setFixedWidth( SCREEN_WIDTH );
	zmleHistory->setFontSize( cfg_chatFontSize );

	mesList messanges;
	QString str = "";
	for ( tHistory::Iterator it = zgui->messageList.begin( ); it != zgui->messageList.end( ); it++) 
	{ 
		if ( it.key() == QString( uin.c_str() ).toUInt() ) 
		{
			messanges = it.data();
			for ( uint n = 0; n < messanges.count(); n++ )
			{	
				str = str + zgui->bdMesToText(  *messanges.at(n) , true  )+"\n";
			}
		}
	}

	QFile file ( ProgDir+"history/"+QString( uin.c_str() ) + ".txt" );
	if (  file.open ( IO_ReadOnly ) )
	{
		QTextStream stream ( &file );
		stream.setEncoding ( QTextStream::UnicodeUTF8 );
		zmleHistory->setText ( stream.read()+str, false );
		file.close();
	} else
	{
		zmleHistory->setText ( str );	
	}
	
	zmleHistory->toEnd();

	setContentWidget ( zmleHistory );

	ZSoftKey *softKey = new ZSoftKey ( "CST_2", this, this );
	softKey->setText ( ZSoftKey::LEFT, LNG_OK, ( ZSoftKey::TEXT_PRIORITY ) 0 );
	softKey->setClickedSlot ( ZSoftKey::LEFT, this, SLOT ( reject() ) );
	setCSTWidget ( softKey );
}

