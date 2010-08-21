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

#ifndef BASEDLG_H
#define BASEDLG_H

#include "GUI_Define.h"
#include <ZKbMainWidget.h>

#include "lng.h"

#ifdef CUTED_PLATFORM
#define _ZKbMainWidget ZMainWidget
#else
#define _ZKbMainWidget ZKbMainWidget
#endif

class MyBaseDlg : public _ZKbMainWidget 
{
  Q_OBJECT

  public:
    MyBaseDlg(); 
    ~MyBaseDlg();
    
    int exec();
    int result() const { return rescode; };
    #ifdef CUTED_PLATFORM
    bool setMainWidgetTitle( const QString &Title );
    bool setSoftKey( ZSoftKey *cstWidget );
    QSize headerSize();
    #endif
    
  protected slots:
    virtual void done(int);
    virtual void accept();
    virtual void reject();
        
  private:
    bool myInLoop;
    int rescode;
    #ifdef CUTED_PLATFORM
    ZHeader * header;
    #endif
        
  protected:
    enum DialogCode { Rejected, Accepted };
    void setResult(int r) { rescode = r; }    
};

#endif
