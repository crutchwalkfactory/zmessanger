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

#ifndef EMOTICON_H
#define EMOTICON_H

#include <qstring.h>
#include <qpixmap.h>

#define MAX_EMOTICON_COUNT  100
#define MAX_EMOTICONT  emoticCount
#define NO_SMILE  3

typedef struct
{
    int width;
    int height;
    QString emoticonstr1;
    QString emoticonstr2;
    QString iconname;
    QPixmap * emoticonPixmap;
}  TYPE_SMILE;

class zEmotIcons
{
public:
    zEmotIcons(QString dir);
    ~zEmotIcons();

    const QPixmap & getEmotIcon(uint k);
    QString getEmotStr(uint k,bool n);
    void clearCash();
    QString getSmilePath(int k);
    int getSmileWidth(int k);	
    int getSmileHeigth(int k);
    
    uint getSmileCount();
    
private:
	QString ProgDir;
	uint emoticCount;
};
#endif
