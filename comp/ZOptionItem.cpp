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

#include "ZOptionItem.h"

#include "ZListBox.h"

#include <ZNumPickerDlg.h>
#include <ZSingleSelectDlg.h>
#include <ZSingleCaptureDlg.h>

#ifndef WITHOUT_EDIT_INTERNET_PROFILE
#include <napi.h>
#endif

#include "lng.h"

ZOptionItem::ZOptionItem( ZListBox* _container, EDIT_TYPE _type ):
	ZSettingItem( _container, "%M" )
{
	type=_type;
	if ( type != EDIT_INTERNET_PROFILE )
		n=0;
	else
		n=1;
	min=0;
	max=0;
	title="";
	text="";
	list=NULL;
	
	appendSubItem(0, " ", true);
	appendSubItem(0, " ");
	
	connect( _container, SIGNAL(selected(ZSettingItem*)), this, SLOT(selected(ZSettingItem*)));
}

ZOptionItem::~ZOptionItem()
{
	disconnect( listBox(), SIGNAL(selected(ZSettingItem*)), this, SLOT(selected(ZSettingItem*)));
	delete this;
}

void ZOptionItem::selected(ZSettingItem* item)
{
	if ( (ZOptionItem*)item != this )
		return;
	
	switch ( type )
	{
		case EDIT_TEXT:
		case EDIT_FILE:
			{
			ZSingleCaptureDlg* zscd = new ZSingleCaptureDlg(title, "", ZSingleCaptureDlg::TypeLineEdit, this, "", true, 0, 0);
			ZLineEdit* zle = (ZLineEdit*)zscd->getLineEdit();
			zle->setText(text);
			if ( zscd->exec() == QDialog::Accepted )
				setText(zle->text());
			delete zle;
			delete zscd;
			}
			break;
		case EDIT_BOOL_YESNO:
		case EDIT_BOOL_ONOFF:
			{
			QStringList itemList;
			if ( type==EDIT_BOOL_ONOFF )
			{
				itemList.append(LNG_ON);
				itemList.append(LNG_OFF);
			} else
			{
				itemList.append(LNG_YES);
				itemList.append(LNG_NO);				
			}
			ZSingleSelectDlg *dlg = new ZSingleSelectDlg(title, "", this);
			dlg->addItemsList(itemList);
			dlg->getListBox()->checkItem(!n, true);
			if ( dlg->exec() == QDialog::Accepted )
				setNum(!dlg->getCheckedItemIndex());
			delete dlg;
			}
			break;
		case EDIT_NUM:
			{
			ZNumPickerDlg * dlg = new ZNumPickerDlg(2, this);
			ZNumModule* num = dlg->getNumModule();
			num->setMaxValue(max);
			num->setMinValue(min);
			num->setValue(n);
			if ( dlg->exec() == QDialog::Accepted )
				setNum( num->getValue() );
			delete num;
			delete dlg;
			}
			break;
		case EDIT_ONE_OF_LIST:
			{
			if ( list == NULL )
				return;
			ZSingleSelectDlg *dlg = new ZSingleSelectDlg(title, "", this);
			dlg->addItemsList(*list);
			dlg->getListBox()->checkItem(n, true);
			if ( dlg->exec() == QDialog::Accepted )
				setNum(dlg->getCheckedItemIndex());
			delete dlg;	
			}	
			break;
		case EDIT_INTERNET_PROFILE:
			#ifndef WITHOUT_EDIT_INTERNET_PROFILE
			{
			UINT32 profiles = NAPI_GetMaxProfiles();
			QStringList list;
			list.append(LNG_ASK);
			if(profiles != -1) 
			{
				char *buf = new char[NAPI_MAX_PROFILE_NAME_LENGTH * profiles];
				if(NAPI_ListAllProfile((INT8*)buf, &profiles) != -1) 
				{
					char *prof_ptr = buf;
					for(int i = 0; i < profiles; i++, prof_ptr = buf + i * NAPI_MAX_PROFILE_NAME_LENGTH)
						list.append(QString::fromUtf8(prof_ptr));
				}
				delete buf;	
			}
			ZSingleSelectDlg *dlg = new ZSingleSelectDlg(title, "", this);
			dlg->addItemsList(list);
			
			int sel=0, i=0;
			for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
				if ( *it == text )
				{
					sel=i;
					break;
				} else
					i++;
			if ( sel >= 0 )
				dlg->getListBox()->checkItem(sel, true);
			if ( dlg->exec() == QDialog::Accepted )
			{
				n=dlg->getCheckedItemIndex();
				setText( *(list.at(n)) );
			}
			delete dlg;
			}
			#endif
			break;
	}
}

void ZOptionItem::setNum(int _n)
{
	n=_n;
	switch ( type )
	{
		case EDIT_BOOL_YESNO:
			setSubItem(0, 1, n?LNG_YES:LNG_NO);
			break;
		case EDIT_BOOL_ONOFF:
			setSubItem(0, 1, n?LNG_ON:LNG_OFF);
			break;
		case EDIT_NUM:
			setSubItem(0, 1, QString::number(n));
			break;
		case EDIT_ONE_OF_LIST:
			if ( list != NULL )
				setSubItem(0, 1, *(list->at(n)));
			break;		
	}
}

void ZOptionItem::setMaxMin(int _max, int _min)
{
	max=_max;
	min=_min;
}

int ZOptionItem::getNum()
{
	return n;
}

void ZOptionItem::setTitle(QString _title)
{
	title=_title;
	setSubItem(0, 0, title);	
}
	
void ZOptionItem::setText(QString _text)
{
	text=_text;
	setSubItem(0, 1, text);
}

QString ZOptionItem::getText()
{		
	#ifndef WITHOUT_EDIT_INTERNET_PROFILE
	if ( type == EDIT_INTERNET_PROFILE && n==0 )
		return "";
	#endif
	
	return text;
}

void ZOptionItem::setList( QStringList * _list )
{
	list = _list;
}
