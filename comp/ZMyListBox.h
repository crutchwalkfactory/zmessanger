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

#ifndef ZMYLISTBOX_H
#define ZMYLISTBOX_H

#include "config.h"

#include "ZContactItem.h"

#include <ZListBox.h>
#include <qmutex.h>

#include <string>


typedef QMap<std::string, ZContactItem*> ListMap;
typedef QMap<int, ZContactItem*> List2Map;

#define PROT_JABBER      100   //0x1000
#define PROT_ICQ         200   //0x2000
#define PROT_SPLIT1      301   //0x3001
#define PROT_SPLIT2      302   //0x3002
#define ICQ_NOT_IN_LIST_GROUP 401

class ZMyListBox : public ZListBox
{
  Q_OBJECT
  
  public:
    ZMyListBox( QWidget* parent = 0, WFlags f = 0);
    ZMyListBox( QString type, QWidget* parent = 0, WFlags f = 0);
    ~ZMyListBox();
    
    void clear();
    
    void contactAdd( ZContactItem * item );
    void contactRemove( string idContact );
    ZContactItem * getConact(string id);
    
    void groupAdd( ZContactItem * item, string idGroup );
    void groupAdd( ZContactItem * item, int idGroup )
    {
		groupAdd( item, QString::number(idGroup).latin1() );
	};
    
    void protAdd( ZContactItem * item );
    
    void showHideGroup(int idGroup);
    
    void insertItem( const ZListItem * item, int index=-1, bool autoHighlighted = TRUE );
    void insertItem( const ZListItem * item, const ZListItem *after, bool autoHighlighted = TRUE);
    void changeItem( const ZListItem * item, int index );
    void takeItem( const ZListItem * item);
    void moveItem(int from, int to);
    void UpdateList();
	ZContactItem * item( int index );
	
	void setShowGroup( bool show );

	// this function is bad
	void dellAllContactWithProtocol( int prot, bool lock = true );

  private slots: 
    virtual void viewportPaintEvent( QPaintEvent * pe);
    virtual void keyReleaseEvent(QKeyEvent* e);

  private:
    mutable QMutex mutexOnRepaint;
    mutable QMutex mutexPaintEvent;
    mutable QMutex mutexAction;    
    bool showGroup;

  signals:
	void onDialButtonPress();
	void onLeftButtonPress();
	void onRigtButtonhPress();
	void onRedButtonPress();
	
  protected:
	ListMap listContact;
	ListMap   listGroup;
	List2Map   listProt;
};


#endif
