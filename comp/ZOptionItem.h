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

#ifndef ZOPTIONITEM_H
#define ZOPTIONITEM_H

#ifdef OLD_SDK
#include <ZListBox.h>
#else
#include <ZSettingItem.h>
#endif

#include <qstringlist.h>

class Q_EXPORT ZOptionItem: public QWidget, public ZSettingItem
{
	Q_OBJECT
public: 
	enum EDIT_TYPE
	{
		EDIT_TEXT,
		EDIT_TEXT_NUM,		
		EDIT_BOOL_YESNO,
		EDIT_BOOL_ONOFF,		
		EDIT_NUM,
		EDIT_ONE_OF_LIST,
		EDIT_INTERNET_PROFILE,
		EDIT_FILE,
	};

	ZOptionItem( ZListBox* _container, EDIT_TYPE _type );
	~ZOptionItem();
	
	void setTitle(QString _title);
	
	void setText(QString _text);
	QString getText();
	
	void setNum(int _n);
	void setMaxMin(int _max, int _min=0);
	int  getNum();
	
	void setList( QStringList * _list );

protected slots:
	void selected(ZSettingItem*);

private:
	EDIT_TYPE type;
	int n, min, max;
	QString title, text;
	QStringList * list;
};

#endif
