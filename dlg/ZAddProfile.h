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

#ifndef ZADDPROFILE_H
#define ZADDPROFILE_H

#include "./../BaseDlg.h"
#include <ZFormContainer.h>
#include <ZComboBox.h>
#include <ZWidget.h>
#include <ZSoftKey.h>

#ifdef _FixLineEdit
#include "./../comp/ZMyLineEdit.h"
#endif

#ifdef _FixLineEdit
	#define ZLineEdit ZMyLineEdit
#endif

class ZAddProfile : public MyBaseDlg 
{
  Q_OBJECT

  public:
    ZAddProfile(); 
    ~ZAddProfile(){};
    
    QString getNewPas()  { return pas;  };
    QString getNewId()   { return id;   };  
    int     getNewNum()  { return num;  }; 
    int     getNewProt() { return prot; }; 
    
    void setEditProfile(QString ID, QString Pas, int Num, int Prot);

  public slots:
    void changeProtocol(int);   
    void addProfile();    
    
  private:
    QString pas;
    QString id;
    int num;
    int prot;   
    bool edit;
    
    ZWidget *myWidget;
    
    ZComboBox * zcbProtocol;
    
    #ifdef _FixLineEdit
    ZMyLineEdit * zleID;
    ZMyLineEdit * zlePas; 
    #else
    ZLineEdit * zleID;
    ZLineEdit * zlePas; 
    #endif
};

#endif // ZADDPROFILE_H
