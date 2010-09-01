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

enum TYPE_BDMES
{
	TYPE_MY_MESSAGE,
	TYPE_MESSAGE,
	TYPE_AUTH_REQUEST,
	TYPE_AUTH_ACCEPTED,
	TYPE_AUTH_DECLINED,
	TYPE_WAY_ADDED,
};

class BDMes
{
public:
	BDMes() {}
	BDMes(const QString title, const QString mes, QDateTime time, TYPE_BDMES type)
	{ title_=title; mes_=mes; time_=time; type_=type; }
	~BDMes(){}

	QString title() const { return title_; }
	QString mes() const { return mes_; }
	QDateTime time() const { return time_; }
	TYPE_BDMES type() const { return type_; }
	void setTitle( QString s ) { title_ = s; }
	void setMessage( QString s ) { mes_ = s; }
	void setTimr( QDateTime s ) { time_ = s; }
	void setType( TYPE_BDMES s = TYPE_MESSAGE ) { type_ = s; }

private:
	QString mes_;
	QString title_;
	QDateTime time_;
	TYPE_BDMES type_;//Message - 1 QuestionAutorize - 2  AskAutorize - 3 � ��
};

typedef QValueList<BDMes> mesList;

typedef QMap<unsigned int, mesList> tHistory;

#ifdef _XMPP
typedef QMap<std::string, mesList> tHistoryXMPP;
#endif

#endif
