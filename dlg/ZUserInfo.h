//
// C++ Interface: ZAboutDlg
//
// Description: 
//
//
// Author: root <root@andLinux>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
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

