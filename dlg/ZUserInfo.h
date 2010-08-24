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

#ifndef ZUSERINFO_H
#define ZUSERINFO_H

#include "BaseDlg.h"
#include "xTextView.h"
#include <ZLabel.h>
#include <qscrollview.h>
#include <ZDetailView.h>
#include <ZNavTabWidget.h>
#include <qlabel.h>
#include "icqkid2.h"

class ZUserInfo : public MyBaseDlg {
  Q_OBJECT

  public:
    ZUserInfo(std::string id); 
    ~ZUserInfo();
    
    void showInfo(ICQKidFullUserInfo  uinfo);
    std::string id;
    bool pthreadExit;
    
  public slots:
  	void menu_update();
  	void menu_close();
    
  private:
  	ZNavTabWidget *tabWidget;
  	ZDetailView * spFull;
  	ZDetailView * spMain;
  	ZDetailView * spHome;
  	ZDetailView * spWork;
  	ZDetailView * spPrivate;
	QScrollView * spAbout;
	pthread_t thread;
	xTextView * desc;
	
	ZWidget *myWidget;

	void initInterface();
};

#endif

