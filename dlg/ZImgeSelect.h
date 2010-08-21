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

#ifndef ZIMGESELECT_H
#define ZIMGESELECT_H

#include "./../BaseDlg.h"
#include <ZIconView.h>
#include <ZSoftKey.h>
#include <ZListBox.h>
#include <ZConfig.h>
#include <ZNavTabWidget.h>

class ZImgeSelect : public MyBaseDlg
{
    Q_OBJECT

public:
    ZImgeSelect( bool smile = true);
    ~ZImgeSelect();
  
signals:
	void addSmile(QString smile); 

private:
    ZIconView * iconView;
    ZListBox * lbStatus;
    ZListBox * lbQuickStatus;
    ZSoftKey *softKey;
    ZNavTabWidget *tabWidget;
    ZWidget *myWidget;
    
protected slots:
    void slotImegeSelected();
    void lbStatusSel( int );
    void lbQStatusSel( int );
    void lbSmileSel( ZIconViewItem *item );
    void lbStatusChange( );
    void lbQStatusChange( );
    void slotPageChanged(QWidget* );
    
};

#endif 

