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

#ifndef xTextView_H
#define xTextView_H

#ifndef _ZScrollPanel
#include <ZScrollView.h>
#else
#define ZScrollView ZScrollPanel
#include <ZScrollPanel.h>
#endif

#include <qpixmap.h>
#include <qnamespace.h>
#include <qfont.h>
#include <qfontmetrics.h>

#include <ZApplication.h>

#include "zEmoticon.h"
#include "zDefs.h"

#include <qmutex.h>

#ifndef NO_ANI_SMILE
#include "UTIL_GifPlayer.h"
#define MAX_SMILE_ON_SCREEN 10
#endif

class Position
{
public:
        Position(): num(0), len(0) {}
        Position( int _num, int _len )
            : num(_num), len(_len)
        { }
		int num;
		int len;
};


typedef QMap<int, Position> iconList;

class xTextView : public ZScrollView
{ 
	Q_OBJECT 
	public: 
		xTextView(QWidget* parent = 0, zEmotIcons * smile = 0); 
		~xTextView(); 
		
		void setFont(QFont);
		void setFontSize(int size);
		
		void setText(QString str, bool repait = true);
		void addText(QString text, bool repait = true);
		
		void nextLine();
		void prevLine();
		void pageUp();
		void pageDown();
		void toEnd();
		bool isEnd;
				
	private slots: 
		virtual void viewportPaintEvent( QPaintEvent * pe);
		void keyPressEvent( QKeyEvent* pEvent );
		
#ifdef _ZScrollPanel
		void drawContents(QPainter*, int cx, int cy, int cw, int ch){};
		void drawContentsOffset(QPainter*, int ox, int oy,int cx, int cy, int cw, int ch){};
#endif
		
	private:
		QString text[100000];
		int fontHeigth;
		QFont font;
		int colLines;
		int posLines;
		int lineOnPage;
		bool notEndColor;
		int maxLineHeigth;
		zEmotIcons * smileIcon;
		QColor int2color(int);
		iconList smile(QString * s);
		int drawText(QPainter * p, int x, int y, QString text);
		void insertText(QString str, int insertTo);
		
		bool autoScrollByKey;
		
		mutable QMutex mutexPaintEvent;
		mutable QMutex mutexInsertText;		
		
		#ifndef NO_ANI_SMILE
		UTIL_GifPlayer * aniSmileCash[MAX_SMILE_ON_SCREEN];
		int aniSmileNum[MAX_SMILE_ON_SCREEN];
		bool aniSmileUse[MAX_SMILE_ON_SCREEN];
		int countAniSmile;
		#endif
};
#endif

