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

#ifndef _ZIMTYPE_H_
#define _ZIMTYPE_H_

#include <sys/timeb.h>
#include <qstring.h>
#include <qmap.h>
#include <qdatetime.h>
#include <qvaluelist.h>

#include "config.h"

#define TYPE_MY_MESSAGE 0
#define TYPE_MESSAGE 1
#define TYPE_AUTH_REQUEST 2
#define TYPE_AUTH_ACCEPTED 3
#define TYPE_AUTH_DECLINED 4
#define TYPE_WAY_ADDED 5

class BDMes
{
public:
	BDMes() {}
	BDMes(const QString title, const QString mes, QDateTime time,	const int type)
	{ title_=title; mes_=mes; time_=time; type_=type; }
	~BDMes(){}

	QString title() const { return title_; }
	QString mes() const { return mes_; }
	QDateTime time() const { return time_; }
	int type() const { return type_; }
	void setTitle( QString salary ) { title_ = salary; }
	void setMessage( QString salary ) { mes_ = salary; }
	void setTimr( QDateTime salary ) { time_ = salary; }
	void setType( int salary = TYPE_MESSAGE ) { type_ = salary; }

private:
	QString mes_;
	QString title_;
	QDateTime time_;
	char type_;//Message - 1 QuestionAutorize - 2  AskAutorize - 3 � ��
};

typedef QValueList<BDMes> mesList;

typedef QMap<unsigned int, mesList> tHistory;

#ifdef _XMPP
typedef QMap<std::string, mesList> tHistoryXMPP;
#endif

#endif
