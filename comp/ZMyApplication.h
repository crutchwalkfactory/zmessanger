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

#ifndef ZMYAPPLICATION_H
#define ZMYAPPLICATION_H

#include <ZApplication.h>

class ZMyApplication : public ZApplication
{
    Q_OBJECT
  public:
    ZMyApplication( int& argc, char **argv, QApplication::Type t = QApplication::GuiClient ); 
    virtual ~ZMyApplication();

  protected slots:
	virtual void slotQuickQuit();
	virtual void slotAppMessage(QCString const&, QArray<char> const&);
};

#endif
