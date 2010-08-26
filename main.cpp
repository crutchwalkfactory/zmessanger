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

#include "zgui.h"

#include "ZMyApplication.h"
#include <qapplication.h>
#include "zEmoticon.h"
#include "config.h"

#ifdef _AllLogInFile
void myMessageOutput( QtMsgType type, const char *msg );
#endif

ZGui* zgui;
ZMyApplication * app;
zEmotIcons * zSmile;

QString ProgDir;

QString cfg_inetProfile;
bool cfg_alertVibr;
bool cfg_alertRing;
int cfg_alertRingVol;
QString cfg_alertPath;
int cfg_timeKeepConnect;
int cfg_maxNumLines;
int cfg_sortType;
bool cfg_dontShowOffLine;
bool cfg_dontShowGroup;
bool cfg_rigthAlignXStatus;
bool cfg_notSendTypeMes;
bool cfg_InetOnUSB;
int cfg_chatFontSize;
int cfg_mesFontSize;
bool cfg_sendByCenter; 
bool cfg_noShowStatusBarInChat; 
QString oldMes;//For save old typed text
int oldSmile;//For save old smile position    

long int trafIN, trafOUT;

int main ( int argc, char **argv )
{
	//init extern
	oldMes = "";
	oldSmile = 0;
	trafIN = 0;
	trafOUT = 0;
	cfg_inetProfile="";

	// Create ZApplication
	app = new ZMyApplication ( argc, argv );

	// If app run - show exist app
	if ( QCopChannel::isRegistered(ZMESSANGER_CHENEL) )
	{
		QCopChannel::send( ZMESSANGER_CHENEL, "showMainWin()" );
		return 0;
	}
	
	#ifdef _AllLogInFile
	if (QString(argv[1]) != QString("-nodbg"))
	{
		qDebug("DebugMode ON");
		if (QFile::exists("/mmc/mmca1/zMessangerLog.txt"))
			QFile::remove("/mmc/mmca1/zMessangerLog.txt");	
		qInstallMsgHandler( myMessageOutput );
	}
	#endif
	
	zgui = new ZGui ( NULL, "ZMainWidget" );
	app->setMainWidget(zgui);
	zgui->show();

	int ret = app->exec();
	delete zgui;
	zgui = NULL;
	delete app;
	app = NULL;
	return ret;
}

//For create Debug log in file
#ifdef _AllLogInFile
void myMessageOutput( QtMsgType type, const char *msg )
{
  	FILE* fd = 0;	
	switch ( type ) 
	{
		case QtDebugMsg:
			{
				fd = fopen( "/mmc/mmca1/zMessangerLog.txt", "a" );
				QString str;
				str = QString(msg)+"\n";
				fwrite (str,1,str.length(),fd);				
				fprintf( stderr, "Debug: %s\n", msg );
				fclose(fd);
				break;
			}
		case QtWarningMsg:
			{
				fd = fopen( "/mmc/mmca1/zMessangerError.txt", "a" );
				QString str;
				str = QString(msg)+"\n";
				fwrite (str,1,str.length(),fd);						
				fprintf( stderr, "Warning: %s\n", msg );
				fclose(fd);
				break;
			}
		case QtFatalMsg:
			{
				fd = fopen( "/mmc/mmca1/zMessangerError.txt", "a" );
				QString str;
				str = QString(msg);
				fwrite (str,1,str.length(),fd)+"\n";				
				fprintf( stderr, "Fatal: %s\n", msg );
				fclose(fd);
				abort();                    // deliberately core dump
			}
	}
}
#endif
