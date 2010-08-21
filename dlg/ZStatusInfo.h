//
// C++ Interface: ZStatusInfo
//
// Description: 
//
//
// Author: Ant-ON <prozanton@gmail.com>, (C) 2009-2010

#ifndef ZSTATUSINFO_H
#define ZSTATUSINFO_H

#include "BaseDlg.h"

#include <string>

#include <ZLabel.h>
#include <ZScrollPanel.h>
#include <ZMultiLineEdit.h>
#include "config.h"


class ZStatusInfo : public MyBaseDlg {
  Q_OBJECT

  public:
    ZStatusInfo(std::string id, bool prot); 
    ~ZStatusInfo();

    std::string id;
    bool protocol;
    void setTitle(QString S);
    void setDesc(QString S);

  public slots:
    void menu_request();

   private:
    ZLabel * title;
    ZMultiLineEdit * desc;
    ZScrollPanel * sv;
    ZWidget *myWidget; 
};

#endif

