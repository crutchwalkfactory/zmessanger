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

#include "ZMyApplication.h"

ZMyApplication::ZMyApplication( int& argc, char **argv, QApplication::Type ):
	ZApplication( argc, argv )
{
}

ZMyApplication::~ZMyApplication()
{
}

void ZMyApplication::slotQuickQuit()
{
	processEvents();
}

void ZMyApplication::slotAppMessage(QCString const&, QArray<char> const&)
{	
	processEvents();	
}	
