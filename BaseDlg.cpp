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
#include <ZApplication.h>
#include <ZSoftKey.h>

MyBaseDlg::MyBaseDlg()
  :_ZKbMainWidget( ZHeader::MAINDISPLAY_HEADER, NULL, "ZMainWidget", WType_Modal | WType_TopLevel )
{
	myInLoop = false;
	#ifdef CUTED_PLATFORM
	header = 0;
	setMainWidgetTitle("");
	#endif
}

MyBaseDlg::~MyBaseDlg()
{

}

#ifdef CUTED_PLATFORM
bool MyBaseDlg::setMainWidgetTitle( const QString &Title )
{
    if ( ! header )
    {
		header = new ZHeader(ZHeader::MAINDISPLAY_HEADER,this,"zHeader",Qt::WStyle_Tool|Qt::WStyle_Customize);
		header->setGeometry( ZGlobal::getHeaderR( false ) );
		setTitleBarWidget(header);
    }
    header->setPrimTitle(Title);
    return true;  
} 

bool MyBaseDlg::setSoftKey( ZSoftKey *cstWidget )
{
	return setCSTWidget(cstWidget);	
}

QSize MyBaseDlg::headerSize()
{
	return header->size();	 
}
#endif

int MyBaseDlg::exec() 
{
  setResult(0);
  show();
  myInLoop = TRUE;
  qApp->enter_loop();
  return result();
}

void MyBaseDlg::done( int r )
{
  hide();
  if (myInLoop) 
  {
    qApp->exit_loop();
  }
  setResult(r);
  if ( qApp->mainWidget() == this )
    qApp->quit();
}

void MyBaseDlg::accept() 
{
  done(Accepted);
}

void MyBaseDlg::reject() 
{
  done(Rejected);
}
