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

#include "ZMyListBox.h"

#include "zDefs.h"


ZMyListBox::ZMyListBox( QWidget* parent, WFlags f)
 : ZListBox ( parent, f )
{
	showGroup=true;
}

ZMyListBox::ZMyListBox( QString type, QWidget* parent, WFlags f)
 : ZListBox ( type, parent, f )
{
	showGroup=true;	
}

ZMyListBox::~ZMyListBox()
{
}

void ZMyListBox::dellAllContactWithProtocol( int prot, bool clear )
{
	for ( ListMap::Iterator it = listContact.begin( ); it != listContact.end( ); it++) 
	{ 
		if ( it.data()->getProtocol() == prot )
		{
			takeItem( it.data() );
			if ( clear )
			{
				delete it.data();
				it.data()=NULL;
				listContact.remove( it );
			}
		}	
	}
		
	for ( ListMap::Iterator it = listGroup.begin( ); it != listGroup.end( ); it++) 
	{ 
		if ( it.data()->getProtocol() == prot )
		{
			takeItem( it.data() );
			if ( clear )
			{			
				delete it.data();
				it.data()=NULL;
				listGroup.remove( it );
			}

		}	
	}
}

void ZMyListBox::contactRemove( string idContact )
{
	ZContactItem * item = listContact[idContact];
	
	if ( item == NULL )
		return;

	takeItem(item);
	delete item;
	item=NULL;
	listContact.remove(idContact);
}

void ZMyListBox::contactAdd( ZContactItem * item )
{
	listContact[item->getUID()] = item;
	
	int insertAt = -1;	
	
	ZContactItem * itemAt;
	if ( showGroup )
		itemAt = listGroup[item->getGroupIdStr()];
	else
		itemAt = listProt[PROT_SPLIT2];
	
	if ( itemAt != NULL )
		insertAt = index(itemAt);
			
	insertItem( item, insertAt);
}

ZContactItem * ZMyListBox::getConact(string id)
{
	return listContact[id];
}

void ZMyListBox::groupAdd( ZContactItem * item, string idGroup )
{
	listGroup[idGroup] = item;
	
	int insertAt = -1;
	
	if ( item->getProtocol() == PROT_ICQ )
	{
		ZContactItem * itemAt = listProt[PROT_SPLIT2];
		if ( itemAt != NULL )
			insertAt = index(itemAt);
	}
	
	if ( showGroup )
		insertItem( item, insertAt);	
}

void ZMyListBox::protAdd( ZContactItem * item )
{
	listProt[item->getProtocol()] = item;
	insertItem( item, -1);	
}

void ZMyListBox::showHideGroup(int idGroup)
{
	ZContactItem * itemAt = listGroup[QString::number(idGroup).latin1()];
	
	if ( itemAt == NULL || !showGroup )
		return;
		
	itemAt->setHide(!itemAt->isHide());

	if ( !itemAt->isHide() )
	{
		ZContactItem * itemC;
		for ( int i=index(itemAt)+1; i<count();i++ )
		{
			itemC = item(i);
			if ( itemC == NULL || itemC->getGroupId() != idGroup )
				break;
			takeItem( itemC );
			itemC->setHide(false);
		}
	} else
	{
		for ( ListMap::Iterator it = listContact.begin( ); it != listContact.end( ); it++) 
			if ( it.data()->getGroupId() == idGroup )
			{
				insertItem( it.data(), index(itemAt));	
				it.data()->setHide(true);
			}	
	}
}

void ZMyListBox::keyReleaseEvent ( QKeyEvent* e )
{
	switch ( e->key() ) 
	{
		case Z6KEY_GREEN:
		{ 
			emit onDialButtonPress();
			break; 
		}    
		case Z6KEY_RED:
		{ 
			emit onRedButtonPress();
			break; 
		}
		case Z6KEY_LEFT:
		{
			int vLines = 0;
			for (int y=0; y<(count()-1); ++y)
			{
				if (itemVisible(y))
				{
					++vLines;
				}
			}
			int t = currentItem();
			if (t > (vLines - 1))
			{
				this->setCurrentItem( t - vLines, true );
				#ifndef CUTED_PLATFORM
				this->setCurrentItem( t + vLines );
				#endif
			} else
			{
				this->setCurrentItem( 0, true );
				#ifndef CUTED_PLATFORM
				this->setCurrentItem( t + vLines );
				#endif
			}
			break; 
		}
		case Z6KEY_RIGHT:
		{ 
			int vLines = 0;
			for (int y=0; y<(count()-1); ++y)
			{
				if (itemVisible(y))
				{
				++vLines;
				}
			}
			int t = currentItem();
			if (t < ( count() - vLines - 1))
			{
				this->setBottomItem( t + vLines );
				this->setCurrentItem( t + vLines, true );
				#ifndef CUTED_PLATFORM
				this->setCurrentItem( t + vLines );
				#endif
			} else
			{
				this->setCurrentItem( count() - 1, true );
				#ifndef CUTED_PLATFORM
				this->setCurrentItem( t + vLines );
				#endif
			}
			break; 
		}

		default:
			ZListBox::keyReleaseEvent( e );
	}
}

void ZMyListBox::setShowGroup(bool show)
{
	if ( showGroup == show )
		return;

	if ( !showGroup )
	{
		for ( ListMap::Iterator it = listGroup.begin( ); it != listGroup.end( ); it++)  
			if ( index(it.data()) != -1 )
				takeItem( it.data() );
	} else
	{
		dellAllContactWithProtocol(PROT_ICQ, false);
		#ifdef _XMPP
		dellAllContactWithProtocol(PROT_JABBER, false);	
		#endif

		int insertAt;
		for ( ListMap::Iterator it = listGroup.begin( ); it != listGroup.end( ); it++) 
		{ 
			insertAt = -1;
		
			if ( it.data()->getProtocol() == PROT_ICQ )
			{
				ZContactItem * itemAt = listProt[PROT_SPLIT2];
				if ( itemAt != NULL )
					insertAt = index(itemAt);
			}

			insertItem( it.data(), insertAt);
			it.data()->setHide( !it.data()->isHide() );
			showHideGroup( it.data()->getGroupId() );
		}
		
	}
}

//+++++++++++++++++++++ Fix std function for multithred ++++++++++++++++++++++++++

void ZMyListBox::clear()
{
	mutexOnRepaint.lock();
	ZListBox::clear();
	mutexOnRepaint.unlock();
}

void ZMyListBox::takeItem( const ZListItem * item)
{
	mutexOnRepaint.lock();
	ZListBox::takeItem( item );
	mutexOnRepaint.unlock();
}

void ZMyListBox::insertItem( const ZListItem * item, int index, bool autoHighlighted)
{
	mutexOnRepaint.lock();
	ZListBox::insertItem( item, index, autoHighlighted);
	mutexOnRepaint.unlock();
}

void ZMyListBox::insertItem( const ZListItem * item, const ZListItem *after, bool autoHighlighted)
{
	mutexOnRepaint.lock();
	ZListBox::insertItem( item, after, autoHighlighted);
	mutexOnRepaint.unlock();
}

void ZMyListBox::changeItem( const ZListItem * item, int index )
{	
	mutexOnRepaint.lock();
	ZListBox::changeItem( item, index );
	mutexOnRepaint.unlock();
}

void ZMyListBox::moveItem(int from, int to)
{
	mutexOnRepaint.lock();
	ZListBox::moveItem( from, to );
	mutexOnRepaint.unlock();
}

void ZMyListBox::viewportPaintEvent( QPaintEvent * event)
{
	if ( mutexOnRepaint.locked() )
		return;
	mutexPaintEvent.lock();
	ZListBox::viewportPaintEvent( event );
	mutexPaintEvent.unlock();
}

void ZMyListBox::UpdateList()
{
	if ( mutexOnRepaint.locked() )
		return;
	//eventFilter( this, (QEvent *)&QKeyEvent(QEvent::KeyPress,Z6KEY_POUND,0,1));
	//eventFilter( this, (QEvent *)&QKeyEvent(QEvent::KeyRelease,Z6KEY_POUND,0,0));
	mutexPaintEvent.lock();
	ZListBox::update();
	mutexPaintEvent.unlock();	
}

ZContactItem * ZMyListBox::item( int index )
{
	return (ZContactItem *)ZListBox::item( index );
}
