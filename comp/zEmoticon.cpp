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

#include "zDefs.h"

    TYPE_SMILE 	masEMOTICON [MAX_EMOTICON_COUNT] =
	{
	    16, 16,  "[*MES*]"      , ""     , "onmes.png" ,       NULL,
	    16, 16,  "[*WADD*]"     , ""     , "wasadd.png",       NULL,
	    16, 16,  "[*AUTH*]"     , ""     , "authok.png",       NULL,
	};

zEmotIcons::zEmotIcons(QString dir)
{
	logMes("zEmotIcons: read smile config");
	emoticCount = 3;
	ProgDir = dir;
	
	FILE * smiles = fopen(ProgDir+"smile/list.cfg","rt");
	if ( smiles != NULL )
	{
		char smile1[255], smile2[255], fileName[255];
		int width, height;
		smile1[0]=0;
		smile2[0]=0;
		fileName[0]=0;
		int ret=0;
		ret = fscanf(smiles,"%d, %d, %s, %s, %s", &height, &width, &smile1, &smile2, &fileName);
		while ( !feof(smiles) && (ret==5) )
		{
			if ( emoticCount >= MAX_EMOTICON_COUNT )
				break;
			masEMOTICON[emoticCount].height = height;
			masEMOTICON[emoticCount].width = width;
			masEMOTICON[emoticCount].emoticonstr1 = (QString(smile1)!="-")?QString(smile1):"";
			masEMOTICON[emoticCount].emoticonstr2 = (QString(smile2)!="-")?QString(smile2):"";
			masEMOTICON[emoticCount].iconname =  QString(fileName);
			masEMOTICON[emoticCount].emoticonPixmap =  NULL;
			emoticCount++;
			ret = fscanf(smiles,"%d, %d, %s, %s, %s", &height, &width, &smile1, &smile2, &fileName);
		}
		fclose(smiles);
	}else
	{
		logMes("zEmotIcons: file not found");
	}
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
			masEMOTICON[k].emoticonPixmap = new  QPixmap(ProgDir + "/smile/" + masEMOTICON[k].iconname);
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
	return ProgDir + "/smile/" + masEMOTICON[k].iconname;
}

uint zEmotIcons::getSmileCount()
{
	return emoticCount;
}
