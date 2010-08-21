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

#ifndef ZMESSAGEDLG_H
#define ZMESSAGEDLG_H

#include "BaseDlg.h"
#include <ZLabel.h>
#include <ZScrollPanel.h>

class MessageDlg : public MyBaseDlg 
{
	Q_OBJECT

	public:
		MessageDlg(int type, QString caption, QString text); 
		~MessageDlg();

	private:
		ZLabel * zlCaption;
		ZLabel * zlText;
		ZScrollPanel * sv;
};

#endif

