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

#include "xTextView.h"

#include <cmath>
#include <algorithm>

xTextView::xTextView(QWidget* parent, zEmotIcons * smile) 
	:ZScrollView(parent, "xTextView", WRepaintNoErase)
{ 
	setMargins(0,0,0,0);
	setFrameShape(QFrame::NoFrame);
	
	font = QFont ( qApp->font() );
	maxLineHeigth = font.pixelSize();
	isEnd = true;	
	lineOnPage = (height()-20) / maxLineHeigth;
	posLines = 0;
	colLines = 0;
	smileIcon = smile;
	
	#ifndef NO_ANI_SMILE
	countAniSmile = 0;
	for ( int i=0; i<MAX_SMILE_ON_SCREEN; i++ )
	{
		aniSmileCash[i]=NULL;
		aniSmileNum[i]=0;
		aniSmileUse[i]=false;
	}
	#endif
	
	setFocusPolicy( StrongFocus );
	
	setHScrollBarMode( QScrollView::AlwaysOff );
	setVScrollBarMode( QScrollView::AlwaysOff );
} 

xTextView::~xTextView()
{
}

void xTextView::setFont(QFont f)
{
	font = f;
	lineOnPage = (height()-20) / maxLineHeigth;
}

void xTextView::setFontSize(int size)
{
	font.setPointSize(size);
	lineOnPage = (height()-20)/ maxLineHeigth;
}

iconList xTextView::smile(QString * s)
{
	iconList res;
	
	if (smileIcon == 0)
		return res;
		
	int ret;
	
	for (int k = 0 ; k < zSmile->getSmileCount();k ++)
	{
		ret = 0;
		while ( ret > -1 )
		{
			ret = s->find(smileIcon->getEmotStr(k, 0), ret);
			if ( ret > -1 )
			{			
				res.insert( ret, Position(k, smileIcon->getEmotStr(k, 0).length()) );
				maxLineHeigth = std::max(maxLineHeigth, smileIcon->getSmileHeigth(k));
				ret += 2;
			}
		}
		if ( smileIcon->getEmotStr(k, 1) == "" )
		{
			continue;
		}
		ret = 0;
		while ( ret > -1 )
		{
			ret = s->find(smileIcon->getEmotStr(k, 1), ret);
			if ( ret > -1 )
			{			
				res.insert( ret, Position(k, smileIcon->getEmotStr(k, 1).length()) );
				maxLineHeigth = std::max(maxLineHeigth, smileIcon->getSmileHeigth(k));
				ret += 2;
			}
		}
	}
	return res;
}

void xTextView::insertText(QString str, int insertTo)
{	
	QMutexLocker locker ( &mutexInsertText );
	
	if ( insertTo == 0)
		maxLineHeigth = font.pixelSize();
	
	QFontMetrics fm( font );
	
	iconList smiles = smile( &str );
	
	QRegExp rx( "$[1-9][@%^&]$[A-Za-z0-9 ]*" );
	
	str.replace( QString("	"), QString::fromUtf8( "     " ));
	str.replace( QRegExp("\10\13"), "\n");

	int pos = 0;
	int posTo=0;
	int line=insertTo;
	int spasePos;
	int spaseLen;
	int len;
	int maxWidth = width() - fm.maxWidth() - 18;
	while ( pos < str.length() )
	{
		posTo = str.find('\n',pos);
		
		if ( posTo < 0 )
			posTo = str.length();
		
		spasePos = pos;
		spaseLen = 0;
    	len=0;
    	for (int i=pos; i<posTo; i++)
		{
			if ( smiles.find(i) != smiles.end() )
			{
				i+=smiles.find(i).data().len;
				len += smileIcon->getSmileWidth(smiles.find(i).data().num);
				continue;
			}
			if ( str.mid(i,5).find(rx) != -1 )
			{
				i+=4;
				continue;
			}
			len += fm.width( str[i] );

			if ( str[i] == ' ' )
			{
				spasePos = i;
				spaseLen = len;
			}				
			if ( len > maxWidth )
			{
				if ( spasePos == pos )
				{
					text[line]=str.mid(pos, i-pos+1 );
					pos = i+1;
					len = 0;
					spaseLen = 0;
				} else
				{
					text[line]=str.mid( pos, spasePos-pos );
					pos = spasePos+1;
					len = len-spaseLen;
				}
				line ++;
				spasePos = pos;
			}
		}
		
		if  ( pos != posTo )
		{
			text[line] = str.mid(pos,posTo-pos);
			line++;
		}
		
		/////////
		pos = posTo+1;
	}
	if ( insertTo == 0)
		posLines = 0;
	colLines = line;

	lineOnPage = (height()-20)/ maxLineHeigth;
}

void xTextView::setText(QString str, bool repait)
{
	insertText(str, 0);
	isEnd = false;
	if (repait)	
		viewport()->update();
}

void xTextView::addText(QString str, bool repait)
{
	insertText(str, colLines);
	isEnd = false;
	if (repait)
		viewport()->update();
}

void xTextView::nextLine()
{
	if (isEnd)
		return;
	if ( colLines-lineOnPage > posLines )
	{
		posLines++;	
		viewport()->update();
		isEnd = false;
	}
}

void xTextView::prevLine()
{
	if ( posLines > 0 )
	{
		posLines--;
		viewport()->update();
		isEnd = false;
	}
}

void xTextView::pageUp()
{
	if ( posLines > lineOnPage )
	{
		posLines = std::max(0,posLines-lineOnPage);
	} else
	{
		posLines = 0;	
		setContentsPos(0, 0);
	}
	viewport()->update();
	isEnd = false;
}

void xTextView::pageDown()
{
	if ( (height()-20)/font.pixelSize() < colLines-posLines )
	{
		posLines = std::min(posLines+lineOnPage, colLines) ;
		viewport()->update();
		isEnd = false;
	} else
	{
		toEnd();
	}
}

void xTextView::toEnd()
{	
	if (isEnd)
		return;
	if ( colLines <= 0 || lineOnPage <= 0 )
		return;
	QFontMetrics fm( font );
	iconList smiles;
	int line = 0;
	int posY=12;
	int maxHeigth;
	for (int i = colLines-1; i >= 0; i-- )
	{
		smiles = smile(&text[i]);
		if ( smiles.count() == 0 )
		{		
			posY += font.pixelSize()+1;
		} else
		{
			iconList::Iterator it;
			maxHeigth = 0;
			for ( it = smiles.begin(); it != smiles.end(); ++it ) 
			{
				maxHeigth = std::max(maxHeigth, smileIcon->getSmileHeigth(it.data().num)+1);
			}
			posY = posY + std::max(maxHeigth, font.pixelSize() )+1;
		}

		if ( posY > height() )
			break;
		
		line++;
	}
	if ( line == 0 )
		posLines = colLines - lineOnPage;
	else
		posLines = colLines - line;
	viewport()->update();
	isEnd = true;
}

QColor xTextView::int2color(int n)
{
	switch (n)
	{
		case 1:  return QColor(255,0,0);   //red
		case 2:  return QColor(0,255,0);   //green
		case 3:  return QColor(0,0,255);    //blue
		case 4:  return QColor(192,194,194);//grey
		default: return QColor(0,0,0);      //black
	}
}

//Font options
//$[COLOR][TYPE]$[TEXT]#
//[COLOR] - num color (0-9)
//[TYPE]:
//			@ - normal
//			% - bold
//			^ - italic
//			& - bold&italic

int xTextView::drawText(QPainter * p, int x, int y, QString paintText)
{
	QRegExp rx( "$[1-9][@%^&]$[ ]*" );
	QFontMetrics fm( font );
	int pos = paintText.find(rx);
	int ret=0;
	if ( pos == -1 ) //No text change element
	{
		if (!notEndColor)
		{
			p->drawText(x, y, paintText, -1);
			ret = fm.width(paintText);
		} else
		{
			int posEnd = paintText.find('#');
			if ( posEnd != -1 )
			{
				p->drawText(x+ret, y, paintText.mid(0, posEnd), -1);	
				ret+=fm.width(paintText.mid(0, posEnd));
				notEndColor = false;
				font.setBold(false);
				font.setItalic(false);
				p->setFont(font);
				p->setPen( QColor(0,0,0) );
				posEnd+=1;
				if ( posEnd<paintText.length() )
				{
					p->drawText(x+ret, y, paintText.mid(posEnd, paintText.length()-posEnd ), -1);
					ret+=fm.width(paintText.mid(posEnd, paintText.length()-posEnd ));
				}
			} else
			{
				p->drawText(x, y, paintText, -1);
				ret = fm.width(paintText);
			}			
		}
	} else
	{	
		
		if (0 != pos)
		{
			p->drawText(x, y, paintText.mid(0,pos), -1);
			ret+=fm.width(paintText.mid(0,pos));
		}
		p->setPen( QColor(int2color( paintText.mid(pos+1,1).at(0).digitValue() ) ) );
		switch ( paintText.mid(pos+2,1).at(0).latin1() )
		{
			case '%':
			{
				font.setBold(true);
				p->setFont(font);
				break;			
			}
			case '^':
			{
				font.setItalic(true);	
				p->setFont(font);
				break;							
			}
			case '&':
			{
				font.setBold(true);
				font.setItalic(true);
				p->setFont(font);
				break;					
			}								
		}
		pos+=4;
		int posEnd = paintText.find('#',pos);
		if ( posEnd != -1 )
		{
			p->drawText(x+ret, y, paintText.mid(pos, posEnd-pos), -1);	
			ret+=fm.width(paintText.mid(pos, posEnd-pos));
			notEndColor = false;
			font.setBold(false);
			font.setItalic(false);
			p->setFont(font);
			p->setPen( QColor(0,0,0) );
			posEnd+=1;
			if ( posEnd<paintText.length() )
			{
				pos = paintText.find(rx);
				if (pos == -1 )
				{
					p->drawText(x+ret, y, paintText.mid(posEnd, paintText.length()-posEnd ), -1);
					ret+=fm.width(paintText.mid(posEnd, paintText.length()-posEnd ));
				} else
					ret = ret + drawText(p, x+ret, y, paintText.mid(posEnd, paintText.length()-posEnd ) );
			}
		} else
		{
			p->drawText(x+ret, y, paintText.mid(pos, paintText.length()-pos), -1);
			ret+=fm.width(paintText.mid(pos, paintText.length()-pos));
			notEndColor = true;	
		}
	}
	return ret;//delta peremeshenia za sians
}

void xTextView::viewportPaintEvent(QPaintEvent * pe) 
{
	if ( mutexPaintEvent.locked() )
		return;
		
	QMutexLocker locker( &mutexPaintEvent );
	
	logMes("xTextView::PaintEvent");
	
	QPixmap pixmap(viewport()->width(), viewport()->height()); 
	QPainter p(&pixmap, viewport()); 

	#ifndef NO_ANI_SMILE
	for ( int i=0; i<MAX_SMILE_ON_SCREEN; i++ )
	{
		aniSmileUse[i] = false;
	}
	countAniSmile=0;
	#endif	

	// Set bg
	p.setPen(QColor(255,255,255));
	p.setBrush(QColor(255,255,255));
	p.drawRect(0, 0, viewport()->width(), viewport()->height());
	
	//set font
	p.setFont( font );
	p.setPen( QColor(0,0,0) );
	
	//QRegExp rx( "$?$*#" );
	QFontMetrics fm( font );
	iconList smiles;
	int left;
	int lefPos;
	int maxHeigth;
	
	// draw text
	int posY=10;
	notEndColor = false;
	int i;
	for (i = posLines; i < colLines; i++ )
	{
		smiles = smile(&text[i]);
		left = 5;
		if ( smiles.count() == 0 )
		{
				drawText(&p, left, posY, text[i]);
		} else
		{
			iconList::Iterator it;
			lefPos=0;
			maxHeigth = 0;
			for ( it = smiles.begin(); it != smiles.end(); ++it ) 
			{
					maxHeigth = std::max(maxHeigth, smileIcon->getSmileHeigth(it.data().num));
			}
			posY = posY + std::max(0, maxHeigth - font.pixelSize() );
			maxHeigth = std::max(0, maxHeigth - fm.descent() );
			for ( it = smiles.begin(); it != smiles.end(); ++it ) 
			{
				if ( lefPos != it.key() )
				{
					left+=drawText(&p, left, posY, text[i].mid(lefPos,std::max(it.key()-lefPos, 0)));
				}
				lefPos=it.key()+it.data().len;
				#ifndef NO_ANI_SMILE
				if ( (smileIcon->isAniSmile( it.data().num )) && (countAniSmile<MAX_SMILE_ON_SCREEN) )
				{
					bool noFind=true;
					int i;
					// find exist smile on screen
					for ( i=0; i<MAX_SMILE_ON_SCREEN; i++ )
						if ( !aniSmileUse[i] && aniSmileNum[i] == it.data().num && aniSmileCash[i] )
						{
							moveChild(aniSmileCash[i], left+1, posY-maxHeigth);

							aniSmileUse[i]=true;
							noFind=false;
						}		
					if (noFind)
					{
						for ( i=0; i<MAX_SMILE_ON_SCREEN; i++ )
							if ( !aniSmileUse[i] )
							 break;	
						if ( aniSmileCash[i] )
						{
							removeChild( aniSmileCash[i] );
							delete aniSmileCash[i];
							aniSmileCash[i]=NULL;
						}
						aniSmileNum[i]=it.data().num;
						aniSmileUse[i]=true;
												
						aniSmileCash[i] = new UTIL_GifPlayer( smileIcon->getSmilePath(aniSmileNum[i]), viewport() );
						aniSmileCash[i]->setMemoryLimit( 0 ); //for minimal use memory
						aniSmileCash[i]->setBackgroundColor( viewport()->backgroundColor() );
						aniSmileCash[i]->setFixedHeight ( smileIcon->getSmileHeigth(aniSmileNum[i]) );
						aniSmileCash[i]->setFixedWidth ( smileIcon->getSmileWidth(aniSmileNum[i])+1 );
						addChild( aniSmileCash[i], left+1, posY-maxHeigth );
						aniSmileCash[i]->show();
						aniSmileCash[i]->startshow();
					}
						
					countAniSmile++;					
				}else
				#endif	
					p.drawPixmap( left+1, posY-maxHeigth, smileIcon->getEmotIcon(it.data().num) );		
				left+=smileIcon->getSmileWidth(it.data().num)+2;
			}
			drawText(&p, left, posY, text[i].mid(lefPos,text[i].length()-lefPos));
		}
		posY = posY + font.pixelSize() + 1;
		if ( posY > viewport()->height() )
			break;
	}
	//draw scroll bar
	QPen pen;
	pen.setWidth(3);
	p.setPen( pen );
	p.drawLine( width()-4, (int)(((double)posLines/colLines)*(height()-10)), width()-4, (int)(((double)i/colLines)*(height()-10))+10 );	
	
	#ifndef NO_ANI_SMILE
	for ( int i=0; i<MAX_SMILE_ON_SCREEN; i++ )
		if ( !aniSmileUse[i] && aniSmileCash[i] )
		{
			removeChild( aniSmileCash[i] );
			delete aniSmileCash[i];
			aniSmileCash[i]=NULL;
		}
	#endif		

	bitBlt(viewport(), 0, 0, &pixmap); 
	ZScrollView::viewportPaintEvent(pe); 
	
	logMes("xTextView::PaintEvent end");
}

void xTextView::keyPressEvent( QKeyEvent* pEvent )
{
	switch ( (pEvent)->key() )
	{
		case Z6KEY_LEFT:  pageUp();    break;
		case Z6KEY_RIGHT: pageDown();  break;
		case Z6KEY_UP:    prevLine();  break;
		case Z6KEY_DOWN:  nextLine();  break;
	}

	return ZScrollView::keyPressEvent( pEvent ); 	
}
