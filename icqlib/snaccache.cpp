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

#include "snaccache.h"

SnacCache::SnacCache()
{
	/*
	pthread_mutexattr_t mutexattr;

	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);

	pthread_mutex_init(&mutex, &mutexattr);

	pthread_mutexattr_destroy(&mutexattr);
	*/
}

SnacCache::~SnacCache()
{
	//pthread_mutex_destroy (&mutex);
}

void SnacCache::addEntry(SNACData & snd)
{
	QMutexLocker locker(&mutex);
	cache.append( new SNAC(time(NULL),snd) );
	//cache.push_back(pair<SNACData, time_t>(snd, time(NULL)));
}

bool SnacCache::findEntry(SNACData & snd)
{
	QMutexLocker locker(&mutex);
	for (int i=0; i<cache.count(); ++i)
	{
		if ((cache.at(i)->data().service_id==snd.service_id || cache.at(i)->data().service_id==0) && \
		(cache.at(i)->data().subtype_id==snd.subtype_id || snd.subtype_id==0) && \
		(cache.at(i)->data().req_id==snd.req_id || snd.req_id==0) )
		{
			snd=cache.at(i)->data();
			cache.remove(i);
			return true;
		}
	}

/*
 for (size_t i=0; i<cache.size(); ++i)
  {
  if ((cache[i].first.service_id==snd.service_id || snd.service_id==0) && \
      (cache[i].first.subtype_id==snd.subtype_id || snd.subtype_id==0) && \
      (cache[i].first.req_id==snd.req_id || snd.req_id==0))
   {
   	snd=cache[i].first;
	cache.erase(cache.begin()+i);

	//logMes_4("UnLock process");
	pthread_mutex_unlock (&mutex);

   	return true;
   }
  }
*/
 return false;
}

bool SnacCache::findEntryFromTime(SNACData & snd, time_t tTime)
{
	QMutexLocker locker(&mutex);	
	for (int i=0; i<cache.count(); ++i)
	{
		if ( (tTime-cache.at(i)->time())>0 ) // >10
			continue;
		if ((cache.at(i)->data().service_id==snd.service_id || cache.at(i)->data().service_id==0) && \
		(cache.at(i)->data().subtype_id==snd.subtype_id || snd.subtype_id==0) && \
		(cache.at(i)->data().req_id==snd.req_id || snd.req_id==0) )
		{
			snd=cache.at(i)->data();
			cache.remove(i);
			return true;
		}
	}

/*
 for (size_t i=0; i<cache.size(); ++i)
  {
  if ( (tTime-cache[i].second)>10 )
  {
  	continue;
  }
  if ((cache[i].first.service_id==snd.service_id || snd.service_id==0) && \
      (cache[i].first.subtype_id==snd.subtype_id || snd.subtype_id==0) && \
      (cache[i].first.req_id==snd.req_id || snd.req_id==0) )
   {
	snd=cache[i].first;
	cache.erase(cache.begin()+i);

	//logMes_4("UnLock process");
	pthread_mutex_unlock (&mutex);

   	return true;
   }
  }
*/
 return false;
}

void SnacCache::clearOlderThan(time_t tmdelta)
{
	QMutexLocker locker(&mutex);
	for (int i=0; i<cache.count();)
	{
		if ( (time(NULL)-cache.at(i)->time())>=tmdelta )
		{
			cache.remove(i);
		} else
		{
			++i;
		}
	}

/*
 for (size_t i=0; i<cache.size(); )
  {
  if ((time(NULL)-cache[i].second)>=tmdelta)
	cache.erase(cache.begin()+i);
  else ++i;
  }*/

}

/*
SnacCache::SnacCache()
{
	pthread_mutexattr_t mutexattr;

	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);

	pthread_mutex_init(&mutex, &mutexattr);

	pthread_mutexattr_destroy(&mutexattr);
}

SnacCache::~SnacCache()
{
	pthread_mutex_destroy (&mutex);
}

void SnacCache::addEntry(SNACData & snd)
{
 //logMes_4("Lock process");
 pthread_mutex_lock (&mutex);


 cache.push_back(pair<SNACData, time_t>(snd, time(NULL)));

 //logMes_4("UnLock process");
 pthread_mutex_unlock (&mutex);
}

bool SnacCache::findEntry(SNACData & snd)
{
 //logMes_4("Lock process");
 pthread_mutex_lock (&mutex);

 for (size_t i=0; i<cache.size(); ++i)
  {
  if ((cache[i].first.service_id==snd.service_id || snd.service_id==0) && \
      (cache[i].first.subtype_id==snd.subtype_id || snd.subtype_id==0) && \
      (cache[i].first.req_id==snd.req_id || snd.req_id==0))
   {
   	snd=cache[i].first;
	cache.erase(cache.begin()+i);

	//logMes_4("UnLock process");
	pthread_mutex_unlock (&mutex);

   	return true;
   }
  }

 //logMes_4("UnLock process");
 pthread_mutex_unlock (&mutex);

 return false;
}

bool SnacCache::findEntryFromTime(SNACData & snd, time_t tTime)
{
 //logMes_4("Lock process");
 pthread_mutex_lock (&mutex);


 for (size_t i=0; i<cache.size(); ++i)
  {
  if ( (tTime-cache[i].second)>10 )
  {
  	continue;
  }
  if ((cache[i].first.service_id==snd.service_id || snd.service_id==0) && \
      (cache[i].first.subtype_id==snd.subtype_id || snd.subtype_id==0) && \
      (cache[i].first.req_id==snd.req_id || snd.req_id==0) )
   {
	snd=cache[i].first;
	cache.erase(cache.begin()+i);

	//logMes_4("UnLock process");
	pthread_mutex_unlock (&mutex);

   	return true;
   }
  }

 //logMes_4("UnLock process");
 pthread_mutex_unlock (&mutex);
 
 return false;
}

void SnacCache::clearOlderThan(time_t tmdelta)
{
 //logMes_4("Lock process");
 pthread_mutex_lock (&mutex);

 for (size_t i=0; i<cache.size(); )
  {
  if ((time(NULL)-cache[i].second)>=tmdelta)
	cache.erase(cache.begin()+i);
  else ++i;
  }

 //logMes_4("UnLock process");
 pthread_mutex_unlock (&mutex);
}
*/
