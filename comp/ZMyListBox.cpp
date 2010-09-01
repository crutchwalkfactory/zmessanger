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

#define qstrcmp strcmp
#include "zDefs.h"

using namespace std;

ZMyListBox::ZMyListBox( QWidget* parent, WFlags f)
 : ZListBox ( parent, f )
{
	showGroup=true;
	sortType=0;
	QFont font ( qApp->font() );
	font.setPointSize ( 16 );
	setItemFont (ZListBox::LISTITEM_REGION_A, ZSkinBase::StStandard, font );
	setItemFont (ZListBox::LISTITEM_REGION_A, ZSkinBase::StHighlightSelected, font );	
	setItemFont (ZListBox::LISTITEM_REGION_A, ZSkinBase::StSelected, font );	
	setItemFont (ZListBox::LISTITEM_REGION_A, ZSkinBase::StHighlighted, font );	
	setDefaultItemHeight(16);
}

ZMyListBox::ZMyListBox( QString type, QWidget* parent, WFlags f)
 : ZListBox ( type, parent, f )
{
	showGroup=true;
	sortType=0;
	QFont font ( qApp->font() );
	font.setPointSize ( 16 );
	setItemFont (ZListBox::LISTITEM_REGION_A, ZSkinBase::StStandard, font );
	setItemFont (ZListBox::LISTITEM_REGION_A, ZSkinBase::StHighlightSelected, font );
	setItemFont (ZListBox::LISTITEM_REGION_A, ZSkinBase::StSelected, font );	
	setItemFont (ZListBox::LISTITEM_REGION_A, ZSkinBase::StHighlighted, font );		
	setDefaultItemHeight(16);
}

ZMyListBox::~ZMyListBox()
{
}

void ZMyListBox::dellAllContactWithProtocol( int prot, bool lock )
{
	if ( lock )
		QMutexLocker locker ( &mutexAction );
	
	logMes_3("ZMyListBox::dellAllContactWithProtocol");
	
	for ( ListMap::Iterator it = listContact.begin( ); it != listContact.end( ); it++) 
	{
		if ( it.data() && it.data()->getProtocol()==prot )
		{
			if ( !it.data()->isHide() )			
				takeItem( it.data() );			
		}	
	}
		
	for ( ListMap::Iterator it = listGroup.begin( ); it != listGroup.end( ); it++) 
	{ 
		if ( it.data() && it.data()->getProtocol()==prot )
		{
			if ( !it.data()->isHide() )			
				takeItem( it.data() );
		}	
	}
}

void ZMyListBox::contactRemove( string idContact )
{
	logMes_3("ZMyListBox::contactRemove");
	
	ListMap::Iterator it = listContact.find(idContact);
	
	if ( it == listContact.end() )
		return;	
		
	ZContactItem* item = it.data();
	
	listContact.remove(it);
	if ( item && !item->isHide() )
		takeItem( item );	
}

void ZMyListBox::contactAdd( ZContactItem * item )
{
	logMes_3("ZMyListBox::contactAdd");
	
	listContact[item->getUID()] = item;
	
	int insertAt = -1;	
	
	ZContactItem * itemAt;
	if ( showGroup )
		itemAt = listGroup[item->getGroupIdStr()];
	else
		itemAt = listProt[PROT_SPLIT2];
	
	if ( itemAt )
		insertAt = index(itemAt)+1;
			
	insertItem( item, insertAt);
	item->setHide(false);
	
	UpdateList();
}

ZContactItem * ZMyListBox::getConact(string id)
{
	return listContact[id];
}

void ZMyListBox::groupAdd( ZContactItem * item, string idGroup )
{
	logMes_3("ZMyListBox::groupAdd");
	
	listGroup[idGroup] = item;
	
	int insertAt = -1;
	
	if ( item->getProtocol() == PROT_ICQ )
	{
		ZContactItem * itemAt = listProt[PROT_SPLIT2];
		if ( itemAt )
			insertAt = index(itemAt)+1;
	}
	
	if ( showGroup )
	{
		insertItem( item, insertAt);	
		item->setHide(false);
	}
}

void ZMyListBox::protAdd( ZContactItem * item )
{
	logMes_3("ZMyListBox::protAdd");
	
	listProt[item->getProtocol()] = item;
	insertItem( item, -1 );
	item->setHide(false);	
}

void ZMyListBox::showHideGroup(int idGroup, bool lock)
{
	logMes_3("ZMyListBox::showHideGroup 1");	
	
	if ( lock )
	{
		if ( mutexAction.locked() )
			return;
		QMutexLocker locker ( &mutexAction );
	}
	
	logMes_3("ZMyListBox::showHideGroup 2");
	
	ZContactItem * itemAt = listGroup[QString::number(idGroup).latin1()];
	
	if ( itemAt == NULL || !showGroup )
		return;
		
	if ( !itemAt->isHide() )
	{
		ZContactItem * itemC;
		int i = index(itemAt)+1;
		while ( (itemC=item(i)) != NULL )
		{
			if ( itemC->getGroupId() != itemAt->getGroupId() )
				break;
			takeItem( itemC );
			itemC->setHide(true);
		}
		itemAt->setHide(true);
	} else
	{
		itemAt->setHide(false);

		for ( ListMap::Iterator it = listContact.begin( ); it != listContact.end( ); it++) 
			if ( it.data() && it.data()->getGroupId()==idGroup )
			{
				if ( !it.data()->isHide() )
					continue;
				insertItem( it.data(), index(itemAt)+1);	
				it.data()->setHide(false);
			}

		sortContact( idGroup, false );
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
				this->setCurrentItem( t + vLines );
			} else
			{
				this->setCurrentItem( 0, true );
				this->setCurrentItem( t + vLines );
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
				this->setCurrentItem( t + vLines );
			} else
			{
				this->setCurrentItem( count() - 1, true );
				this->setCurrentItem( t + vLines );
			}
			break; 
		}

		default:
			ZListBox::keyReleaseEvent( e );
	}
}

void ZMyListBox::setShowGroup(bool show)
{
	logMes_3("ZMyListBox::setShowGroup 1");	
	
	QMutexLocker locker ( &mutexAction );
	
	logMes_3("ZMyListBox::setShowGroup 2");
	
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
		
			if ( it.data() && it.data()->getProtocol()==PROT_ICQ )
			{
				ZContactItem * itemAt = listProt[PROT_SPLIT2];
				if ( itemAt != NULL )
					insertAt = index(itemAt)+1;
			}

			insertItem( it.data(), insertAt);
			it.data()->setHide( !it.data()->isHide() );
			showHideGroup( it.data()->getGroupId(), false );
		}
		
	}
}

int ZMyListBox::cmpContact( ZContactItem * item1, ZContactItem * item2 )
{
    if ( !item1 || !item2 )
		return 0;

	qDebug("cmp Contact");

	if ( item1->intStat() > item2->intStat() )
		return 1;
	else
	if ( item1->intStat() < item2->intStat() )	
		return -1;
	else
	if ( sortType == SORT_STATUS )
		return 0;
	else
	if ( item1->getNick() > item2->getNick() )
		return 1;
	else
	if ( item1->getNick() < item2->getNick() )	
		return -1;	
	
	return 0;
}

void ZMyListBox::sortContact( int idGroup, bool lock )
{
	qDebug("sortContact: 0");
	
	if ( lock )
	{
		if ( mutexAction.locked() )
			return;
			
		QMutexLocker locker ( &mutexAction );
	}
	qDebug("sortContact: 1");
	
	ZContactItem * group = listGroup[QString::number(idGroup).latin1()];
	
	if ( group==NULL || (group->isHide() && showGroup ) )
		return;
	
	int i;
	
	ZContactItem * item1;
	ZContactItem * item2;
	
	int s=(showGroup?index(group):0)+1;
	int e=(showGroup?0:count())-1;

	if ( showGroup )
	{
		for (i = s; i<count(); i++ )
			if ( item(i)->isGroup() )
			{
				e = i-1;
				break;
			}
	}

	qDebug("Sort %d - %d",s,e);
	
	if ( s==-1 || e==-1 || e-s<2 ) return;

	while (e > s)
	{
		qDebug("while (%d > %d)",s,e);
		for (i=s; i<e; i++)
		{
			item1 = item( i );
			item2 = item( i+1 );
			if ( cmpContact( item1, item2 ) == 1 )
				moveItem(i+1,i);
		}
		for (i=e; i>s; i--)
		{
			item1 = item( i );
			item2 = item( i-1 );
			if ( cmpContact( item1, item2 ) == -1 )
				moveItem(i-1,i);
		}
		s++;
		e--;
	}
}

void ZMyListBox::sortContactAll()
{
	logMes_3("ZMyListBox::sortContactAll");
	
	if ( sortType == NO_SORT  || count()<4 )
		return;
	
	for ( ListMap::Iterator it = listGroup.begin( ); it != listGroup.end( ); it++) 
		if ( it.data() )
			sortContact( it.data()->getGroupId() );	
}

void ZMyListBox::setSortType( int type )
{
	logMes_3("ZMyListBox::setSortType");
	
	if ( sortType == type )
		return;
		
	sortType = type;
	
	sortContactAll();
}

void ZMyListBox::setNewMes( ZContactItem * item )
{
	if ( !item )
		return;
	
	logMes_3("ZMyListBox::setNewMes");
	
	item->setNewMes( true );
	if ( item->isHide() )
	{
		ZContactItem * group = listGroup[ item->getGroupIdStr() ];
		if ( group )
			group->setNewMes( true );
	} else
		sortContact( item->getGroupId() );		
}

void ZMyListBox::changeStatus( ZContactItem * item, int n )
{
	if ( !item )
		return;
	
	logMes_3("ZMyListBox::changeStatus");
	
	item->setStatus( n );
	sortContact( item->getGroupId() );
}

//+++++++++++++++++++++ Fix std function for multithred ++++++++++++++++++++++++++

void ZMyListBox::clear()
{
	mutexOnRepaint.lock();
	listContact.clear();
	listGroup.clear();
	listProt.clear();	
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
	if ( mutexOnRepaint.locked() || mutexPaintEvent.locked() )
		return;
	mutexPaintEvent.lock();
	ZListBox::update();
	mutexPaintEvent.unlock();	
}

ZContactItem * ZMyListBox::item( int index )
{
	return (ZContactItem *)ZListBox::item( index );
}
