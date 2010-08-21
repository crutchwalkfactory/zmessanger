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

#ifndef ZEYE_H
#define ZEYE_H

#include "./../BaseDlg.h"
#include <ZSoftKey.h>
#include <ZListBox.h>

class ZEyeDialog : public MyBaseDlg
{
    Q_OBJECT

public:
    ZEyeDialog();
    ~ZEyeDialog();

private:
    ZListBox * lbList;
    ZSoftKey *softKey;

};

#endif 

