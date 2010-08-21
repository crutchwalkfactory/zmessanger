//
// C++ Interface: ZXStatusText
//
// Description: 
//
//
// Author: Ant-ON <prozanton@gmail.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ZXSTATUSTETX_H
#define ZXSTATUSTETX_H

#include "BaseDlg.h"
#include <ZLabel.h>
#include <ZScrollPanel.h>

#ifdef _FixLineEdit
	#include "ZMyLineEdit.h"
#else
	#include <ZLineEdit.h>
#endif

#include "zgui.h"
#include "zDefs.h"

class ZXStatusText : public MyBaseDlg {
  Q_OBJECT

  public:
    ZXStatusText(int idStatus1); 
    ~ZXStatusText(){};

    int idStatus;

  public slots:
    void  menu_setStat();

   private:
    #ifdef _FixLineEdit
    ZMyLineEdit * statTitle;
    #else
    ZLineEdit * statTitle; //28 max
    #endif
    ZMultiLineEdit * statDesc;
    ZScrollPanel * sv;
};

#endif
