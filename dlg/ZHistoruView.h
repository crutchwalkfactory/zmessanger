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

#ifndef ZHISTORYVIEW_H
#define ZHISTORYVIEW_H

#include "BaseDlg.h"

#include <string>

#include "xTextView.h"

#include <ZLabel.h>
#include <ZScrollPanel.h>

class ZHistoryView : public MyBaseDlg 
{
	Q_OBJECT

	public:
		ZHistoryView( std::string uin ); 
		~ZHistoryView();

	protected:
		bool eventFilter(QObject* o, QEvent* pEvent);

	private:
		xTextView * zmleHistory;
};

#endif
