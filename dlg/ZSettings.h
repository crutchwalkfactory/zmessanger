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

#ifndef ZSETTINGS_H
#define ZSETTINGS_H

#include "BaseDlg.h"
#include "config.h"
#include <ZSoftKey.h>
#include <ZConfig.h>
#include <ZNavTabWidget.h>

#include <qtextcodec.h>

#include "ZOptionItem.h"

class ZSettingsDlg : public MyBaseDlg
{
    Q_OBJECT

public:
    ZSettingsDlg();
    ~ZSettingsDlg();

private:
    ZNavTabWidget* tabWidget;

    ZOptionItem * optInetLink;
    ZOptionItem * optICQServer;
    ZOptionItem * optICQPort;
    ZOptionItem * optICQKeepTime;
    ZOptionItem * optICQdelayReadSnec;
    ZOptionItem * optTonePath;
    ZOptionItem * optCodePage;

    ZOptionItem * optVibrate;
    ZOptionItem * optTone;
    ZOptionItem * optToneVol;

    ZOptionItem * optShowOffLine;
    ZOptionItem * optShowGroup;
    ZOptionItem * optRigthXStatus;

    ZOptionItem * optXTrazRequest;
    ZOptionItem * optAutoMsgRequest;
    ZOptionItem * optEye;
    ZOptionItem * optSendTypeMes;
    ZOptionItem * optSendByCenter;

    #ifndef _NoUSBNetOption
    ZOptionItem *optUSBNet;
    #endif
    
    ZOptionItem * optSaveStusWithExit;

    ZOptionItem * optChatFontSize;
    ZOptionItem * optMesFontSize;
    ZOptionItem * optMaxNumLine;
	ZOptionItem * optNoShowStatusBarInChat;
    ZOptionItem * optSortType;
    
    ZWidget *myWidget;
    
    ZListBox *net;
    ZListBox *alert;
    ZListBox *CL;
    ZListBox *Chat;
    ZListBox *othe;
    
    QStringList * codec;

public slots:
    void saveSetting();
};

#endif 
