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

#include "zEmoticon.h"
#include <stdio.h>

#include "qdir.h"

#include "zDefs.h"

    TYPE_SMILE 	masEMOTICON [MAX_EMOTICON_COUNT] =
	{
	    16, 13,  "[*MES*]"      , ""     , "mes.png" ,         NULL,
	    16, 13,  "[*MES_MY*]"   , ""     , "mes_my.png" ,      NULL,   
	    16, 16,  "[*WADD*]"     , ""     , "wasadd.png",       NULL,
	    16, 16,  "[*AUTH*]"     , ""     , "authok.png",       NULL,
	    
	};

zEmotIcons::zEmotIcons(QString dir)
{
	logMes("zEmotIcons: read smile config");
	emoticCount = 3;
	ProgDir = dir;
	ani = false;

	logMes("zEmotIcons: all smile added");
}

zEmotIcons::~zEmotIcons()
{
}

void zEmotIcons::clearCash()
{
    for (uint k =0 ; k < emoticCount ;k ++)
    {
        if (masEMOTICON[k].emoticonPixmap != NULL)	
	{	
            delete masEMOTICON[k].emoticonPixmap;
            masEMOTICON[k].emoticonPixmap = NULL;
	}
    }
}

const QPixmap & zEmotIcons::getEmotIcon(uint k)
{
	if ( k >= emoticCount )
		return QPixmap();
	
	if (masEMOTICON[k].emoticonPixmap == NULL)
	{
		if (k<NO_SMILE)
		{
			masEMOTICON[k].emoticonPixmap = new  QPixmap(ProgDir + "/CL/" + masEMOTICON[k].iconname);
		} else
		{
			masEMOTICON[k].emoticonPixmap = new  QPixmap(ProgDir + "/smile/" + smilePack + "/" + masEMOTICON[k].iconname);
		}
	}
	return *(masEMOTICON[k].emoticonPixmap);
}

QString zEmotIcons::getEmotStr(uint k, bool n)
{
	if ( k >= emoticCount )
		return "";
			
	if (!n)
	{
		return 	masEMOTICON[k].emoticonstr1;   
	} else
	{
		return 	masEMOTICON[k].emoticonstr2; 
	}
}

int zEmotIcons::getSmileWidth(int k)
{
	return 	masEMOTICON[k].width; 	
}

int zEmotIcons::getSmileHeigth(int k)
{
	return 	masEMOTICON[k].height; 	
}

QString zEmotIcons::getSmilePath(int k)
{
	return ProgDir + "/smile/" + smilePack + "/" + masEMOTICON[k].iconname;
}

uint zEmotIcons::getSmileCount()
{
	return emoticCount;
}

QStringList * zEmotIcons::getSmilePackList()
{
	QStringList * list = new QStringList();
	
	QDir dir ( ProgDir+"smile/", "", QDir::Name | QDir::DirsFirst | QDir::IgnoreCase );
	#ifdef OLD_SDK
	dir.setMatchAllDirs ( true );
	#endif
	dir.setFilter ( QDir::Dirs );
	if ( !dir.isReadable() )
		return list;
		
	QStringList entries = dir.entryList();
	for ( QStringList::ConstIterator it = entries.begin(); it != entries.end(); it++ )
		if ( (*it)[0] != "."  )
			list->append(*it);
	
	return list;
}

void zEmotIcons::setSmilePack( QString name )
{
	emoticCount = 3;
	smilePack = name;
	FILE * smiles = fopen(ProgDir+"smile/"+name+"/list.cfg","rt");
	if ( smiles != NULL )
	{
		char smile1[255], smile2[255], fileName[255];
		int width, height;
		smile1[0]=0;
		smile2[0]=0;
		fileName[0]=0;
		int ret=0;
		ret = fscanf(smiles,"%d\n", &ani );
		ret = fscanf(smiles,"%d %d %s %s %s\n", &width, &height, &smile1, &smile2, &fileName);
		char * c = NULL;
		while ( !feof(smiles) && (ret>=5) )
		{
			if ( emoticCount >= MAX_EMOTICON_COUNT )
				break;
			c = strchr(smile1, '#');
			if ( c!=NULL ) *c = ' '; 
			c = strchr(smile2, '#');
			if ( c!=NULL ) *c = ' '; 
			masEMOTICON[emoticCount].height = height;
			masEMOTICON[emoticCount].width = width;
			masEMOTICON[emoticCount].emoticonstr1 = (QString(smile1)!="-")?QString(smile1):"";
			masEMOTICON[emoticCount].emoticonstr2 = (QString(smile2)!="-")?QString(smile2):"";
			masEMOTICON[emoticCount].iconname =  QString(fileName);
			masEMOTICON[emoticCount].emoticonPixmap =  NULL;
			emoticCount++;
			ret = fscanf(smiles,"%d %d %s %s %s\n", &width, &height, &smile1, &smile2, &fileName);
		}
		fclose(smiles);
	}	
}

bool zEmotIcons::isAniSmile( int k )
{
	if ( k < NO_SMILE )
		return false;
	return ani;
}
