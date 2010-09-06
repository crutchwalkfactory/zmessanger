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

#ifndef ZABOUTDLG_H
#define ZABOUTDLG_H

#include <ZScrollPanel.h>
#include <ZNavTabWidget.h>
#include <ZListBox.h>
#ifndef OLD_SDK
#include <ZSettingItem.h>
#endif

#include "BaseDlg.h"

class ZAboutDialog : public MyBaseDlg 
{
	Q_OBJECT

public:
	ZAboutDialog(); 
	~ZAboutDialog();

protected slots:
    void slotPageChanged(QWidget* );

private:
	ZWidget *myWidget;
	ZListBox* phoneInfo;
	ZNavTabWidget* tabWidget;
	ZScrollPanel * sv;
};

#endif

