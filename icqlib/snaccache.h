/***************************************************************************
 *   Copyright (C) 2007 by Alexander S. Salieff                            *
 *   salieff@mail.ru                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _SNAC_CACHE_H_
#define _SNAC_CACHE_H_

#include "flap.h"
#include <time.h>

#include <qmutex.h>
#include <qmap.h>
#include <qvector.h>
#include <qlist.h>

class SNAC
{
	public:
		SNAC(time_t t, SNACData d)
			{time_=t; data_=d;};
		~SNAC(){};
		time_t time(){return time_;};
		SNACData data(){return data_;};
	private:
		time_t time_;
		SNACData data_;
};

class SnacCache {
//Q_OBJECT
 public:
  SnacCache();
  ~SnacCache();
  void addEntry(SNACData & snd);
  bool findEntry(SNACData & snd);
  void clearOlderThan(time_t tmdelta);
  bool findEntryFromTime(SNACData & snd, time_t tmdelta);

 private:
  //vector<pair<SNACData, time_t> > cache;
  //typedef QMap <time_t, SNACData> tSnacCache;

  QList<SNAC> cache;

  mutable QMutex mutex;

  //pthread_mutex_t mutex;
  //vector<SNACData> waitSNAC;
};

/*
#include <vector>

using namespace std;

class SnacCache {
 public:
  SnacCache();
  ~SnacCache();
  void addEntry(SNACData & snd);
  bool findEntry(SNACData & snd);
  void clearOlderThan(time_t tmdelta);
  bool findEntryFromTime(SNACData & snd, time_t tmdelta);

  
 private:
  vector<pair<SNACData, time_t> > cache;

  pthread_mutex_t mutex;
  //vector<SNACData> waitSNAC;
};
*/
#endif
