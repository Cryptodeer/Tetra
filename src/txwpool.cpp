#include "txwpool.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp> 

#define UPDATE_TIME UNIXTIME_MINUTE


bool CTxWPool::Init(){
    tgroup = NULL;
    if(!dbRead()){
        dbWrite();
    }
    return true;
}

bool CTxWPool::Sort(){
	std::sort(this->plist.begin(),this->plist.end(), SortStrategy);
}

bool CTxWPool::SortStrategy( const DummyParams & x, const DummyParams & y){
	return x.getAccount() < y.getAccount(); 
}

bool CTxWPool::AllocatedBalance(const std::string &Account, Cash &Balance){
	BOOST_FOREACH(const DummyParams &p, this->plist)
		if( p.getAccount() == Account )
			BOOST_FOREACH(const PAIRTYPE(std::string, Cash) &t, p.getTarget())
				Balance += t.second;
}